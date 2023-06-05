/*
 * Copyright 2019-2020 NXP
 * Copyright 2023 PHYTEC Messtechnik GmbH
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include "clock_config.h"
#include "fsl_audiomix.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* Fractional PLLs: Fout = ((mainDiv+dsm/65536) * refSel) / (preDiv * 2^ postDiv) */
/* Integer PLLs: Fout = (mainDiv * refSel) / (preDiv * 2^ postDiv) */
/* SYSTEM PLL1 configuration */
const ccm_analog_integer_pll_config_t g_sysPll1Config = {
    .refSel  = kANALOG_PllRefOsc24M, /*!< PLL reference OSC24M */
    .mainDiv = 400U,
    .preDiv  = 3U,
    .postDiv = 2U, /*!< SYSTEM PLL1 frequency  = 800MHZ */
};

/* SYSTEM PLL2 configuration */
const ccm_analog_integer_pll_config_t g_sysPll2Config = {
    .refSel  = kANALOG_PllRefOsc24M, /*!< PLL reference OSC24M */
    .mainDiv = 250U,
    .preDiv  = 3U,
    .postDiv = 1U, /*!< SYSTEM PLL2 frequency  = 1000MHZ */
};

/* SYSTEM PLL3 configuration */
const ccm_analog_integer_pll_config_t g_sysPll3Config = {
    .refSel  = kANALOG_PllRefOsc24M, /*!< PLL reference OSC24M */
    .mainDiv = 300,
    .preDiv  = 3U,
    .postDiv = 2U, /*!< SYSTEM PLL3 frequency  = 600MHZ */
};

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
void BOARD_BootClockRUN(void)
{
    /* * The following steps just show how to configure the PLL clock sources using the clock driver on M7 core side .
     * Please note that the ROM has already configured the SYSTEM PLL1 to 800Mhz when power up the SOC, meanwhile A core
     * would enable SYSTEM PLL1, SYSTEM PLL2 and SYSTEM PLL3 by U-Boot.
     * Therefore, there is no need to configure the system PLL again on M7 side, otherwise it would have a risk to make
     * the SOC hang.
     */

    /* switch AHB NOC root to 24M first in order to configure the SYSTEM PLL1. */
    CLOCK_SetRootMux(kCLOCK_RootAhb, kCLOCK_AhbRootmuxOsc24M);

    /* switch AXI M7 root to 24M first in order to configure the SYSTEM PLL2. */
    CLOCK_SetRootMux(kCLOCK_RootM7, kCLOCK_M7RootmuxOsc24M);

    // CLOCK_InitSysPll2(&g_sysPll2Config); /* init SYSTEM PLL2 run at 1000MHZ */
    // CLOCK_InitSysPll3(&g_sysPll3Config); /* init SYSTEM PLL3 run at 600MHZ */

    CLOCK_SetRootDivider(kCLOCK_RootM7, 1U, 1U);              /* Set root clock to 800M */
    CLOCK_SetRootMux(kCLOCK_RootM7, kCLOCK_M7RootmuxSysPll1); /* switch cortex-m7 to SYSTEM PLL1 */

    // CLOCK_SetRootDivider(kCLOCK_RootQspi, 1U, 2U);              /* Set root clock to 800M */
    // CLOCK_SetRootMux(kCLOCK_RootM7, kCLOCK_M7RootmuxSysPll1); /* switch QSPI to SYSTEM PLL1 */

    CLOCK_SetRootDivider(kCLOCK_RootAhb, 1U, 1U);                   /* Set root clock freq to 133M / 1= 133MHZ */
    CLOCK_SetRootMux(kCLOCK_RootAhb, kCLOCK_AhbRootmuxSysPll1Div6); /* switch AHB to SYSTEM PLL1 DIV6 */

    CLOCK_SetRootMux(kCLOCK_RootUart4, kCLOCK_UartRootmuxSysPll1Div10); /* Set UART source to SysPLL1 Div10 80MHZ */
    CLOCK_SetRootDivider(kCLOCK_RootUart4, 1U, 1U);                     /* Set root clock to 80MHZ/ 1= 80MHZ */

    CLOCK_EnableClock(kCLOCK_Rdc);   /* Enable RDC clock */
    CLOCK_EnableClock(kCLOCK_Ocram); /* Enable Ocram clock */

    /* The purpose to enable the following modules clock is to make sure the M7 core could work normally when A53 core
     * enters the low power status.*/
    CLOCK_EnableClock(kCLOCK_Sim_m);
    CLOCK_EnableClock(kCLOCK_Sim_main);
    CLOCK_EnableClock(kCLOCK_Sim_s);
    CLOCK_EnableClock(kCLOCK_Sim_wakeup);
    CLOCK_EnableClock(kCLOCK_Debug);
    CLOCK_EnableClock(kCLOCK_Dram);
    CLOCK_EnableClock(kCLOCK_Sec_Debug);

    /* Update core clock */
    SystemCoreClockUpdate();
}
