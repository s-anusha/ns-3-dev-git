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
 * PORT NOTE: This code was ported from ns-2.36rc1 (queue/rem.h).
 * Most of the comments are also ported from the same.
 */

#ifndef REM_QUEUE_DISC_H
#define REM_QUEUE_DISC_H

#include <queue>
#include "ns3/packet.h"
#include "ns3/queue-disc.h"
#include "ns3/nstime.h"
#include "ns3/boolean.h"
#include "ns3/data-rate.h"
#include "ns3/timer.h"
#include "ns3/event-id.h"
#include "ns3/random-variable-stream.h"

namespace ns3 {

class TraceContainer;
class UniformRandomVariable;

/**
 * \ingroup traffic-control
 *
 * \brief Implements REM Active Queue Management discipline
 */
class RemQueueDisc : public QueueDisc
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  /**
   * \brief RemQueueDisc Constructor
   */
  RemQueueDisc ();

  /**
   * \brief RemQueueDisc Destructor
   */
  virtual ~RemQueueDisc ();

  /**
   * Assign a fixed random variable stream number to the random variables
   * used by this model. Return the number of streams (possibly zero) that
   * have been assigned.
   *
   * \param stream first stream index to use
   * \return the number of stream indices assigned by this model
   */
  int64_t AssignStreams (int64_t stream);

  // Reasons for dropping packets
  static constexpr const char* UNFORCED_DROP = "Unforced drop";  //!< Early probability drops: proactive
  static constexpr const char* FORCED_DROP = "Forced drop";      //!< Drops due to queue limit: reactive
  // Reasons for marking packets
  static constexpr const char* UNFORCED_MARK = "Unforced mark";  //!< Early probability marks
  static constexpr const char* FORCED_MARK = "Forced mark";      //!< Forced marks

protected:
  /**
   * \brief Dispose of the object
   */
  virtual void DoDispose (void);

private:
  virtual bool DoEnqueue (Ptr<QueueDiscItem> item);
  virtual Ptr<QueueDiscItem> DoDequeue (void);
  virtual Ptr<const QueueDiscItem> DoPeek (void) const;
  virtual bool CheckConfig (void);

  /**
   * \brief Initialize the queue parameters.
   */
  virtual void InitializeParams (void);

  /**
   * \brief Check if a packet needs to be dropped due to probability drop
   * \param item queue item
   * \returns 0 for no drop, 1 for drop
   */
  bool DropEarly (Ptr<QueueDiscItem> item);

  /**
   * Compute the average input rate, the price and the dropping probability
   * Probability is updated periodically after m_updateInterval time
   */
  void RunUpdateRule ();

  // ** Variables supplied by user
  double m_inW;                                 //!< Weight assigned to number of bytes/packets arriving (input rate) during one update time interval.
  double m_phi;                                 //!< Constant for calculation of probability
  uint32_t m_meanPktSize;                       //!< Average packet size in bytes
  Time m_updateInterval;                        //!< Time period after which RunUpdateRule () is called
  uint32_t m_target;                            //!< Target queue length (or target buffer occupancy as mentioned in REM paper)
  double m_gamma;                               //!< Weight assigned to deviation of queue length from target and input rate from capacity
  double m_alpha;                               //!< Weight assigned to difference between current queue length and m_target
  double m_ptc;                                 //!< Bandwidth in packets per second
  DataRate m_linkBandwidth;                     //!< Link bandwidth
  bool m_useEcn;                                //!< True if ECN is used (packets are marked instead of being dropped)

  // ** Variables maintained by REM
  double m_linkPrice;                           //!< Variable to compute the link price
  double m_dropProb;                            //!< Probability of packet dropping
  double m_inputRate;                           //!< Variable used in computing the input rate
  double m_avgInputRate;                        //!< Variable to store the average input rate
  uint32_t m_count;                             //!< Number of bytes or packets arriving at the link during each update time interval
  uint32_t m_countInBytes;                      //!< Queue length in bytes

  EventId m_rtrsEvent;                          //!< Event used to decide the decision of interval of drop probability calculation
  Ptr<UniformRandomVariable> m_uv;              //!< Rng stream
};

}    // namespace ns3

#endif // REM_QUEUE_DISC_H
