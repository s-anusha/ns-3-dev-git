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
 *
 */

#include "ns3/test.h"
#include "ns3/rem-queue-disc.h"
#include "ns3/uinteger.h"
#include "ns3/string.h"
#include "ns3/double.h"
#include "ns3/log.h"
#include "ns3/simulator.h"

using namespace ns3;

/**
 * \ingroup traffic-control-test
 * \ingroup tests
 *
 * \brief Rem Queue Disc Test Item
 */
class RemQueueDiscTestItem : public QueueDiscItem
{
public:
  /**
   * Constructor
   *
   * \param p the packet
   * \param addr the address
   * \param protocol the protocol
   */
  RemQueueDiscTestItem (Ptr<Packet> p, const Address & addr, bool ecnCapable);
  virtual ~RemQueueDiscTestItem ();
  virtual void AddHeader (void);
  virtual bool Mark (void);

private:
  RemQueueDiscTestItem ();
  /// copy constructor
  RemQueueDiscTestItem (const RemQueueDiscTestItem &);
  /// assignment operator
  RemQueueDiscTestItem &operator = (const RemQueueDiscTestItem &);
  bool m_ecnCapablePacket; ///< ECN capable packet?
};

RemQueueDiscTestItem::RemQueueDiscTestItem (Ptr<Packet> p, const Address & addr, bool ecnCapable)
  : QueueDiscItem (p, addr, 0),
    m_ecnCapablePacket (ecnCapable)
{
}

RemQueueDiscTestItem::~RemQueueDiscTestItem ()
{
}

void
RemQueueDiscTestItem::AddHeader (void)
{
}

bool
RemQueueDiscTestItem::Mark (void)
{
  if (m_ecnCapablePacket)
    {
      return true;
    }
  return false;
}

/**
 * \ingroup traffic-control-test
 * \ingroup tests
 *
 * \brief Rem Queue Disc Test Case
 */
class RemQueueDiscTestCase : public TestCase
{
public:
  RemQueueDiscTestCase ();
  virtual void DoRun (void);
private:
  /**
   * Enqueue function
   * \param queue the queue disc
   * \param size the size
   * \param nPkt the number of packets
   */
  void Enqueue (Ptr<RemQueueDisc> queue, uint32_t size, uint32_t nPkt, bool ecnCapable);
  /**
   * Enqueue with delay function
   * \param queue the queue disc
   * \param size the size
   * \param nPkt the number of packets
   */
  void EnqueueWithDelay (Ptr<RemQueueDisc> queue, uint32_t size, uint32_t nPkt,bool ecnCapable);
  /**
  * Dequeue function
  * \param queue the queue disc
  * \param nPkt the number of packets
  */
  void Dequeue (Ptr<RemQueueDisc> queue, uint32_t nPkt);
  /**
  * Dequeue with delay function
  * \param queue the queue disc
  * \param delay the delay
  * \param nPkt the number of packets
  */
  void DequeueWithDelay (Ptr<RemQueueDisc> queue, double delay, uint32_t nPkt);
  /**
  * Run test function
  * \param mode the test mode
  */
  void RunRemTest (QueueSizeUnit mode);
};

RemQueueDiscTestCase::RemQueueDiscTestCase ()
  : TestCase ("Sanity check on the rem queue disc implementation")
{
}

