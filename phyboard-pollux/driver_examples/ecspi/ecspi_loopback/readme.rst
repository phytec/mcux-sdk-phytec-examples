Overview
========

The ecspi_loopback demo shows how the ecspi does a loopback transfer, internally.
The ECSPI connects the transmitter and receiver sections internally, and the data shifted out from the
most-significant bit of the shift register is looped back into the least-significant bit of the Shift register.
In this way, a self-test of the complete transmit/receive path can be made. The output pins are not affected,
and the input pins are ignored.

Running the demo
================

If the demo runs successfully, the log below will be printed in the terminal window:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

***ECSPI Loopback Demo***

This demo is a loopback transfer test for ECSPI.
The ECSPI will connect the transmitter and receiver sections internally.
So, there is no need to connect the MOSI and MISO pins.

ECSPI loopback test pass!

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
