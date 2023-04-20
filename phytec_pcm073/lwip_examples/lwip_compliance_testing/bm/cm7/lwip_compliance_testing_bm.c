/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2020 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "lwip/opt.h"

#if LWIP_IPV4 && LWIP_DHCP

#include "lwip/timeouts.h"
#include "lwip/ip_addr.h"
#include "lwip/init.h"
#include "lwip/dhcp.h"
#include "lwip/prot/dhcp.h"
#include "netif/ethernet.h"
#include "ethernetif.h"

#ifndef configMAC_ADDR
#include "fsl_silicon_id.h"
#endif
#include "fsl_phy.h"
#include "pin_mux.h"
#include "board.h"

#include "fsl_phyksz8081.h"
#include "fsl_phydp83867ir.h"
#include "fsl_enet.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* @TEST_ANCHOR */

/* IP address configuration. */
#ifndef configIP_ADDR0
#define configIP_ADDR0 192
#endif
#ifndef configIP_ADDR1
#define configIP_ADDR1 168
#endif
#ifndef configIP_ADDR2
#define configIP_ADDR2 0
#endif
#ifndef configIP_ADDR3
#define configIP_ADDR3 102
#endif

/* Netmask configuration. */
#ifndef configNET_MASK0
#define configNET_MASK0 255
#endif
#ifndef configNET_MASK1
#define configNET_MASK1 255
#endif
#ifndef configNET_MASK2
#define configNET_MASK2 255
#endif
#ifndef configNET_MASK3
#define configNET_MASK3 0
#endif

/* Gateway address configuration. */
#ifndef configGW_ADDR0
#define configGW_ADDR0 192
#endif
#ifndef configGW_ADDR1
#define configGW_ADDR1 168
#endif
#ifndef configGW_ADDR2
#define configGW_ADDR2 0
#endif
#ifndef configGW_ADDR3
#define configGW_ADDR3 100
#endif

extern phy_ksz8081_resource_t g_phy0_resource;
#define EXAMPLE_ENET0 ENET
/* Address of PHY interface. */
#define EXAMPLE_PHY0_ADDRESS BOARD_ENET0_PHY_ADDRESS
/* PHY operations. */
#define EXAMPLE_PHY0_OPS &phyksz8081_ops
/* ENET instance select. */
#define EXAMPLE_NETIF0_INIT_FN ethernetif0_init

extern phy_dp83867ir_resource_t g_phy1_resource;
#define EXAMPLE_ENET1          ENET_1G
/* Address of PHY interface. */
#define EXAMPLE_PHY1_ADDRESS   BOARD_ENET1_PHY_ADDRESS
/* PHY operations. */
#define EXAMPLE_PHY1_OPS       &phydp83867ir_ops
/* ENET instance select. */
#define EXAMPLE_NETIF1_INIT_FN ethernetif1_init

/* PHY resource. */
#define EXAMPLE_PHY0_RESOURCE &g_phy0_resource
#define EXAMPLE_PHY1_RESOURCE &g_phy1_resource

/* ENET clock frequency. */
#define EXAMPLE_CLOCK_FREQ CLOCK_GetRootClockFreq(kCLOCK_Root_Bus)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
phy_ksz8081_resource_t g_phy0_resource;
phy_dp83867ir_resource_t g_phy1_resource;

static phy_handle_t phyHandle;

int ENET_100M = 0;

/*******************************************************************************
 * Code
 ******************************************************************************/
void BOARD_InitModuleClock(void)
{
    const clock_sys_pll1_config_t sysPll1Config = {
        .pllDiv2En = true,
    };
    CLOCK_InitSysPll1(&sysPll1Config);

    if(ENET_100M==1)
    {
    	clock_root_config_t rootCfg = {.mux = 4, .div = 10}; /* Generate 50M root clock. */
    	CLOCK_SetRootClock(kCLOCK_Root_Enet1, &rootCfg);
    }
    else
    {
    	clock_root_config_t rootCfg = {.mux = 4, .div = 4}; /* Generate 125M root clock. */
    	CLOCK_SetRootClock(kCLOCK_Root_Enet2, &rootCfg);
    }
}

void IOMUXC_SelectENETClock(void)
{
	if(ENET_100M==1)
		IOMUXC_GPR->GPR4 |= IOMUXC_GPR_GPR4_ENET_REF_CLK_DIR_MASK; /* 50M ENET_REF_CLOCK output to PHY and ENET module. */
	else
		IOMUXC_GPR->GPR5 |= IOMUXC_GPR_GPR5_ENET1G_RGMII_EN_MASK; /* bit1:iomuxc_gpr_enet_clk_dir
                                                                 bit0:GPR_ENET_TX_CLK_SEL(internal or OSC) */
}

void BOARD_ENETFlexibleConfigure(enet_config_t *config)
{
	if(ENET_100M==1)
		config->miiMode = kENET_RmiiMode;
	else
		config->miiMode = kENET_RgmiiMode;
}

static void MDIO_Init(void)
{
	if(ENET_100M==1)
	{
		(void)CLOCK_EnableClock(s_enetClock[ENET_GetInstance(EXAMPLE_ENET0)]);
		ENET_SetSMI(EXAMPLE_ENET0, EXAMPLE_CLOCK_FREQ, false);
	}
	else
	{
		(void)CLOCK_EnableClock(s_enetClock[ENET_GetInstance(EXAMPLE_ENET1)]);
		ENET_SetSMI(EXAMPLE_ENET1, EXAMPLE_CLOCK_FREQ, false);
	}
}

static status_t MDIO_Write(uint8_t phyAddr, uint8_t regAddr, uint16_t data)
{
	if(ENET_100M==1)
		return ENET_MDIOWrite(EXAMPLE_ENET0, phyAddr, regAddr, data);
	else
		return ENET_MDIOWrite(EXAMPLE_ENET1, phyAddr, regAddr, data);
}

static status_t MDIO_Read(uint8_t phyAddr, uint8_t regAddr, uint16_t *pData)
{
	if(ENET_100M==1)
		return ENET_MDIORead(EXAMPLE_ENET0, phyAddr, regAddr, pData);
	else
		return ENET_MDIORead(EXAMPLE_ENET1, phyAddr, regAddr, pData);
}


/*!
 * @brief Interrupt service for SysTick timer.
 */
void SysTick_Handler(void)
{
    time_isr();
}

/*!
 * @brief Prints DHCP status of the interface when it has changed from last status.
 *
 * @param netif network interface structure
 */
static void print_dhcp_state(struct netif *netif)
{
    static u8_t dhcp_last_state = DHCP_STATE_OFF;
    struct dhcp *dhcp           = netif_dhcp_data(netif);

    if (dhcp == NULL)
    {
        dhcp_last_state = DHCP_STATE_OFF;
    }
    else if (dhcp_last_state != dhcp->state)
    {
        dhcp_last_state = dhcp->state;

        PRINTF(" DHCP state       : ");
        switch (dhcp_last_state)
        {
            case DHCP_STATE_OFF:
                PRINTF("OFF");
                break;
            case DHCP_STATE_REQUESTING:
                PRINTF("REQUESTING");
                break;
            case DHCP_STATE_INIT:
                PRINTF("INIT");
                break;
            case DHCP_STATE_REBOOTING:
                PRINTF("REBOOTING");
                break;
            case DHCP_STATE_REBINDING:
                PRINTF("REBINDING");
                break;
            case DHCP_STATE_RENEWING:
                PRINTF("RENEWING");
                break;
            case DHCP_STATE_SELECTING:
                PRINTF("SELECTING");
                break;
            case DHCP_STATE_INFORMING:
                PRINTF("INFORMING");
                break;
            case DHCP_STATE_CHECKING:
                PRINTF("CHECKING");
                break;
            case DHCP_STATE_BOUND:
                PRINTF("BOUND");
                break;
            case DHCP_STATE_BACKING_OFF:
                PRINTF("BACKING_OFF");
                break;
            default:
                PRINTF("%u", dhcp_last_state);
                assert(0);
                break;
        }
        PRINTF("\r\n");

        if (dhcp_last_state == DHCP_STATE_BOUND)
        {
            PRINTF("\r\n IPv4 Address     : %s\r\n", ipaddr_ntoa(&netif->ip_addr));
            PRINTF(" IPv4 Subnet mask : %s\r\n", ipaddr_ntoa(&netif->netmask));
            PRINTF(" IPv4 Gateway     : %s\r\n\r\n", ipaddr_ntoa(&netif->gw));
        }
    }
}

