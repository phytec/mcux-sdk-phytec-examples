Overview
========

The flexcan_loopback_functional example shows how to use the loopback test mode
to debug your CAN Bus design:

To demonstrates this example, only one board is needed. The example will config
one FlexCAN Message Buffer to Rx Message Buffer and the other FlexCAN Message
Buffer to Tx Message Buffer with same ID. After that, the example will send a
CAN Message from the Tx Message Buffer to the Rx Message Buffer throuth internal
loopback interconnect and print out the Message payload to terminal.

Board settings
==============

A modified device tree with flexcan1 node disabled is needed when running this
via the remoteproc framework.

Running the demo
================

When the example runs successfully, following information can be seen on the
terminal:

~~~~~~~~~~~~~~~~~~~~~~

==FlexCAN loopback functional example -- Start.==

Send message from MB8 to MB9
tx word0 = 0x0
tx word1 = 0x1
tx word2 = 0x2
tx word3 = 0x3

Received message from MB9
rx word0 = 0x0
rx word1 = 0x1
rx word2 = 0x2
rx word3 = 0x3

==FlexCAN loopback functional example -- Finish.==

~~~~~~~~~~~~~~~~~~~~~~