void
RemQueueDiscTestCase::RunRemTest (QueueSizeUnit mode)
{
  uint32_t pktSize = 0;

  // 1 for packets; pktSize for bytes
  uint32_t modeSize = 1;

  uint32_t qSize = 300;
  Ptr<RemQueueDisc> queue = CreateObject<RemQueueDisc> ();


  // test 1: simple enqueue/dequeue with defaults, no drops
  /*NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("Mode", mode), true,
                         "Verify that we can actually set the attribute Mode");*/

  Address dest;

  if (mode == QueueSizeUnit::BYTES)
    {
      // pktSize should be same as MeanPktSize to avoid performance gap between byte and packet mode
      pktSize = 1000;
      modeSize = pktSize;
      qSize = qSize * modeSize;
    }

  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("MaxSize", QueueSizeValue (QueueSize (mode, qSize))),
                         true, "Verify that we can actually set the attribute MaxSize");

  Ptr<Packet> p1, p2, p3, p4, p5, p6, p7, p8;
  p1 = Create<Packet> (pktSize);
  p2 = Create<Packet> (pktSize);
  p3 = Create<Packet> (pktSize);
  p4 = Create<Packet> (pktSize);
  p5 = Create<Packet> (pktSize);
  p6 = Create<Packet> (pktSize);
  p7 = Create<Packet> (pktSize);
  p8 = Create<Packet> (pktSize);

  queue->Initialize ();
  NS_TEST_EXPECT_MSG_EQ (queue->GetCurrentSize ().GetValue (), 0 * modeSize, "There should be no packets in there");
  queue->Enqueue (Create<RemQueueDiscTestItem> (p1, dest, false));
  NS_TEST_EXPECT_MSG_EQ (queue->GetCurrentSize ().GetValue (), 1 * modeSize, "There should be one packet in there");
  queue->Enqueue (Create<RemQueueDiscTestItem> (p2, dest, false));
  NS_TEST_EXPECT_MSG_EQ (queue->GetCurrentSize ().GetValue (), 2 * modeSize, "There should be two packets in there");
  queue->Enqueue (Create<RemQueueDiscTestItem> (p3, dest, false));
  queue->Enqueue (Create<RemQueueDiscTestItem> (p4, dest, false));
  queue->Enqueue (Create<RemQueueDiscTestItem> (p5, dest, false));
  queue->Enqueue (Create<RemQueueDiscTestItem> (p6, dest, false));
  queue->Enqueue (Create<RemQueueDiscTestItem> (p7, dest, false));
  queue->Enqueue (Create<RemQueueDiscTestItem> (p8, dest, false));
  NS_TEST_EXPECT_MSG_EQ (queue->GetCurrentSize ().GetValue (), 8 * modeSize, "There should be eight packets in there");

  Ptr<QueueDiscItem> item;

  item = queue->Dequeue ();
  NS_TEST_EXPECT_MSG_EQ ((item != 0), true, "I want to remove the first packet");
  NS_TEST_EXPECT_MSG_EQ (queue->GetCurrentSize ().GetValue (), 7 * modeSize, "There should be seven packets in there");
  NS_TEST_EXPECT_MSG_EQ (item->GetPacket ()->GetUid (), p1->GetUid (), "was this the first packet ?");

  item = queue->Dequeue ();
  NS_TEST_EXPECT_MSG_EQ ((item != 0), true, "I want to remove the second packet");
  NS_TEST_EXPECT_MSG_EQ (queue->GetCurrentSize ().GetValue (), 6 * modeSize, "There should be six packet in there");
  NS_TEST_EXPECT_MSG_EQ (item->GetPacket ()->GetUid (), p2->GetUid (), "Was this the second packet ?");

  item = queue->Dequeue ();
  NS_TEST_EXPECT_MSG_EQ ((item != 0), true, "I want to remove the third packet");
  NS_TEST_EXPECT_MSG_EQ (queue->GetCurrentSize ().GetValue (), 5 * modeSize, "There should be five packets in there");
  NS_TEST_EXPECT_MSG_EQ (item->GetPacket ()->GetUid (), p3->GetUid (), "Was this the third packet ?");

  item = queue->Dequeue ();
  item = queue->Dequeue ();
  item = queue->Dequeue ();
  item = queue->Dequeue ();
  item = queue->Dequeue ();

  item = queue->Dequeue ();
  NS_TEST_EXPECT_MSG_EQ ((item == 0), true, "There are really no packets in there");

  // test 2: more data with defaults, unforced drops but no forced drops
  queue = CreateObject<RemQueueDisc> ();
  pktSize = 1000;
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("MaxSize", QueueSizeValue (QueueSize (mode, qSize))),
                         true, "Verify that we can actually set the attribute MaxSize");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("Alpha", DoubleValue (0.1)), true,
                         "Verify that we can actually set the attribute Alpha");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("Gamma", DoubleValue (0.1)), true,
                         "Verify that we can actually set the attribute Gamma");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("UpdateInterval", TimeValue (Seconds (0.002))), true,
                         "Verify that we can actually set the attribute UpdateInterval");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("Target", UintegerValue (50)), true,
                         "Verify that we can actually set the attribute Target");
  queue->Initialize ();
  EnqueueWithDelay (queue, pktSize, 600,false);
  DequeueWithDelay (queue,0.012,600);

  Simulator::Stop (Seconds (8.0));
  Simulator::Run ();

  QueueDisc::Stats st = queue->GetStats ();
  uint32_t test2 = st.GetNDroppedPackets (RemQueueDisc::UNFORCED_DROP);
  NS_TEST_EXPECT_MSG_NE (test2, 0, "There should be some unforced drops");
  NS_TEST_EXPECT_MSG_EQ (st.GetNDroppedPackets (RemQueueDisc::FORCED_DROP), 0, "There should be zero forced drops");


  // test 3: same as test 2, but with higher Target
  queue = CreateObject<RemQueueDisc> ();
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("MaxSize", QueueSizeValue (QueueSize (mode, qSize))),
                         true, "Verify that we can actually set the attribute MaxSize");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("Alpha", DoubleValue (0.1)), true,
                         "Verify that we can actually set the attribute Alpha");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("Gamma", DoubleValue (0.1)), true,
                         "Verify that we can actually set the attribute Gamma");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("UpdateInterval", TimeValue (Seconds (0.002))), true,
                         "Verify that we can actually set the attribute UpdateInterval");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("Target", UintegerValue ( 65)), true,
                         "Verify that we can actually set the attribute Target");
  queue->Initialize ();
  EnqueueWithDelay (queue, pktSize, 600,false);
  DequeueWithDelay (queue,0.012,600);

  Simulator::Stop (Seconds (8.0));
  Simulator::Run ();
  
  st = queue->GetStats ();
  uint32_t test3 = st.GetNDroppedPackets (RemQueueDisc::UNFORCED_DROP);
  NS_TEST_EXPECT_MSG_LT (test3, test2, "Test 3 should have less unforced drops than test 2");
  NS_TEST_EXPECT_MSG_EQ (st.GetNDroppedPackets (RemQueueDisc::FORCED_DROP), 0, "There should be zero forced drops");


  // test 4: Packets are ECN capable, but REM queue disc is not ECN enabled
  queue = CreateObject<RemQueueDisc> ();
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("MaxSize", QueueSizeValue (QueueSize (mode, qSize))),
                         true, "Verify that we can actually set the attribute MaxSize");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("Alpha", DoubleValue (0.1)), true,
                         "Verify that we can actually set the attribute Alpha");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("Gamma", DoubleValue (0.1)), true,
                         "Verify that we can actually set the attribute Gamma");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("UpdateInterval", TimeValue (Seconds (0.002))), true,
                         "Verify that we can actually set the attribute UpdateInterval");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("Target", UintegerValue (65)), true,
                         "Verify that we can actually set the attribute Target");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("UseEcn", BooleanValue (false)), true,
                         "Verify that we can actually set the attribute UseECN");
  queue->Initialize ();
  EnqueueWithDelay (queue, pktSize, 600,true);
  DequeueWithDelay (queue,0.012,600);

  Simulator::Stop (Seconds (8.0));
  Simulator::Run ();
  
  st = queue->GetStats ();
  NS_TEST_EXPECT_MSG_NE (st.GetNDroppedPackets (RemQueueDisc::UNFORCED_DROP), 0,
                         "There should be some unforced drops");
  NS_TEST_EXPECT_MSG_EQ (st.GetNMarkedPackets (RemQueueDisc::UNFORCED_MARK), 0,
                         "There should be no unforced marks");


  //test 5: Packets are ECN capable and REM queue disc is ECN enabled
  queue = CreateObject<RemQueueDisc> ();
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("MaxSize", QueueSizeValue (QueueSize (mode, qSize))),
                         true, "Verify that we can actually set the attribute MaxSize");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("Alpha", DoubleValue (0.1)), true,
                         "Verify that we can actually set the attribute Alpha");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("Gamma", DoubleValue (0.1)), true,
                         "Verify that we can actually set the attribute Gamma");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("UpdateInterval", TimeValue (Seconds (0.002))), true,
                         "Verify that we can actually set the attribute UpdateInterval");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("Target", UintegerValue (65)), true,
                         "Verify that we can actually set the attribute Target");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("UseEcn", BooleanValue (true)), true,
                         "Verify that we can actually set the attribute UseECN");
  queue->Initialize ();
  EnqueueWithDelay (queue, pktSize, 600,true);
  DequeueWithDelay (queue,0.012,600);
  Simulator::Stop (Seconds (8.0));
  Simulator::Run ();

  st = queue->GetStats ();
  NS_TEST_EXPECT_MSG_EQ (st.GetNDroppedPackets (RemQueueDisc::UNFORCED_DROP), 0,
                         "There should be no unforced drops");
  NS_TEST_EXPECT_MSG_NE (st.GetNMarkedPackets (RemQueueDisc::UNFORCED_MARK), 0,
                         "There should be some unforced marks");

}

