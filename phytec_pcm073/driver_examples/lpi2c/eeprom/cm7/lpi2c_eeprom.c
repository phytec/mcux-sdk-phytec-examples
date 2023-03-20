/*
 * Copyright 2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*  Standard C Included Files */
#include <stdio.h>
#include <string.h>
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "fsl_lpi2c.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define EXAMPLE_I2C_MASTER_BASE (LPI2C2_BASE)

/* Get frequency of lpi2c clock */
#define LPI2C_CLOCK_FREQUENCY (CLOCK_GetFreq(kCLOCK_OscRc48MDiv2))

#define LPI2C_MASTER_CLOCK_FREQUENCY LPI2C_CLOCK_FREQUENCY
#define WAIT_TIME                    10U

#define EXAMPLE_I2C_MASTER ((LPI2C_Type *)EXAMPLE_I2C_MASTER_BASE)

#define LPI2C_MASTER_SLAVE_ADDR_7BIT 0x50U		// 0x50U addressing the memory array, 0x58U accessing the Identification page
#define LPI2C_BAUDRATE               100000U
#define LPI2C_DATA_LENGTH            3U			// Byte Write/Read

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

uint8_t g_master_txBuff[LPI2C_DATA_LENGTH];
uint8_t g_master_rxBuff[LPI2C_DATA_LENGTH];

/*******************************************************************************
 * Code
 ******************************************************************************/

status_t EEPROM_ByteWrite(uint8_t *addr0, uint8_t *addr1)
{
    status_t reVal        = kStatus_Fail;
    size_t txCount        = 0xFFU;

    /* Send master blocking data to slave */
    if (kStatus_Success == LPI2C_MasterStart(EXAMPLE_I2C_MASTER, LPI2C_MASTER_SLAVE_ADDR_7BIT, kLPI2C_Write))
    {
        /* Check master tx FIFO empty or not */
        LPI2C_MasterGetFifoCounts(EXAMPLE_I2C_MASTER, NULL, &txCount);
        while (txCount)
        {
            LPI2C_MasterGetFifoCounts(EXAMPLE_I2C_MASTER, NULL, &txCount);
        }
        /* Check communicate with slave successful or not */
        if (LPI2C_MasterGetStatusFlags(EXAMPLE_I2C_MASTER) & kLPI2C_MasterNackDetectFlag)
        {
            return kStatus_LPI2C_Nak;
        }

        reVal = LPI2C_MasterSend(EXAMPLE_I2C_MASTER, addr0, 1);
        if (reVal != kStatus_Success)
        {
            if (reVal == kStatus_LPI2C_Nak)
            {
                LPI2C_MasterStop(EXAMPLE_I2C_MASTER);
            }
            return -1;
        }

        reVal = LPI2C_MasterSend(EXAMPLE_I2C_MASTER, addr1, 1);
        if (reVal != kStatus_Success)
        {
            if (reVal == kStatus_LPI2C_Nak)
            {
                LPI2C_MasterStop(EXAMPLE_I2C_MASTER);
            }
            return -1;
        }

        reVal = LPI2C_MasterSend(EXAMPLE_I2C_MASTER, g_master_txBuff, 1);
        if (reVal != kStatus_Success)
        {
            if (reVal == kStatus_LPI2C_Nak)
            {
                LPI2C_MasterStop(EXAMPLE_I2C_MASTER);
            }
            return -1;
        }

        reVal = LPI2C_MasterStop(EXAMPLE_I2C_MASTER);
        if (reVal != kStatus_Success)
        {
            return -1;
        }
    }

    /* Wait until the slave is ready for transmit, wait time depend on user's case.
       Slave devices that need some time to process received byte or are not ready yet to
       send the next byte, can pull the clock low to signal to the master that it should wait.*/
    for (uint32_t i = 0U; i < WAIT_TIME; i++)
    {
        __NOP();
    }


	return reVal;
}

