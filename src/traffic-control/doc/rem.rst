.. include:: replace.txt
.. highlight:: cpp

REM queue disc
----------------

This chapter describes the REM ([Athuraliya]_) queue disc implementation 
in |ns3|. 

Random Exponential Marking (REM) is a queuing discipline that aims to achieve high utilization, negligible loss and delay in
a simple and scalable manner. The model in ns-3 is a port of Sanjeewa Athuraliya's ns-2 REM model.


Model Description
*****************

The source code for the REM model is located in the directory ``src/traffic-control/model``
and consists of 2 files `rem-queue-disc.h` and `rem-queue-disc.cc` defining a RemQueueDisc
class. The code was ported to |ns3| by Aparna R. Joshi, Isha Tarte and Navya R. S.
based on ns-2 code implemented by Sanjeewa Athuraliya.

* class :cpp:class:`RemQueueDisc`: This class implements the main REM algorithm:

  * ``RemQueueDisc::DoEnqueue ()``: This routine checks whether the queue is full, and if so, drops the packets and records the number of drops due to queue overflow. If queue is not full, this routine calls ``RemQueueDisc::DropEarly()``, and depending on the value returned, the incoming packet is either enqueued or dropped.

  * ``RemQueueDisc::DropEarly ()``: The decision to enqueue or drop the packet is taken by invoking this routine, which returns a boolean value; false indicates enqueue and true indicates drop.

  * ``RemQueueDisc::RunUpdateRule ()``: This routine is called at a regular interval of `m_updateInterval` and updates the drop probability, which is required by ``RemQueueDisc::DropEarly()``

  * ``RemQueueDisc::DoDequeue ()``: This routine removes the packet from the queue.`  

References
==========

.. [Athuraliya] Athuraliya, S., Low, S. H., Li, V. H., & Yin, Q. (2001). REM: Active queue management. IEEE network, 15(3), 48-53. Available online at `<http://ieeexplore.ieee.org/stamp/stamp.jsp?arnumber=923940&tag=1>`_.

Attributes
==========

The key attributes that the RemQueueDisc class holds include the following: 

* ``Mode:`` REM operating mode (BYTES or PACKETS). The default mode is PACKETS. 
* ``QueueLimit:`` The maximum number of bytes or packets the queue can hold. The default value is 50 bytes / packets.
* ``MeanPktSize:`` Mean packet size in bytes. The default value is 1000 bytes.
* ``UpdateInterval:`` Time period to calculate drop probability. The default value is 2 ms. 
* ``InputWeight:`` Weight assigned to input rate. The default value is 1.0. 
* ``Phi:`` Value of Phi used to calculate probability. The default value is 1.001. 
* ``Target:`` Target queue length. The default value is 20 packets.
* ``Alpha:`` Value of Alpha. The default value is 0.1.
* ``Gamma:`` Value of Beta. The default value is 0.001.
* ``LinkBandwidth:`` The REM link bandwidth. The default value is 1.5 Mbps.

Examples
========

The example for REM is `rem-example.cc` located in ``src/examples/traffic-control``.  To run the file (the first invocation below shows the available
command-line options):

:: 

   $ ./waf --run "rem-example --PrintHelp"
   $ ./waf --run "rem-example --writePcap=1" 

The expected output from the previous commands are 10 .pcap files.

Validation
**********

The REM model is tested using :cpp:class:`RemQueueDiscTestSuite` class defined in `src/traffic-control/test/rem-queue-test-suite.cc`. The suite includes 3 test cases:

* Test 1: simple enqueue/dequeue with defaults, no drops
* Test 2: more data with defaults, unforced drops but no forced drops
* Test 3: same as test 2, but with higher Target

The test suite can be run using the following commands: 

::

  $ ./waf configure --enable-examples --enable-tests
  $ ./waf build
  $ ./test.py -s rem-queue-disc

or  

::

  $ NS_LOG="RemQueueDisc" ./waf --run "test-runner --suite=rem-queue-disc"