/* command states */
typedef enum {
  CMD_WAITING				= 0,
  CMD_TEST_MODE_1			= 1,
  CMD_TEST_MODE_2			= 2,
  CMD_TEST_MODE_2T			= 21,
  CMD_TEST_MODE_3			= 3,
  CMD_TEST_MODE_3T			= 31,
  CMD_TEST_MODE_4			= 4,
  CMD_TEST_MODE_4A			= 41,
  CMD_TEST_MODE_4B			= 42,
  CMD_TEST_MODE_4C			= 43,
  CMD_TEST_MODE_4D			= 44,
  CMD_TEST_MODE_5_MDI		= 51,
  CMD_TEST_MODE_5_MDIX		= 52,
  CMD_LINK_PULSE			= 6,
  CMD_STANDARD				= 7,
  CMD_CUSTOM_WRITE			= 81,
  CMD_CUSTOM_READ			= 82,
  CMD_END					= 9
} cmd_state_enum_t;

int ksz8081rnb_compliance_testing(phy_handle_t *phyHandle)
{
	u8_t cmd = CMD_WAITING;
	uint16_t RegVal = 0;
	u8_t input_done = 0;
	uint16_t RegData = 0;

	while(cmd!=CMD_END)
	{
		PRINTF("\r\nCompliance Testing for KSZ8081RNB\r\n");
		PRINTF("\r\nPlease enter Test Mode:\r\n");
		PRINTF("1: 1000 Base Test Mode 1\r\n");
		PRINTF("2: 1000 Base Test Mode 2\r\n");
		PRINTF("3: 1000 Base Test Mode 3\r\n");
		PRINTF("4: 1000 Base Test Mode 4\r\n");
		PRINTF("81: Custom Write Mode\r\n");
		PRINTF("82: Custom Read Mode\r\n");
		PRINTF("9: Quit\r\n");

		SCANF("%d", &cmd);
		if(cmd==CMD_TEST_MODE_1)
		{
			PRINTF("\r\n1: 1000 Base Test Mode 1 is selected\r\n");
            phyHandle->ops->phyWrite(phyHandle, PHY_BASICCONTROL_REG, 0x0140);	// Reg 0x0000 = 0x0140
            PRINTF("Reg 0x%x <= Data 0x%x\r\n", PHY_BASICCONTROL_REG, 0x0140);
            phyHandle->ops->phyWrite(phyHandle, PHY_1000BASET_CONTROL_REG, 0x3B00);	// Reg 0x0009 = 0x3B00
            PRINTF("Reg 0x%x <= Data 0x%x\r\n", PHY_1000BASET_CONTROL_REG, 0x3B00);

            phyHandle->ops->phyRead(phyHandle, PHY_BASICCONTROL_REG, &RegData);
            PRINTF("Reg 0x%x:\t0x%x\r\n", PHY_BASICCONTROL_REG, RegData);
            phyHandle->ops->phyRead(phyHandle, PHY_1000BASET_CONTROL_REG, &RegData);
            PRINTF("Reg 0x%x:\t0x%x\r\n", PHY_1000BASET_CONTROL_REG, RegData);
		}
		else if(cmd==CMD_TEST_MODE_2)
		{
			PRINTF("\r\n2: 1000 Base Test Mode 2 is selected\r\n");
            phyHandle->ops->phyWrite(phyHandle, PHY_BASICCONTROL_REG, 0x0140);	// Reg 0x0000 = 0x0140
            PRINTF("Reg 0x%x <= Data 0x%x\r\n", PHY_BASICCONTROL_REG, 0x0140);
            phyHandle->ops->phyWrite(phyHandle, PHY_1000BASET_CONTROL_REG, 0x5B00);	// Reg 0x0009 = 0x5B00
            PRINTF("Reg 0x%x <= Data 0x%x\r\n", PHY_1000BASET_CONTROL_REG, 0x5B00);

            phyHandle->ops->phyRead(phyHandle, PHY_BASICCONTROL_REG, &RegData);
            PRINTF("Reg 0x%x:\t0x%x\r\n", PHY_BASICCONTROL_REG, RegData);
            phyHandle->ops->phyRead(phyHandle, PHY_1000BASET_CONTROL_REG, &RegData);
            PRINTF("Reg 0x%x:\t0x%x\r\n", PHY_1000BASET_CONTROL_REG, RegData);
		}
		else if(cmd==CMD_TEST_MODE_3)
		{
			PRINTF("\r\n3: 1000 Base Test Mode 3 is selected\r\n");
            phyHandle->ops->phyWrite(phyHandle, PHY_BASICCONTROL_REG, 0x0140);	// Reg 0x0000 = 0x0140
            PRINTF("Reg 0x%x <= Data 0x%x\r\n", PHY_BASICCONTROL_REG, 0x0140);
            phyHandle->ops->phyWrite(phyHandle, PHY_1000BASET_CONTROL_REG, 0x7300);	// Reg 0x0009 = 0x7300
            PRINTF("Reg 0x%x <= Data 0x%x\r\n", PHY_1000BASET_CONTROL_REG, 0x7300);

            phyHandle->ops->phyRead(phyHandle, PHY_BASICCONTROL_REG, &RegData);
            PRINTF("Reg 0x%x:\t0x%x\r\n", PHY_BASICCONTROL_REG, RegData);
            phyHandle->ops->phyRead(phyHandle, PHY_1000BASET_CONTROL_REG, &RegData);
            PRINTF("Reg 0x%x:\t0x%x\r\n", PHY_1000BASET_CONTROL_REG, RegData);

		}
		else if(cmd==CMD_TEST_MODE_4)
		{
			PRINTF("\r\n41: 1000 Base Test Mode 4 (Channel A) is selected\r\n");
            phyHandle->ops->phyWrite(phyHandle, PHY_BASICCONTROL_REG, 0x0140);	// Reg 0x0000 = 0x0140
            PRINTF("Reg 0x%x <= Data 0x%x\r\n", PHY_BASICCONTROL_REG, 0x0140);
            phyHandle->ops->phyWrite(phyHandle, PHY_1000BASET_CONTROL_REG, 0x9B00);	// Reg 0x0009 = 0x9B00
            PRINTF("Reg 0x%x <= Data 0x%x\r\n", PHY_1000BASET_CONTROL_REG, 0x9B00);

            phyHandle->ops->phyRead(phyHandle, PHY_BASICCONTROL_REG, &RegData);
            PRINTF("Reg 0x%x:\t0x%x\r\n", PHY_BASICCONTROL_REG, RegData);
            phyHandle->ops->phyRead(phyHandle, PHY_1000BASET_CONTROL_REG, &RegData);
            PRINTF("Reg 0x%x:\t0x%x\r\n", PHY_1000BASET_CONTROL_REG, RegData);

		}
		else if(cmd==CMD_CUSTOM_WRITE)
		{
			PRINTF("\r\n8: Custom Write Mode is selected\r\n");
			while(input_done==0)
			{
				PRINTF("Please enter Reg Value in HEX format (no \"0x\")\r\n");
				SCANF("%x", &RegVal);
				PRINTF("Please enter Reg Data in HEX format (no \"0x\")\r\n");
				SCANF("%x", &RegData);
				PRINTF("Reg 0x%x <= Data 0x%x\r\n", RegVal, RegData);
				phyHandle->ops->phyWrite(phyHandle, RegVal, RegData);
				PRINTF("Continue? (y/n)\r\n");
				SCANF("%c", &RegData);
				if(RegData==110)
					input_done = 1;
			}
			input_done = 0;
		}
		else if(cmd==CMD_CUSTOM_READ)
		{
			PRINTF("\r\n8: Custom Read Mode is selected\r\n");
			while(input_done==0)
			{
				PRINTF("Please enter Reg Value in HEX format (no \"0x\")\r\n");
				SCANF("%x", &RegVal);
				phyHandle->ops->phyRead(phyHandle, RegVal, &RegData);
	            PRINTF("Reg 0x%x:\t0x%x\r\n", RegVal, RegData);
				PRINTF("Continue? (y/n)\r\n");
				SCANF("%c", &RegData);
				if(RegData==110)
					input_done = 1;
			}
			input_done = 0;
		}
		else if(cmd==CMD_END)
		{
			PRINTF("\r\n9: Quit is selected\r\n");

		}
		else
		{
			PRINTF("\r\nInvalid Input\r\n");
		}

	}

	return 0;
}

