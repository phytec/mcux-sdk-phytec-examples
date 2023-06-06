Overview
========
The PWM project is a simple demonstration program of the SDK PWM driver. It sets
up the PWM hardware block to output PWM signals on one PWM channel. The example
also shows PWM duty cycle is increase or decrease. The FIFO empty interrupt is
provided. A new value will be loaded into FIFO when FIFO empty status bit is
set. The PWM will run at the last set duty-cycle setting if all the values of
the FIFO has been utilized, until the FIFO is reloaded or the PWM is disabled.
When a new value is written, the duty cycle changes after the current period is
over. The outputs can be observed by oscilloscope.

Running the demo
================

These instructions are displayed/shown on the terminal window:

~~~~~~~~~~~~~~~~~~~~~~~

PWM driver example.

~~~~~~~~~~~~~~~~~~~~~~~

The blue LED (D11) on the phyBOARD-Polis is used to show the PWM output signal.
