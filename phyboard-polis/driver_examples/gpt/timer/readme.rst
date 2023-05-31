Overview
========
The gpt_timer project is a simple demonstration program of the SDK GPT driver.
It sets up the GPT hardware block to trigger a periodic interrupt after every 1
second. When the GPT interrupt is triggered a message a printed on the UART
terminal.

Running the demo
================
When the example runs successfully, following information can be seen on the
terminal:

~~~~~~~~~~~~~~~~~~~~~

Press any key to start the example
Starting GPT timer ...
 GPT interrupt occurred!
 GPT interrupt occurred!
 GPT interrupt occurred!
 GPT interrupt occurred!
 .
 .
 .
 GPT interrupt occurred!
 .
 .
 .

~~~~~~~~~~~~~~~~~~~~~
