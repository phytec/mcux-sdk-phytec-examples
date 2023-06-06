Overview
========
The i2c_interrupt_b2b_transfer_slave example shows how to use i2c driver as slave to do board to board transfer 
with interrupt:

In this example, one i2c instance as slave and another i2c instance on the other board as master. Master sends a 
piece of data to slave, and receive a piece of data from slave. This example checks if the data received from 
slave is correct.

Board settings
==============
  - Transfer data from MASTER_BOARD to SLAVE_BOARD of I2C interface, I2C4 pins of MASTER_BOARD are connected with
    I2C4 pins of SLAVE_BOARD
  - Remove "imx8mp-phyboard-pollux-peb-av-010.dtbo" from the bootenv.txt
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SLAVE_BOARD        CONNECTS TO          MASTER_BOARD
Pin Name   Board Location     Pin Name   Board Location
I2C4_SCL       X15-24         I2C4_SCL      X15-24
I2C4_SDA       X15-22         I2C4_SDA      X15-22
GND            X15-21         GND           X15-21
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Running the demo
================
When the demo runs successfully, the following message is displayed in the terminal:

I2C board2board interrupt example -- Slave transfer.


Slave received data :
0x 0  0x 1  0x 2  0x 3  0x 4  0x 5  0x 6  0x 7
0x 8  0x 9  0x a  0x b  0x c  0x d  0x e  0x f
0x10  0x11  0x12  0x13  0x14  0x15  0x16  0x17
0x18  0x19  0x1a  0x1b  0x1c  0x1d  0x1e  0x1f


End of I2C example .