int dp83867ir_compliance_testing(phy_handle_t *phyHandle)
{
	u8_t cmd = CMD_WAITING;
	uint16_t RegVal = 0;
	u8_t input_done = 0;
	uint16_t RegData = 0;

	while(cmd!=CMD_END)
	{
		PRINTF("\r\nCompliance Testing for DP83867IR\r\n");
		PRINTF("\r\nPlease enter Test Mode:\r\n");
		PRINTF("1: 1000 Base Test Mode 1\r\n");
		PRINTF("2: 1000 Base Test Mode 2\r\n");
		PRINTF("21: 1000 Base Test Mode 2 with TX_TCLK\r\n");
		PRINTF("3: 1000 Base Test Mode 3\r\n");
		PRINTF("31: 1000 Base Test Mode 3 with TX_TCLK\r\n");
		PRINTF("41: 1000 Base Test Mode 4 (Channel A)\r\n");
		PRINTF("42: 1000 Base Test Mode 4 (Channel B)\r\n");
		PRINTF("43: 1000 Base Test Mode 4 (Channel C)\r\n");
		PRINTF("44: 1000 Base Test Mode 4 (Channel D)\r\n");
		PRINTF("51: 100 Base Standard MDI (Test Mode 5)\r\n");
		PRINTF("52: 100 Base Standard MDIX (Test Mode 5)\r\n");
		PRINTF("6: 10 Base Link Pulse\r\n");
		PRINTF("7: 10 Base Standard\r\n");
		PRINTF("81: Custom Write Mode\r\n");
		PRINTF("82: Custom Read Mode\r\n");
		PRINTF("9: Quit\r\n");

		SCANF("%d", &cmd);
		if(cmd==CMD_TEST_MODE_1)
		{
			PRINTF("\r\n1: 1000 Base Test Mode 1 is selected\r\n");
            phyHandle->ops->phyWrite(phyHandle, PHY_CTRL_REG, 0x8000);	// Reg 0x001F = 0x8000
            PRINTF("Reg 0x%x <= Data 0x%x\r\n", PHY_CTRL_REG, 0x8000);
            phyHandle->ops->phyWrite(phyHandle, PHY_BASICCONTROL_REG, 0x0140);	// Reg 0x0000 = 0x0140
            PRINTF("Reg 0x%x <= Data 0x%x\r\n", PHY_BASICCONTROL_REG, 0x0140);
            phyHandle->ops->phyWrite(phyHandle, PHY_PHYCR_REG, 0x5008);	// Reg 0x0010 = 0x5008
            PRINTF("Reg 0x%x <= Data 0x%x\r\n", PHY_PHYCR_REG, 0x5008);
            phyHandle->ops->phyWrite(phyHandle, PHY_1000BASET_CONTROL_REG, 0x3B00);	// Reg 0x0009 = 0x3B00
            PRINTF("Reg 0x%x <= Data 0x%x\r\n", PHY_1000BASET_CONTROL_REG, 0x3B00);
            phyHandle->ops->phyWrite(phyHandle, PHY_TMCH_CTRL_REG, 0x0480);	// Reg 0x0025 = 0x0480
            PRINTF("Reg 0x%x <= Data 0x%x\r\n", PHY_TMCH_CTRL_REG, 0x0480);
            phyHandle->ops->phyWrite(phyHandle, PHY_PROG_GAIN_REG, 0xF508);	// Reg 0x01D5 = 0xF508
            PRINTF("Reg 0x%x <= Data 0x%x\r\n", PHY_PROG_GAIN_REG, 0xF508);

            phyHandle->ops->phyRead(phyHandle, PHY_CTRL_REG, &RegData);
            PRINTF("Reg 0x%x:\t0x%x\r\n", PHY_CTRL_REG, RegData);
            phyHandle->ops->phyRead(phyHandle, PHY_BASICCONTROL_REG, &RegData);
            PRINTF("Reg 0x%x:\t0x%x\r\n", PHY_BASICCONTROL_REG, RegData);
            phyHandle->ops->phyRead(phyHandle, PHY_PHYCR_REG, &RegData);
            PRINTF("Reg 0x%x:\t0x%x\r\n", PHY_PHYCR_REG, RegData);
            phyHandle->ops->phyRead(phyHandle, PHY_1000BASET_CONTROL_REG, &RegData);
            PRINTF("Reg 0x%x:\t0x%x\r\n", PHY_1000BASET_CONTROL_REG, RegData);
            phyHandle->ops->phyRead(phyHandle, PHY_TMCH_CTRL_REG, &RegData);
            PRINTF("Reg 0x%x:\t0x%x\r\n", PHY_TMCH_CTRL_REG, RegData);
            phyHandle->ops->phyRead(phyHandle, PHY_PROG_GAIN_REG, &RegData);
            PRINTF("Reg 0x%x:\t0x%x\r\n", PHY_PROG_GAIN_REG, RegData);
		}
		else if(cmd==CMD_TEST_MODE_2)
		{
			PRINTF("\r\n2: 1000 Base Test Mode 2 is selected\r\n");
            phyHandle->ops->phyWrite(phyHandle, PHY_CTRL_REG, 0x8000);	// Reg 0x001F = 0x8000
            PRINTF("Reg 0x%x <= Data 0x%x\r\n", PHY_CTRL_REG, 0x8000);
            phyHandle->ops->phyWrite(phyHandle, PHY_BASICCONTROL_REG, 0x0140);	// Reg 0x0000 = 0x0140
            PRINTF("Reg 0x%x <= Data 0x%x\r\n", PHY_BASICCONTROL_REG, 0x0140);
            phyHandle->ops->phyWrite(phyHandle, PHY_PHYCR_REG, 0x5008);	// Reg 0x0010 = 0x5008
            PRINTF("Reg 0x%x <= Data 0x%x\r\n", PHY_PHYCR_REG, 0x5008);
            phyHandle->ops->phyWrite(phyHandle, PHY_1000BASET_CONTROL_REG, 0x5B00);	// Reg 0x0009 = 0x5B00
            PRINTF("Reg 0x%x <= Data 0x%x\r\n", PHY_1000BASET_CONTROL_REG, 0x5B00);
            phyHandle->ops->phyWrite(phyHandle, PHY_TMCH_CTRL_REG, 0x0480);	// Reg 0x0025 = 0x0480
            PRINTF("Reg 0x%x <= Data 0x%x\r\n", PHY_TMCH_CTRL_REG, 0x0480);

            phyHandle->ops->phyRead(phyHandle, PHY_CTRL_REG, &RegData);
            PRINTF("Reg 0x%x:\t0x%x\r\n", PHY_CTRL_REG, RegData);
            phyHandle->ops->phyRead(phyHandle, PHY_BASICCONTROL_REG, &RegData);
            PRINTF("Reg 0x%x:\t0x%x\r\n", PHY_BASICCONTROL_REG, RegData);
            phyHandle->ops->phyRead(phyHandle, PHY_PHYCR_REG, &RegData);
            PRINTF("Reg 0x%x:\t0x%x\r\n", PHY_PHYCR_REG, RegData);
            phyHandle->ops->phyRead(phyHandle, PHY_1000BASET_CONTROL_REG, &RegData);
            PRINTF("Reg 0x%x:\t0x%x\r\n", PHY_1000BASET_CONTROL_REG, RegData);
            phyHandle->ops->phyRead(phyHandle, PHY_TMCH_CTRL_REG, &RegData);
            PRINTF("Reg 0x%x:\t0x%x\r\n", PHY_TMCH_CTRL_REG, RegData);
		}
		else if(cmd==CMD_TEST_MODE_2T)
		{
			PRINTF("\r\n21: 1000 Base Test Mode 2 with TX_TCLK is selected\r\n");
            phyHandle->ops->phyWrite(phyHandle, PHY_CTRL_REG, 0x8000);	// Reg 0x001F = 0x8000
            PRINTF("Reg 0x%x <= Data 0x%x\r\n", PHY_CTRL_REG, 0x8000);
            phyHandle->ops->phyWrite(phyHandle, PHY_BASICCONTROL_REG, 0x0140);	// Reg 0x0000 = 0x0140
            PRINTF("Reg 0x%x <= Data 0x%x\r\n", PHY_BASICCONTROL_REG, 0x0140);
            phyHandle->ops->phyWrite(phyHandle, PHY_PHYCR_REG, 0x5008);	// Reg 0x0010 = 0x5008
            PRINTF("Reg 0x%x <= Data 0x%x\r\n", PHY_PHYCR_REG, 0x5008);
            phyHandle->ops->phyWrite(phyHandle, PHY_1000BASET_CONTROL_REG, 0x5B00);	// Reg 0x0009 = 0x5B00
            PRINTF("Reg 0x%x <= Data 0x%x\r\n", PHY_1000BASET_CONTROL_REG, 0x5B00);
            phyHandle->ops->phyWrite(phyHandle, PHY_TMCH_CTRL_REG, 0x0480);	// Reg 0x0025 = 0x0480
            PRINTF("Reg 0x%x <= Data 0x%x\r\n", PHY_TMCH_CTRL_REG, 0x0480);
            phyHandle->ops->phyWrite(phyHandle, PHY_IO_MUX_CFG_REG, 0x81F);	// Reg 0x0170 = 0x81F
            PRINTF("Reg 0x%x <= Data 0x%x\r\n", PHY_IO_MUX_CFG_REG, 0x81F);
            phyHandle->ops->phyWrite(phyHandle, PHY_PLLCTL_REG, 0x0010);	// Reg 0x00C6 = 0x0010
            PRINTF("Reg 0x%x <= Data 0x%x\r\n", PHY_PLLCTL_REG, 0x0010);

            phyHandle->ops->phyRead(phyHandle, PHY_CTRL_REG, &RegData);
            PRINTF("Reg 0x%x:\t0x%x\r\n", PHY_CTRL_REG, RegData);
            phyHandle->ops->phyRead(phyHandle, PHY_BASICCONTROL_REG, &RegData);
            PRINTF("Reg 0x%x:\t0x%x\r\n", PHY_BASICCONTROL_REG, RegData);
            phyHandle->ops->phyRead(phyHandle, PHY_PHYCR_REG, &RegData);
            PRINTF("Reg 0x%x:\t0x%x\r\n", PHY_PHYCR_REG, RegData);
            phyHandle->ops->phyRead(phyHandle, PHY_1000BASET_CONTROL_REG, &RegData);
            PRINTF("Reg 0x%x:\t0x%x\r\n", PHY_1000BASET_CONTROL_REG, RegData);
            phyHandle->ops->phyRead(phyHandle, PHY_TMCH_CTRL_REG, &RegData);
            PRINTF("Reg 0x%x:\t0x%x\r\n", PHY_TMCH_CTRL_REG, RegData);
            phyHandle->ops->phyRead(phyHandle, PHY_IO_MUX_CFG_REG, &RegData);
            PRINTF("Reg 0x%x:\t0x%x\r\n", PHY_IO_MUX_CFG_REG, RegData);
            phyHandle->ops->phyRead(phyHandle, PHY_PLLCTL_REG, &RegData);
            PRINTF("Reg 0x%x:\t0x%x\r\n", PHY_PLLCTL_REG, RegData);
		}
		else if(cmd==CMD_TEST_MODE_3)
		{
			PRINTF("\r\n3: 1000 Base Test Mode 3 is selected\r\n");
            phyHandle->ops->phyWrite(phyHandle, PHY_CTRL_REG, 0x8000);	// Reg 0x001F = 0x8000
            PRINTF("Reg 0x%x <= Data 0x%x\r\n", PHY_CTRL_REG, 0x8000);
            phyHandle->ops->phyWrite(phyHandle, PHY_BASICCONTROL_REG, 0x0140);	// Reg 0x0000 = 0x0140
            PRINTF("Reg 0x%x <= Data 0x%x\r\n", PHY_BASICCONTROL_REG, 0x0140);
            phyHandle->ops->phyWrite(phyHandle, PHY_PHYCR_REG, 0x5008);	// Reg 0x0010 = 0x5008
            PRINTF("Reg 0x%x <= Data 0x%x\r\n", PHY_PHYCR_REG, 0x5008);
            phyHandle->ops->phyWrite(phyHandle, PHY_1000BASET_CONTROL_REG, 0x7B00);	// Reg 0x0009 = 0x7B00
            PRINTF("Reg 0x%x <= Data 0x%x\r\n", PHY_1000BASET_CONTROL_REG, 0x7B00);
            phyHandle->ops->phyWrite(phyHandle, PHY_TMCH_CTRL_REG, 0x0480);	// Reg 0x0025 = 0x0480
            PRINTF("Reg 0x%x <= Data 0x%x\r\n", PHY_TMCH_CTRL_REG, 0x0480);

            phyHandle->ops->phyRead(phyHandle, PHY_CTRL_REG, &RegData);
            PRINTF("Reg 0x%x:\t0x%x\r\n", PHY_CTRL_REG, RegData);
            phyHandle->ops->phyRead(phyHandle, PHY_BASICCONTROL_REG, &RegData);
            PRINTF("Reg 0x%x:\t0x%x\r\n", PHY_BASICCONTROL_REG, RegData);
            phyHandle->ops->phyRead(phyHandle, PHY_PHYCR_REG, &RegData);
            PRINTF("Reg 0x%x:\t0x%x\r\n", PHY_PHYCR_REG, RegData);
            phyHandle->ops->phyRead(phyHandle, PHY_1000BASET_CONTROL_REG, &RegData);
            PRINTF("Reg 0x%x:\t0x%x\r\n", PHY_1000BASET_CONTROL_REG, RegData);
            phyHandle->ops->phyRead(phyHandle, PHY_TMCH_CTRL_REG, &RegData);
            PRINTF("Reg 0x%x:\t0x%x\r\n", PHY_TMCH_CTRL_REG, RegData);

		}
		else if(cmd==CMD_TEST_MODE_3T)
		{
			PRINTF("\r\n31: 1000 Base Test Mode 3 with TX_TCLK is selected\r\n");
            phyHandle->ops->phyWrite(phyHandle, PHY_CTRL_REG, 0x8000);	// Reg 0x001F = 0x8000
            PRINTF("Reg 0x%x <= Data 0x%x\r\n", PHY_CTRL_REG, 0x8000);
            phyHandle->ops->phyWrite(phyHandle, PHY_BASICCONTROL_REG, 0x0140);	// Reg 0x0000 = 0x0140
            PRINTF("Reg 0x%x <= Data 0x%x\r\n", PHY_BASICCONTROL_REG, 0x0140);
            phyHandle->ops->phyWrite(phyHandle, PHY_PHYCR_REG, 0x5008);	// Reg 0x0010 = 0x5008
            PRINTF("Reg 0x%x <= Data 0x%x\r\n", PHY_PHYCR_REG, 0x5008);
            phyHandle->ops->phyWrite(phyHandle, PHY_1000BASET_CONTROL_REG, 0x7B00);	// Reg 0x0009 = 0x7B00
            PRINTF("Reg 0x%x <= Data 0x%x\r\n", PHY_1000BASET_CONTROL_REG, 0x7B00);
            phyHandle->ops->phyWrite(phyHandle, PHY_TMCH_CTRL_REG, 0x0480);	// Reg 0x0025 = 0x0480
            PRINTF("Reg 0x%x <= Data 0x%x\r\n", PHY_TMCH_CTRL_REG, 0x0480);
            phyHandle->ops->phyWrite(phyHandle, PHY_IO_MUX_CFG_REG, 0x81F);	// Reg 0x0170 = 0x81F
            PRINTF("Reg 0x%x <= Data 0x%x\r\n", PHY_IO_MUX_CFG_REG, 0x81F);
            phyHandle->ops->phyWrite(phyHandle, PHY_PLLCTL_REG, 0x0010);	// Reg 0x00C6 = 0x0010
            PRINTF("Reg 0x%x <= Data 0x%x\r\n", PHY_PLLCTL_REG, 0x0010);

            phyHandle->ops->phyRead(phyHandle, PHY_CTRL_REG, &RegData);
            PRINTF("Reg 0x%x:\t0x%x\r\n", PHY_CTRL_REG, RegData);
            phyHandle->ops->phyRead(phyHandle, PHY_BASICCONTROL_REG, &RegData);
            PRINTF("Reg 0x%x:\t0x%x\r\n", PHY_BASICCONTROL_REG, RegData);
            phyHandle->ops->phyRead(phyHandle, PHY_PHYCR_REG, &RegData);
            PRINTF("Reg 0x%x:\t0x%x\r\n", PHY_PHYCR_REG, RegData);
            phyHandle->ops->phyRead(phyHandle, PHY_1000BASET_CONTROL_REG, &RegData);
            PRINTF("Reg 0x%x:\t0x%x\r\n", PHY_1000BASET_CONTROL_REG, RegData);
            phyHandle->ops->phyRead(phyHandle, PHY_TMCH_CTRL_REG, &RegData);
            PRINTF("Reg 0x%x:\t0x%x\r\n", PHY_TMCH_CTRL_REG, RegData);
            phyHandle->ops->phyRead(phyHandle, PHY_IO_MUX_CFG_REG, &RegData);
            PRINTF("Reg 0x%x:\t0x%x\r\n", PHY_IO_MUX_CFG_REG, RegData);
            phyHandle->ops->phyRead(phyHandle, PHY_PLLCTL_REG, &RegData);
            PRINTF("Reg 0x%x:\t0x%x\r\n", PHY_PLLCTL_REG, RegData);

		}
		else if(cmd==CMD_TEST_MODE_4A)
		{
			PRINTF("\r\n41: 1000 Base Test Mode 4 (Channel A) is selected\r\n");
            phyHandle->ops->phyWrite(phyHandle, PHY_CTRL_REG, 0x8000);	// Reg 0x001F = 0x8000
            PRINTF("Reg 0x%x <= Data 0x%x\r\n", PHY_CTRL_REG, 0x8000);
            phyHandle->ops->phyWrite(phyHandle, PHY_BASICCONTROL_REG, 0x0140);	// Reg 0x0000 = 0x0140
            PRINTF("Reg 0x%x <= Data 0x%x\r\n", PHY_BASICCONTROL_REG, 0x0140);
            phyHandle->ops->phyWrite(phyHandle, PHY_PHYCR_REG, 0x5008);	// Reg 0x0010 = 0x5008
            PRINTF("Reg 0x%x <= Data 0x%x\r\n", PHY_PHYCR_REG, 0x5008);
            phyHandle->ops->phyWrite(phyHandle, PHY_1000BASET_CONTROL_REG, 0x9B00);	// Reg 0x0009 = 0x9B00
            PRINTF("Reg 0x%x <= Data 0x%x\r\n", PHY_1000BASET_CONTROL_REG, 0x9B00);
            phyHandle->ops->phyWrite(phyHandle, PHY_TMCH_CTRL_REG, 0x0400);	// Reg 0x0025 = 0x0400
            PRINTF("Reg 0x%x <= Data 0x%x\r\n", PHY_TMCH_CTRL_REG, 0x0400);

            phyHandle->ops->phyRead(phyHandle, PHY_CTRL_REG, &RegData);
            PRINTF("Reg 0x%x:\t0x%x\r\n", PHY_CTRL_REG, RegData);
            phyHandle->ops->phyRead(phyHandle, PHY_BASICCONTROL_REG, &RegData);
            PRINTF("Reg 0x%x:\t0x%x\r\n", PHY_BASICCONTROL_REG, RegData);
            phyHandle->ops->phyRead(phyHandle, PHY_PHYCR_REG, &RegData);
            PRINTF("Reg 0x%x:\t0x%x\r\n", PHY_PHYCR_REG, RegData);
            phyHandle->ops->phyRead(phyHandle, PHY_1000BASET_CONTROL_REG, &RegData);
            PRINTF("Reg 0x%x:\t0x%x\r\n", PHY_1000BASET_CONTROL_REG, RegData);
            phyHandle->ops->phyRead(phyHandle, PHY_TMCH_CTRL_REG, &RegData);
            PRINTF("Reg 0x%x:\t0x%x\r\n", PHY_TMCH_CTRL_REG, RegData);

		}
		else if(cmd==CMD_TEST_MODE_4B)
		{
			PRINTF("\r\n42: 1000 Base Test Mode 4 (Channel B) is selected\r\n");
            phyHandle->ops->phyWrite(phyHandle, PHY_CTRL_REG, 0x8000);	// Reg 0x001F = 0x8000
            PRINTF("Reg 0x%x <= Data 0x%x\r\n", PHY_CTRL_REG, 0x8000);
            phyHandle->ops->phyWrite(phyHandle, PHY_BASICCONTROL_REG, 0x0140);	// Reg 0x0000 = 0x0140
            PRINTF("Reg 0x%x <= Data 0x%x\r\n", PHY_BASICCONTROL_REG, 0x0140);
            phyHandle->ops->phyWrite(phyHandle, PHY_PHYCR_REG, 0x5008);	// Reg 0x0010 = 0x5008
            PRINTF("Reg 0x%x <= Data 0x%x\r\n", PHY_PHYCR_REG, 0x5008);
            phyHandle->ops->phyWrite(phyHandle, PHY_1000BASET_CONTROL_REG, 0x9B00);	// Reg 0x0009 = 0x9B00
            PRINTF("Reg 0x%x <= Data 0x%x\r\n", PHY_1000BASET_CONTROL_REG, 0x9B00);
            phyHandle->ops->phyWrite(phyHandle, PHY_TMCH_CTRL_REG, 0x0420);	// Reg 0x0025 = 0x0420
            PRINTF("Reg 0x%x <= Data 0x%x\r\n", PHY_TMCH_CTRL_REG, 0x0420);

            phyHandle->ops->phyRead(phyHandle, PHY_CTRL_REG, &RegData);
            PRINTF("Reg 0x%x:\t0x%x\r\n", PHY_CTRL_REG, RegData);
            phyHandle->ops->phyRead(phyHandle, PHY_BASICCONTROL_REG, &RegData);
            PRINTF("Reg 0x%x:\t0x%x\r\n", PHY_BASICCONTROL_REG, RegData);
            phyHandle->ops->phyRead(phyHandle, PHY_PHYCR_REG, &RegData);
            PRINTF("Reg 0x%x:\t0x%x\r\n", PHY_PHYCR_REG, RegData);
            phyHandle->ops->phyRead(phyHandle, PHY_1000BASET_CONTROL_REG, &RegData);
            PRINTF("Reg 0x%x:\t0x%x\r\n", PHY_1000BASET_CONTROL_REG, RegData);
            phyHandle->ops->phyRead(phyHandle, PHY_TMCH_CTRL_REG, &RegData);
            PRINTF("Reg 0x%x:\t0x%x\r\n", PHY_TMCH_CTRL_REG, RegData);

		}
		else if(cmd==CMD_TEST_MODE_4C)
		{
			PRINTF("\r\n43: 1000 Base Test Mode 4 (Channel C) is selected\r\n");
            phyHandle->ops->phyWrite(phyHandle, PHY_CTRL_REG, 0x8000);	// Reg 0x001F = 0x8000
            PRINTF("Reg 0x%x <= Data 0x%x\r\n", PHY_CTRL_REG, 0x8000);
            phyHandle->ops->phyWrite(phyHandle, PHY_BASICCONTROL_REG, 0x0140);	// Reg 0x0000 = 0x0140
            PRINTF("Reg 0x%x <= Data 0x%x\r\n", PHY_BASICCONTROL_REG, 0x0140);
            phyHandle->ops->phyWrite(phyHandle, PHY_PHYCR_REG, 0x5008);	// Reg 0x0010 = 0x5008
            PRINTF("Reg 0x%x <= Data 0x%x\r\n", PHY_PHYCR_REG, 0x5008);
            phyHandle->ops->phyWrite(phyHandle, PHY_1000BASET_CONTROL_REG, 0x9B00);	// Reg 0x0009 = 0x9B00
            PRINTF("Reg 0x%x <= Data 0x%x\r\n", PHY_1000BASET_CONTROL_REG, 0x9B00);
            phyHandle->ops->phyWrite(phyHandle, PHY_TMCH_CTRL_REG, 0x0440);	// Reg 0x0025 = 0x0440
            PRINTF("Reg 0x%x <= Data 0x%x\r\n", PHY_TMCH_CTRL_REG, 0x0440);

            phyHandle->ops->phyRead(phyHandle, PHY_CTRL_REG, &RegData);
            PRINTF("Reg 0x%x:\t0x%x\r\n", PHY_CTRL_REG, RegData);
            phyHandle->ops->phyRead(phyHandle, PHY_BASICCONTROL_REG, &RegData);
            PRINTF("Reg 0x%x:\t0x%x\r\n", PHY_BASICCONTROL_REG, RegData);
            phyHandle->ops->phyRead(phyHandle, PHY_PHYCR_REG, &RegData);
            PRINTF("Reg 0x%x:\t0x%x\r\n", PHY_PHYCR_REG, RegData);
            phyHandle->ops->phyRead(phyHandle, PHY_1000BASET_CONTROL_REG, &RegData);
            PRINTF("Reg 0x%x:\t0x%x\r\n", PHY_1000BASET_CONTROL_REG, RegData);
            phyHandle->ops->phyRead(phyHandle, PHY_TMCH_CTRL_REG, &RegData);
            PRINTF("Reg 0x%x:\t0x%x\r\n", PHY_TMCH_CTRL_REG, RegData);

		}
		else if(cmd==CMD_TEST_MODE_4D)
		{
			PRINTF("\r\n44: 1000 Base Test Mode 4 (Channel D) is selected\r\n");
            phyHandle->ops->phyWrite(phyHandle, PHY_CTRL_REG, 0x8000);	// Reg 0x001F = 0x8000
            PRINTF("Reg 0x%x <= Data 0x%x\r\n", PHY_CTRL_REG, 0x8000);
            phyHandle->ops->phyWrite(phyHandle, PHY_BASICCONTROL_REG, 0x0140);	// Reg 0x0000 = 0x0140
            PRINTF("Reg 0x%x <= Data 0x%x\r\n", PHY_BASICCONTROL_REG, 0x0140);
            phyHandle->ops->phyWrite(phyHandle, PHY_PHYCR_REG, 0x5008);	// Reg 0x0010 = 0x5008
            PRINTF("Reg 0x%x <= Data 0x%x\r\n", PHY_PHYCR_REG, 0x5008);
            phyHandle->ops->phyWrite(phyHandle, PHY_1000BASET_CONTROL_REG, 0x9B00);	// Reg 0x0009 = 0x9B00
            PRINTF("Reg 0x%x <= Data 0x%x\r\n", PHY_1000BASET_CONTROL_REG, 0x9B00);
            phyHandle->ops->phyWrite(phyHandle, PHY_TMCH_CTRL_REG, 0x0460);	// Reg 0x0025 = 0x0460
            PRINTF("Reg 0x%x <= Data 0x%x\r\n", PHY_TMCH_CTRL_REG, 0x0460);

            phyHandle->ops->phyRead(phyHandle, PHY_CTRL_REG, &RegData);
            PRINTF("Reg 0x%x:\t0x%x\r\n", PHY_CTRL_REG, RegData);
            phyHandle->ops->phyRead(phyHandle, PHY_BASICCONTROL_REG, &RegData);
            PRINTF("Reg 0x%x:\t0x%x\r\n", PHY_BASICCONTROL_REG, RegData);
            phyHandle->ops->phyRead(phyHandle, PHY_PHYCR_REG, &RegData);
            PRINTF("Reg 0x%x:\t0x%x\r\n", PHY_PHYCR_REG, RegData);
            phyHandle->ops->phyRead(phyHandle, PHY_1000BASET_CONTROL_REG, &RegData);
            PRINTF("Reg 0x%x:\t0x%x\r\n", PHY_1000BASET_CONTROL_REG, RegData);
            phyHandle->ops->phyRead(phyHandle, PHY_TMCH_CTRL_REG, &RegData);
            PRINTF("Reg 0x%x:\t0x%x\r\n", PHY_TMCH_CTRL_REG, RegData);

		}
		else if(cmd==CMD_TEST_MODE_5_MDI)
		{
			PRINTF("\r\n51: 100 Base Standard MDI (Test Mode 5) is selected\r\n");
            phyHandle->ops->phyWrite(phyHandle, PHY_CTRL_REG, 0x8000);	// Reg 0x001F = 0x8000
            PRINTF("Reg 0x%x <= Data 0x%x\r\n", PHY_CTRL_REG, 0x8000);
            phyHandle->ops->phyWrite(phyHandle, PHY_BASICCONTROL_REG, 0x2100);	// Reg 0x0000 = 0x2100
            PRINTF("Reg 0x%x <= Data 0x%x\r\n", PHY_BASICCONTROL_REG, 0x2100);
            phyHandle->ops->phyWrite(phyHandle, PHY_PHYCR_REG, 0x5008);	// Reg 0x0010 = 0x5008
            PRINTF("Reg 0x%x <= Data 0x%x\r\n", PHY_PHYCR_REG, 0x5008);
            phyHandle->ops->phyWrite(phyHandle, PHY_1000BASET_CONTROL_REG, 0xBB00);	// Reg 0x0009 = 0xBB00
            PRINTF("Reg 0x%x <= Data 0x%x\r\n", PHY_1000BASET_CONTROL_REG, 0xBB00);
            phyHandle->ops->phyWrite(phyHandle, PHY_TMCH_CTRL_REG, 0x0480);	// Reg 0x0025 = 0x0480
            PRINTF("Reg 0x%x <= Data 0x%x\r\n", PHY_TMCH_CTRL_REG, 0x0480);

            phyHandle->ops->phyRead(phyHandle, PHY_CTRL_REG, &RegData);
            PRINTF("Reg 0x%x:\t0x%x\r\n", PHY_CTRL_REG, RegData);
            phyHandle->ops->phyRead(phyHandle, PHY_BASICCONTROL_REG, &RegData);
            PRINTF("Reg 0x%x:\t0x%x\r\n", PHY_BASICCONTROL_REG, RegData);
            phyHandle->ops->phyRead(phyHandle, PHY_PHYCR_REG, &RegData);
            PRINTF("Reg 0x%x:\t0x%x\r\n", PHY_PHYCR_REG, RegData);
            phyHandle->ops->phyRead(phyHandle, PHY_1000BASET_CONTROL_REG, &RegData);
            PRINTF("Reg 0x%x:\t0x%x\r\n", PHY_1000BASET_CONTROL_REG, RegData);
            phyHandle->ops->phyRead(phyHandle, PHY_TMCH_CTRL_REG, &RegData);
            PRINTF("Reg 0x%x:\t0x%x\r\n", PHY_TMCH_CTRL_REG, RegData);

		}
		else if(cmd==CMD_TEST_MODE_5_MDIX)
		{
			PRINTF("\r\n52: 100 Base Standard MDIX (Test Mode 5) is selected\r\n");
            phyHandle->ops->phyWrite(phyHandle, PHY_CTRL_REG, 0x8000);	// Reg 0x001F = 0x8000
            PRINTF("Reg 0x%x <= Data 0x%x\r\n", PHY_CTRL_REG, 0x8000);
            phyHandle->ops->phyWrite(phyHandle, PHY_BASICCONTROL_REG, 0x2100);	// Reg 0x0000 = 0x2100
            PRINTF("Reg 0x%x <= Data 0x%x\r\n", PHY_BASICCONTROL_REG, 0x2100);
            phyHandle->ops->phyWrite(phyHandle, PHY_PHYCR_REG, 0x5028);	// Reg 0x0010 = 0x5028
            PRINTF("Reg 0x%x <= Data 0x%x\r\n", PHY_PHYCR_REG, 0x5028);
            phyHandle->ops->phyWrite(phyHandle, PHY_1000BASET_CONTROL_REG, 0xBB00);	// Reg 0x0009 = 0xBB00
            PRINTF("Reg 0x%x <= Data 0x%x\r\n", PHY_1000BASET_CONTROL_REG, 0xBB00);
            phyHandle->ops->phyWrite(phyHandle, PHY_TMCH_CTRL_REG, 0x0480);	// Reg 0x0025 = 0x0480
            PRINTF("Reg 0x%x <= Data 0x%x\r\n", PHY_TMCH_CTRL_REG, 0x0480);

            phyHandle->ops->phyRead(phyHandle, PHY_CTRL_REG, &RegData);
            PRINTF("Reg 0x%x:\t0x%x\r\n", PHY_CTRL_REG, RegData);
            phyHandle->ops->phyRead(phyHandle, PHY_BASICCONTROL_REG, &RegData);
            PRINTF("Reg 0x%x:\t0x%x\r\n", PHY_BASICCONTROL_REG, RegData);
            phyHandle->ops->phyRead(phyHandle, PHY_PHYCR_REG, &RegData);
            PRINTF("Reg 0x%x:\t0x%x\r\n", PHY_PHYCR_REG, RegData);
            phyHandle->ops->phyRead(phyHandle, PHY_1000BASET_CONTROL_REG, &RegData);
            PRINTF("Reg 0x%x:\t0x%x\r\n", PHY_1000BASET_CONTROL_REG, RegData);
            phyHandle->ops->phyRead(phyHandle, PHY_TMCH_CTRL_REG, &RegData);
            PRINTF("Reg 0x%x:\t0x%x\r\n", PHY_TMCH_CTRL_REG, RegData);

		}
		else if(cmd==CMD_LINK_PULSE)
		{
			PRINTF("\r\n6: 10 Base Link Pulse is selected\r\n");
            phyHandle->ops->phyWrite(phyHandle, PHY_CTRL_REG, 0x8000);	// Reg 0x001F = 0x8000
            PRINTF("Reg 0x%x <= Data 0x%x\r\n", PHY_CTRL_REG, 0x8000);
            phyHandle->ops->phyWrite(phyHandle, PHY_BASICCONTROL_REG, 0x0100);	// Reg 0x0000 = 0x0100
            PRINTF("Reg 0x%x <= Data 0x%x\r\n", PHY_BASICCONTROL_REG, 0x0100);
            phyHandle->ops->phyWrite(phyHandle, PHY_PHYCR_REG, 0x5008);	// Reg 0x0010 = 0x5008
            PRINTF("Reg 0x%x <= Data 0x%x\r\n", PHY_PHYCR_REG, 0x5008);

            phyHandle->ops->phyRead(phyHandle, PHY_CTRL_REG, &RegData);
            PRINTF("Reg 0x%x:\t0x%x\r\n", PHY_CTRL_REG, RegData);
            phyHandle->ops->phyRead(phyHandle, PHY_BASICCONTROL_REG, &RegData);
            PRINTF("Reg 0x%x:\t0x%x\r\n", PHY_BASICCONTROL_REG, RegData);
            phyHandle->ops->phyRead(phyHandle, PHY_PHYCR_REG, &RegData);
            PRINTF("Reg 0x%x:\t0x%x\r\n", PHY_PHYCR_REG, RegData);

		}
		else if(cmd==CMD_STANDARD)
		{
			PRINTF("\r\n7: 10 Base Standard is selected\r\n");
            phyHandle->ops->phyWrite(phyHandle, PHY_CTRL_REG, 0x8000);	// Reg 0x001F = 0x8000
            PRINTF("Reg 0x%x <= Data 0x%x\r\n", PHY_CTRL_REG, 0x8000);
            phyHandle->ops->phyWrite(phyHandle, PHY_BASICCONTROL_REG, 0x0100);	// Reg 0x0000 = 0x0100
            PRINTF("Reg 0x%x <= Data 0x%x\r\n", PHY_BASICCONTROL_REG, 0x0100);
            phyHandle->ops->phyWrite(phyHandle, PHY_PHYCR_REG, 0x5008);	// Reg 0x0010 = 0x5008
            PRINTF("Reg 0x%x <= Data 0x%x\r\n", PHY_PHYCR_REG, 0x5008);
            phyHandle->ops->phyWrite(phyHandle, PHY_BIST_REG, 0x0020);	// Reg 0x0016 = 0x0020
            PRINTF("Reg 0x%x <= Data 0x%x\r\n", PHY_BIST_REG, 0x0020);

            phyHandle->ops->phyRead(phyHandle, PHY_CTRL_REG, &RegData);
            PRINTF("Reg 0x%x:\t0x%x\r\n", PHY_CTRL_REG, RegData);
            phyHandle->ops->phyRead(phyHandle, PHY_BASICCONTROL_REG, &RegData);
            PRINTF("Reg 0x%x:\t0x%x\r\n", PHY_BASICCONTROL_REG, RegData);
            phyHandle->ops->phyRead(phyHandle, PHY_PHYCR_REG, &RegData);
            PRINTF("Reg 0x%x:\t0x%x\r\n", PHY_PHYCR_REG, RegData);
            phyHandle->ops->phyRead(phyHandle, PHY_BIST_REG, &RegData);
            PRINTF("Reg 0x%x:\t0x%x\r\n", PHY_BIST_REG, RegData);

		}
		else if(cmd==CMD_CUSTOM_WRITE)
		{
			PRINTF("\r\n8: Custom Write Mode is selected\r\n");
			while(input_done==0)
			{
				PRINTF("Please enter Reg Value in HEX format (no \"0x\")\r\n");
				SCANF("%x", &RegVal);
				PRINTF("Please enter Reg Data in HEX format (no \"0x\")\r\n");
				SCANF("%x", &RegData);
				PRINTF("Reg 0x%x <= Data 0x%x\r\n", RegVal, RegData);
				phyHandle->ops->phyWrite(phyHandle, RegVal, RegData);
				PRINTF("Continue? (y/n)\r\n");
				SCANF("%c", &RegData);
				if(RegData==110)
					input_done = 1;
			}
			input_done = 0;
		}
		else if(cmd==CMD_CUSTOM_READ)
		{
			PRINTF("\r\n8: Custom Read Mode is selected\r\n");
			while(input_done==0)
			{
				PRINTF("Please enter Reg Value in HEX format (no \"0x\")\r\n");
				SCANF("%x", &RegVal);
				phyHandle->ops->phyRead(phyHandle, RegVal, &RegData);
	            PRINTF("Reg 0x%x:\t0x%x\r\n", RegVal, RegData);
				PRINTF("Continue? (y/n)\r\n");
				SCANF("%c", &RegData);
				if(RegData==110)
					input_done = 1;
			}
			input_done = 0;
		}
		else if(cmd==CMD_END)
		{
			PRINTF("\r\n9: Quit is selected\r\n");

		}
		else
		{
			PRINTF("\r\nInvalid Input\r\n");
		}

	}

	return 0;
}