status_t EEPROM_ByteRead(uint8_t *addr0, uint8_t *addr1)
{
    status_t reVal        = kStatus_Fail;
    size_t txCount        = 0xFFU;

    if (kStatus_Success == LPI2C_MasterStart(EXAMPLE_I2C_MASTER, LPI2C_MASTER_SLAVE_ADDR_7BIT, kLPI2C_Write))
    {
        /* Check master tx FIFO empty or not */
        LPI2C_MasterGetFifoCounts(EXAMPLE_I2C_MASTER, NULL, &txCount);
        while (txCount)
        {
            LPI2C_MasterGetFifoCounts(EXAMPLE_I2C_MASTER, NULL, &txCount);
        }
        /* Check communicate with slave successful or not */
        if (LPI2C_MasterGetStatusFlags(EXAMPLE_I2C_MASTER) & kLPI2C_MasterNackDetectFlag)
        {
            return kStatus_LPI2C_Nak;
        }

        reVal = LPI2C_MasterSend(EXAMPLE_I2C_MASTER, addr0, 1);
        if (reVal != kStatus_Success)
        {
            if (reVal == kStatus_LPI2C_Nak)
            {
                LPI2C_MasterStop(EXAMPLE_I2C_MASTER);
            }
            return -1;
        }

        reVal = LPI2C_MasterSend(EXAMPLE_I2C_MASTER, addr1, 1);
        if (reVal != kStatus_Success)
        {
            if (reVal == kStatus_LPI2C_Nak)
            {
                LPI2C_MasterStop(EXAMPLE_I2C_MASTER);
            }
            return -1;
        }

        reVal = LPI2C_MasterStart(EXAMPLE_I2C_MASTER, LPI2C_MASTER_SLAVE_ADDR_7BIT, kLPI2C_Read);
        if (reVal != kStatus_Success)
        {
            if (reVal == kStatus_LPI2C_Nak)
            {
                LPI2C_MasterStop(EXAMPLE_I2C_MASTER);
            }
            return -1;
        }

        reVal = LPI2C_MasterReceive(EXAMPLE_I2C_MASTER, g_master_rxBuff, 1);
        if (reVal != kStatus_Success)
        {
            if (reVal == kStatus_LPI2C_Nak)
            {
                LPI2C_MasterStop(EXAMPLE_I2C_MASTER);
            }
            return -1;
        }

        reVal = LPI2C_MasterStop(EXAMPLE_I2C_MASTER);
        if (reVal != kStatus_Success)
        {
            return -1;
        }
    }

	return reVal;
}

/*!
 * @brief Main function
 */
int main(void)
{
    lpi2c_master_config_t masterConfig;
    status_t reVal        = kStatus_Fail;
    uint8_t addr0 = 0x00U;
    uint8_t addr1 = 0x00U;
    int input = 0x00U;

    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    PRINTF("\r\nLPI2C EEPROM example\r\n");

    /* Set up i2c master to send data to slave*/
    /* First data in txBuff is data length of the transmiting data. */
    g_master_txBuff[0] = 0x2;

    /*
     * masterConfig.debugEnable = false;
     * masterConfig.ignoreAck = false;
     * masterConfig.pinConfig = kLPI2C_2PinOpenDrain;
     * masterConfig.baudRate_Hz = 100000U;
     * masterConfig.busIdleTimeout_ns = 0;
     * masterConfig.pinLowTimeout_ns = 0;
     * masterConfig.sdaGlitchFilterWidth_ns = 0;
     * masterConfig.sclGlitchFilterWidth_ns = 0;
     */
    LPI2C_MasterGetDefaultConfig(&masterConfig);

    /* Change the default baudrate configuration */
    masterConfig.baudRate_Hz = LPI2C_BAUDRATE;

    /* Initialize the LPI2C master peripheral */
    LPI2C_MasterInit(EXAMPLE_I2C_MASTER, &masterConfig, LPI2C_MASTER_CLOCK_FREQUENCY);

    while(1)
    {
    	PRINTF("\r\nPlease select 1 (WRITE) or 2 (READ)\r\n");
    	SCANF("%d\r\n", &input);
    	if(input==1)
    	{
    		PRINTF("\r\nPlease enter addr0:\r\n");
    		SCANF("%d\r\n", &input);
    		addr0 = (uint8_t)input;
    		PRINTF("\r\naddr0: 0x%2x, addr1: 0x%2x\r\n", addr0, addr1);
    		PRINTF("\r\nPlease enter addr1:\r\n");
    		SCANF("%d\r\n", &input);
    		addr1 = (uint8_t)input;
    		PRINTF("\r\naddr0: 0x%2x, addr1: 0x%2x\r\n", addr0, addr1);
    		PRINTF("\r\nPlease enter data:\r\n");
    		SCANF("%d\r\n", &input);
    		g_master_txBuff[0] = (uint8_t)input;
    		PRINTF("data: 0x%2x  ", g_master_txBuff[0]);
    		reVal = EEPROM_ByteWrite(&addr0, &addr1);
    	}
    	else if(input==2)
    	{
    		PRINTF("Receive sent data from slave :");

    		PRINTF("\r\nPlease enter addr0:\r\n");
    		SCANF("%d", &input);
    		addr0 = (uint8_t)input;
    		PRINTF("\r\nPlease enter addr1:\r\n");
    		SCANF("%d", &input);
    		addr1 = (uint8_t)input;
    		PRINTF("\r\naddr0: 0x%2x, addr1: 0x%2x\r\n", addr0, addr1);

    		reVal = EEPROM_ByteRead(&addr0, &addr1);

    		PRINTF("data: 0x%2x  ", g_master_rxBuff[0]);
    		PRINTF("\r\n\r\n");
    	}
    	else
    		PRINTF("Invalid Input\r\n");
    }

    PRINTF("\r\nEnd of LPI2C example .\r\n");
}
