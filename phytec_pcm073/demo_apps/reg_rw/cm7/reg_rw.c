/*
 * Copyright (c) 2013 - 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/


/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief Main function
 */
int main(void)
{
    char ch;
    uint32_t *regVal, regData;

    /* Init board hardware. */
    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    PRINTF("reg_rw example\r\n");

    while (1)
    {
    	PRINTF("Please select 1) Enable LDE_SNVS_DIG or 2) Other registers\r\n");
        ch = GETCHAR();
        if(ch=='1')
        {
        	regVal = 0x40C84000 + 0x540;
        	PRINTF("PMU_LDO_SNVS_DIG_REGISTER (PMU_LDO_SNVS_DIG)\r\n");
            PRINTF("PMU_LDO_SNVS_DIG base address: 0x40C8 4000\r\n");
            PRINTF("PMU_LDO_SNVS_DIG Offset 540h\r\n");
            regData = *regVal;
            PRINTF("Before ==> 0x%x\r\n", regData);
            *regVal = *regVal | 0x7;
            regData = *regVal;
            PRINTF("After ==> 0x%x\r\n", regData);
        }
        else if(ch=='2')
        {
            PRINTF("Please select 1) Read or 2) Write\r\n");
            ch = GETCHAR();
            if(ch=='1')
            {
            	PRINTF("Please enter register value in hex format (no '0x')\r\n");
            	SCANF("%x", &regData);
                regVal = regData;
                PRINTF("0x%x (addr) ==> 0x%x (data)\r\n", regVal, *regVal);
            }
            else if(ch=='2')
            {
            	PRINTF("Please enter register value in hex format (no '0x')\r\n");
            	SCANF("%x", &regData);
                regVal = regData;
                PRINTF("0x%x (addr) ==> 0x%x (data)\r\n", regVal, *regVal);
            	PRINTF("Please enter data value in hex format (no '0x')\r\n");
            	SCANF("%x", &regData);
                *regVal = regData;
                PRINTF("0x%x (addr) ==> 0x%x (data)\r\n", regVal, *regVal);
            }
            else
            	PRINTF("Invalid input\r\n");
        }
        else
        	PRINTF("Invalid input\r\n");
    }
}