void
RemQueueDiscTestCase::Enqueue (Ptr<RemQueueDisc> queue, uint32_t size, uint32_t nPkt,bool ecnCapable)
{
  Address dest;
  for (uint32_t i = 0; i < nPkt; i++)
    {
      queue->Enqueue (Create<RemQueueDiscTestItem> (Create<Packet> (size), dest/*, 0*/, ecnCapable));
    }
}

void
RemQueueDiscTestCase::EnqueueWithDelay (Ptr<RemQueueDisc> queue, uint32_t size, uint32_t nPkt,bool ecnCapable)
{
  Address dest;
  double delay = 0.01;  // enqueue packets with delay
  for (uint32_t i = 0; i < nPkt; i++)
    {
      Simulator::Schedule (Time (Seconds ((i + 1) * delay)), &RemQueueDiscTestCase::Enqueue, this, queue, size, 1, ecnCapable);
    }
}

void
RemQueueDiscTestCase::Dequeue (Ptr<RemQueueDisc> queue, uint32_t nPkt)
{
  for (uint32_t i = 0; i < nPkt; i++)
    {
      Ptr<QueueDiscItem> item = queue->Dequeue ();
    }
}

void
RemQueueDiscTestCase::DequeueWithDelay (Ptr<RemQueueDisc> queue, double delay, uint32_t nPkt)
{
  for (uint32_t i = 0; i < nPkt; i++)
    {
      Simulator::Schedule (Time (Seconds ((i + 1) * delay)), &RemQueueDiscTestCase::Dequeue, this, queue, 1);
    }
}

void
RemQueueDiscTestCase::DoRun (void)
{
  RunRemTest (QueueSizeUnit::PACKETS);
  RunRemTest (QueueSizeUnit::BYTES);
  Simulator::Destroy ();
}

/**
 * \ingroup traffic-control-test
 * \ingroup tests
 *
 * \brief Rem Queue Disc Test Suite
 */
static class RemQueueDiscTestSuite : public TestSuite
{
public:
  RemQueueDiscTestSuite ()
    : TestSuite ("rem-queue-disc", UNIT)
  {
    AddTestCase (new RemQueueDiscTestCase (), TestCase::QUICK);
  }
} g_remQueueTestSuite; ///< the test suite