/*!
 * @brief Main function.
 */
int main(void)
{
    struct netif netif;
    ethernetif_config_t enet0_config = {.phyHandle   = &phyHandle,
                                        .phyAddr     = EXAMPLE_PHY0_ADDRESS,
                                        .phyOps      = EXAMPLE_PHY0_OPS,
                                        .phyResource = EXAMPLE_PHY0_RESOURCE,
 #ifdef configMAC_ADDR
                                        .macAddress = configMAC_ADDR
 #endif
    };
    ethernetif_config_t enet1_config = {.phyHandle   = &phyHandle,
                                        .phyAddr     = EXAMPLE_PHY1_ADDRESS,
                                        .phyOps      = EXAMPLE_PHY1_OPS,
                                        .phyResource = EXAMPLE_PHY1_RESOURCE,
 #ifdef configMAC_ADDR
                                        .macAddress = configMAC_ADDR
 #endif
    };

    gpio_pin_config_t gpio_config = {kGPIO_DigitalOutput, 0, kGPIO_NoIntmode};

    u8_t cmd = 0;

    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    while(1)
	{
		PRINTF("\r\nCompliance Testing v0.2\r\n");
		PRINTF("\r\nPlease enter 1) DP83867IR, 2) KSZ8081RNB:\r\n");
		SCANF("%d", &cmd);
		if(cmd==1)
		{
			ENET_100M = 0;		// enet1
			PRINTF("\r\n1) DP83867IR is selected\r\n");
		}
		else if(cmd==2)
		{
			ENET_100M = 1;		// enet0
			PRINTF("\r\n2) KSZ8081RNB is selected\r\n");
		}
		else
		{
			PRINTF("\r\nInvalid Input\r\n");
			continue;
		}
		break;
	}



    BOARD_InitModuleClock();

    IOMUXC_SelectENETClock();

    if(ENET_100M==1)
    {
    	BOARD_InitEnetPins();
    	GPIO_PinInit(GPIO12, 12, &gpio_config);
    	GPIO_WritePinOutput(GPIO12, 12, 0);
    	SDK_DelayAtLeastUs(10000, CLOCK_GetFreq(kCLOCK_CpuClk));
    	GPIO_WritePinOutput(GPIO12, 12, 1);
    	SDK_DelayAtLeastUs(6, CLOCK_GetFreq(kCLOCK_CpuClk));
    }
    else
    {
    	BOARD_InitEnet1GPins();
    	GPIO_PinInit(GPIO11, 14, &gpio_config);
    	/* For a complete PHY reset of RTL8211FDI-CG, this pin must be asserted low for at least 10ms. And
    	 * wait for a further 30ms(for internal circuits settling time) before accessing the PHY register */
    	GPIO_WritePinOutput(GPIO11, 14, 0);
    	SDK_DelayAtLeastUs(10000, CLOCK_GetFreq(kCLOCK_CpuClk));
    	GPIO_WritePinOutput(GPIO11, 14, 1);
    	SDK_DelayAtLeastUs(30000, CLOCK_GetFreq(kCLOCK_CpuClk));

    	EnableIRQ(ENET_1G_MAC0_Tx_Rx_1_IRQn);
    	EnableIRQ(ENET_1G_MAC0_Tx_Rx_2_IRQn);
    }

    MDIO_Init();
    g_phy0_resource.read  = MDIO_Read;
    g_phy0_resource.write = MDIO_Write;
    g_phy1_resource.read  = MDIO_Read;
    g_phy1_resource.write = MDIO_Write;

    time_init();

    /* Set MAC address. */
#ifndef configMAC_ADDR
    (void)SILICONID_ConvertToMacAddr(&enet0_config.macAddress);
    (void)SILICONID_ConvertToMacAddr(&enet1_config.macAddress);
#endif

    /* Get clock after hardware init. */
    enet0_config.srcClockHz = EXAMPLE_CLOCK_FREQ;
    enet1_config.srcClockHz = EXAMPLE_CLOCK_FREQ;

    lwip_init();

    if(ENET_100M==1)
    {
    	netif_add(&netif, NULL, NULL, NULL, &enet0_config, EXAMPLE_NETIF0_INIT_FN, ethernet_input);
    	netif_set_default(&netif);
    	netif_set_up(&netif);
    }
    else
    {
    	netif_add(&netif, NULL, NULL, NULL, &enet1_config, EXAMPLE_NETIF1_INIT_FN, ethernet_input);
    	netif_set_default(&netif);
    	netif_set_up(&netif);
    }

    //while (ethernetif_wait_linkup(&netif, 5000) != ERR_OK)
    //{
        //PRINTF("PHY Auto-negotiation failed. Please check the cable connection and link partner setting.\r\n");
    //}

#if 0
    dhcp_start(&netif);

    PRINTF("\r\n************************************************\r\n");
    PRINTF(" DHCP example\r\n");
    PRINTF("************************************************\r\n");

    while (1)
    {
        /* Poll the driver, get any outstanding frames */
        ethernetif_input(&netif);

        /* Handle all system timeouts for all core protocols */
        sys_check_timeouts();

        /* Print DHCP progress */
        print_dhcp_state(&netif);
    }
#else
    if(ENET_100M==1)
    	ksz8081rnb_compliance_testing(&phyHandle);
    else
    	dp83867ir_compliance_testing(&phyHandle);

#endif
}
#endif
