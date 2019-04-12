/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2017 NITK Surathkal
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Authors: Aparna R. Joshi <aparna29th@gmail.com>
 *          Isha Tarte <tarteisha@gmail.com>
 *          Navya R S <navyars82@gmail.com>
 */

/*
 * PORT NOTE: This code was ported from ns-2.36rc1 (queue/rem.cc).
 * Most of the comments are also ported from the same.
 */

#include "ns3/log.h"
#include "ns3/enum.h"
#include "ns3/uinteger.h"
#include "ns3/double.h"
#include "ns3/simulator.h"
#include "ns3/abort.h"
#include "rem-queue-disc.h"
#include "ns3/drop-tail-queue.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("RemQueueDisc");

NS_OBJECT_ENSURE_REGISTERED (RemQueueDisc);

TypeId RemQueueDisc::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::RemQueueDisc")
    .SetParent<QueueDisc> ()
    .SetGroupName ("TrafficControl")
    .AddConstructor<RemQueueDisc> ()
    .AddAttribute ("InputWeight",
                   "Weight assigned to input rate",
                   DoubleValue (1.0),
                   MakeDoubleAccessor (&RemQueueDisc::m_inW),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("Phi",
                   "Value of phi used to calculate probability",
                   DoubleValue (1.001),
                   MakeDoubleAccessor (&RemQueueDisc::m_phi),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("MeanPktSize",
                   "Average packet size",
                   UintegerValue (1000),
                   MakeUintegerAccessor (&RemQueueDisc::m_meanPktSize),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("UpdateInterval",
                   "Time period after which link price and probability are calculated",
                   TimeValue (Seconds (0.002)),
                   MakeTimeAccessor (&RemQueueDisc::m_updateInterval),
                   MakeTimeChecker ())
    .AddAttribute ("Target",
                   "Target queue length",
                   UintegerValue (20),
                   MakeUintegerAccessor (&RemQueueDisc::m_target),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("Gamma",
                   "Value of gamma",
                   DoubleValue (0.001),
                   MakeDoubleAccessor (&RemQueueDisc::m_gamma),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("Alpha",
                   "Value of alpha",
                   DoubleValue (0.1),
                   MakeDoubleAccessor (&RemQueueDisc::m_alpha),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("MaxSize",
                   "The maximum number of packets accepted by this queue disc",
                   QueueSizeValue (QueueSize ("25p")),
                   MakeQueueSizeAccessor (&QueueDisc::SetMaxSize,
                                          &QueueDisc::GetMaxSize),
                   MakeQueueSizeChecker ())
    .AddAttribute ("LinkBandwidth",
                   "The REM link bandwidth",
                   DataRateValue (DataRate ("1.5Mbps")),
                   MakeDataRateAccessor (&RemQueueDisc::m_linkBandwidth),
                   MakeDataRateChecker ())
    .AddAttribute ("UseEcn",
                   "True to use ECN (packets are marked instead of being dropped)",
                   BooleanValue (false),
                   MakeBooleanAccessor (&RemQueueDisc::m_useEcn),
                   MakeBooleanChecker ())
  ;

  return tid;
}

RemQueueDisc::RemQueueDisc ()
  : QueueDisc (QueueDiscSizePolicy::SINGLE_INTERNAL_QUEUE)
{
  NS_LOG_FUNCTION (this);
  m_uv = CreateObject<UniformRandomVariable> ();
  m_rtrsEvent = Simulator::Schedule (m_updateInterval, &RemQueueDisc::RunUpdateRule, this);
}

RemQueueDisc::~RemQueueDisc ()
{
  NS_LOG_FUNCTION (this);
}

void
RemQueueDisc::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  m_uv = 0;
  Simulator::Remove (m_rtrsEvent);
  QueueDisc::DoDispose ();
}

int64_t
RemQueueDisc::AssignStreams (int64_t stream)
{
  NS_LOG_FUNCTION (this << stream);
  m_uv->SetStream (stream);
  return 1;
}

void
RemQueueDisc::InitializeParams (void)
{
  // Initially queue is empty so variables are initialize to zero
  m_linkPrice = 0.0;
  m_dropProb = 0.0;
  m_inputRate = 0.0;
  m_avgInputRate = 0.0;
  m_count = 0;
  m_countInBytes = 0;

  m_ptc = m_linkBandwidth.GetBitRate () / (8.0 * m_meanPktSize);
}

bool
RemQueueDisc::DoEnqueue (Ptr<QueueDiscItem> item)
{
  NS_LOG_FUNCTION (this << item);
  if (GetMaxSize ().GetUnit () == QueueSizeUnit::PACKETS)
    {
      m_count++;
    }
  else
    {
      m_count += item->GetSize ();
    }

  uint32_t nQueued = GetInternalQueue (0)->GetCurrentSize ().GetValue ();

  if ((GetMaxSize ().GetUnit () == QueueSizeUnit::PACKETS && nQueued + 1 > GetMaxSize (). GetValue())
      || (GetMaxSize ().GetUnit () == QueueSizeUnit::BYTES && nQueued + item->GetSize () > GetMaxSize (). GetValue())
     )
    {
      // Drops due to queue limit: reactive
      DropBeforeEnqueue (item, FORCED_DROP);
      return false;
    }
  else if (!m_useEcn && DropEarly (item))
    {
      // Early probability drop: proactive
      DropBeforeEnqueue (item, UNFORCED_DROP);
      return false;
    }

  // No drop
  bool retval = GetInternalQueue (0)->Enqueue (item);

  // If Queue::Enqueue fails, QueueDisc::Drop is called by the internal queue
  // because QueueDisc::AddInternalQueue sets the drop callback

  NS_LOG_LOGIC ("\t bytesInQueue  " << GetInternalQueue (0)->GetNBytes ());
  NS_LOG_LOGIC ("\t packetsInQueue  " << GetInternalQueue (0)->GetNPackets ());

  return retval;
}

bool RemQueueDisc::DropEarly (Ptr<QueueDiscItem> item)
{
  NS_LOG_FUNCTION (this << item);

  double p = m_dropProb;
  bool earlyDrop = true;
  double u = m_uv->GetValue ();

  if (u > p)
    {
      earlyDrop = false;
    }
  return earlyDrop;

}

Ptr<QueueDiscItem>
RemQueueDisc::DoDequeue (void)
{
  NS_LOG_FUNCTION (this);

  if (GetInternalQueue (0)->IsEmpty ())
    {
      NS_LOG_LOGIC ("Queue empty");

      return 0;
    }
  else
    {
      Ptr<QueueDiscItem> item = GetInternalQueue (0)->Dequeue ();

      if (m_useEcn)
        {
          DropEarly (item);
          Mark (item, UNFORCED_MARK);
        }

      NS_LOG_LOGIC ("Popped " << item);

      NS_LOG_LOGIC ("Number packets " << GetInternalQueue (0)->GetNPackets ());
      NS_LOG_LOGIC ("Number bytes " << GetInternalQueue (0)->GetNBytes ());

      return item;
    }
}

Ptr<const QueueDiscItem>
RemQueueDisc::DoPeek (void) const
{
  NS_LOG_FUNCTION (this);
  if (GetInternalQueue (0)->IsEmpty ())
    {
      NS_LOG_LOGIC ("Queue empty");
      return 0;
    }

  Ptr<const QueueDiscItem> item = GetInternalQueue (0)->Peek ();

  NS_LOG_LOGIC ("Number packets " << GetInternalQueue (0)->GetNPackets ());
  NS_LOG_LOGIC ("Number bytes " << GetInternalQueue (0)->GetNBytes ());

  return item;
}

void
RemQueueDisc::RunUpdateRule (void)
{
  NS_LOG_FUNCTION (this);
  double lp, in, in_avg, nQueued, c, exp, prob;

  // lp is link price (congestion measure)
  lp = m_linkPrice;

  // in is the number of bytes (if Queue mode is in bytes) or packets (otherwise)
  // arriving at the link (input rate) during one update time interval
  in = m_count;

  // in_avg is the low pass filtered input rate
  in_avg = m_avgInputRate;

  in_avg = in_avg * (1.0 - m_inW);

  if (GetMaxSize ().GetUnit () == QueueSizeUnit::BYTES)
    {
      in_avg = in_avg + m_inW * in / m_meanPktSize;
      nQueued = GetInternalQueue (0)->GetCurrentSize ().GetValue () / m_meanPktSize;
    }
  else
    {
      in_avg = in_avg + m_inW * in;
      nQueued = GetInternalQueue (0)->GetCurrentSize ().GetValue ();
    }

  // c measures the maximum number of packets that
  // could be sent during one update interval
  c = m_updateInterval.GetSeconds () * m_ptc;

  lp = lp + m_gamma * (in_avg + m_alpha * (nQueued - m_target) - c);

  if (lp < 0.0)
    {
      lp = 0.0;
    }

  exp = pow (m_phi, -lp);
  prob = 1.0 - exp;

  m_count = 0.0;
  m_avgInputRate = in_avg;
  m_linkPrice = lp;
  m_dropProb = prob;

  m_rtrsEvent = Simulator::Schedule (m_updateInterval, &RemQueueDisc::RunUpdateRule, this);
}

bool
RemQueueDisc::CheckConfig (void)
{
  NS_LOG_FUNCTION (this);
  if (GetNQueueDiscClasses () > 0)
    {
      NS_LOG_ERROR ("RemQueueDisc cannot have classes");
      return false;
    }

  if (GetNPacketFilters () > 0)
    {
      NS_LOG_ERROR ("RemQueueDisc cannot have packet filters");
      return false;
    }

  if (GetNInternalQueues () == 0)
    {
      // create a DropTail queue
      AddInternalQueue (CreateObjectWithAttributes<DropTailQueue<QueueDiscItem> >
                          ("MaxSize", QueueSizeValue (GetMaxSize ())));
    }

  if (GetNInternalQueues () != 1)
    {
      NS_LOG_ERROR ("RemQueueDisc needs 1 internal queue");
      return false;
    }

  return true;
}

} //namespace ns3
