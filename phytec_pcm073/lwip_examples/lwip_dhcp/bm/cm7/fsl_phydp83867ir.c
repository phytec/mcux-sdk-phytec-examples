/*
 * Copyright 2020-2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_phydp83867ir.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*! @brief Defines the PHY DP83867IR vendor defined registers. */
#define PHY_PHYCTRL_REG	0x10U
#define PHY_STATUS_REG	0x11U /*!< The PHY specific status register. */

/*! @brief Defines the PHY DP83867IR ID number. */
#define PHY_CONTROL_ID1 0x2000U /*!< The PHY ID1 . */
#define PHY_CONTROL_ID2 0xa231U /*!< The PHY ID2 . */

/*! @brief Defines the mask flag in specific status register. */
#define PHY_SSTATUS_LINKSTATUS_MASK 0x0400U /*!< The PHY link status mask. */
#define PHY_SSTATUS_LINKSPEED_MASK  0xC000U /*!< The PHY link speed mask. */
#define PHY_SSTATUS_LINKDUPLEX_MASK 0x2000U /*!< The PHY link duplex mask. */
#define PHY_SSTATUS_LINKSPEED_SHIFT 14U    /*!< The link speed shift */

/*! @brief MDIO MMD Devices .*/
#define PHY_MDIO_MMD_PCS 3U
#define PHY_MDIO_MMD_AN  7U

/*! @brief MDIO MMD Physical Coding layer device registers .*/
#define PHY_MDIO_PCS_EEE_CAP 0x14U /* EEE capability */

/*! @brief MDIO MMD AutoNegotiation device registers .*/
#define PHY_MDIO_AN_EEE_ADV 0x3CU /* EEE advertisement */

/*! @brief MDIO MMD EEE mask flags. (common for adv and cap) */
#define PHY_MDIO_EEE_100TX 0x2U
#define PHY_MDIO_EEE_1000T 0x4U

/*! @brief Defines the timeout macro. */
#define PHY_READID_TIMEOUT_COUNT 1000U

/* PHY CTRL bits */
#define DP83867_PHYCR_TX_FIFO_DEPTH_SHIFT	14
#define DP83867_PHYCR_RX_FIFO_DEPTH_SHIFT	12
#define DP83867_PHYCR_FIFO_DEPTH_MAX		0x03
#define DP83867_PHYCR_TX_FIFO_DEPTH_MASK	0xc0
#define DP83867_PHYCR_RX_FIFO_DEPTH_MASK	0x30


/* RGMIIDCTL bits */
#define DP83867_RGMII_TX_CLK_DELAY_MAX		0xf
#define DP83867_RGMII_TX_CLK_DELAY_SHIFT	4
#define DP83867_RGMII_TX_CLK_DELAY_INV	(DP83867_RGMII_TX_CLK_DELAY_MAX + 1)
#define DP83867_RGMII_RX_CLK_DELAY_MAX		0xf
#define DP83867_RGMII_RX_CLK_DELAY_SHIFT	0
#define DP83867_RGMII_RX_CLK_DELAY_INV	(DP83867_RGMII_RX_CLK_DELAY_MAX + 1)

/* RGMIICTL bits */
#define DP83867_RGMII_TX_CLK_DELAY_EN		0x02
#define DP83867_RGMII_RX_CLK_DELAY_EN		0x01

#define DP83867_DEVADDR		0x1f
#define DP83867_RGMIICTL	0x0032
#define DP83867_STRAP_STS1	0x006E
#define DP83867_STRAP_STS2	0x006f
#define DP83867_RGMIIDCTL	0x0086
#define DP83867_DSP_FFE_CFG	0x012C
#define DP83867_RXFCFG		0x0134
#define DP83867_RXFPMD1	0x0136
#define DP83867_RXFPMD2	0x0137
#define DP83867_RXFPMD3	0x0138
#define DP83867_RXFSOP1	0x0139
#define DP83867_RXFSOP2	0x013A
#define DP83867_RXFSOP3	0x013B
#define DP83867_IO_MUX_CFG	0x0170
#define DP83867_SGMIICTL	0x00D3
#define DP83867_10M_SGMII_CFG   0x016F

/*! @brief Defines the PHY resource interface. */
#define PHY_DP83867IR_WRITE(handle, regAddr, data) \
    ((phy_dp83867ir_resource_t *)(handle)->resource)->write((handle)->phyAddr, regAddr, data)
#define PHY_DP83867IR_READ(handle, regAddr, pData) \
    ((phy_dp83867ir_resource_t *)(handle)->resource)->read((handle)->phyAddr, regAddr, pData)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static status_t PHY_DP83867IR_MMD_SetDevice(phy_handle_t *handle,
                                           uint8_t device,
                                           uint16_t addr,
                                           phy_mmd_access_mode_t mode);
static inline status_t PHY_DP83867IR_MMD_ReadData(phy_handle_t *handle, uint16_t *pData);
static inline status_t PHY_DP83867IR_MMD_WriteData(phy_handle_t *handle, uint16_t data);
static status_t PHY_DP83867IR_MMD_Read(phy_handle_t *handle, uint8_t device, uint16_t addr, uint16_t *pData);
static status_t PHY_DP83867IR_MMD_Write(phy_handle_t *handle, uint8_t device, uint16_t addr, uint16_t data);

/*******************************************************************************
 * Variables
 ******************************************************************************/

const phy_operations_t phydp83867ir_ops = {.phyInit             = PHY_DP83867IR_Init,
                                          .phyWrite            = PHY_DP83867IR_Write,
                                          .phyRead             = PHY_DP83867IR_Read,
                                          .getAutoNegoStatus   = PHY_DP83867IR_GetAutoNegotiationStatus,
                                          .getLinkStatus       = PHY_DP83867IR_GetLinkStatus,
                                          .getLinkSpeedDuplex  = PHY_DP83867IR_GetLinkSpeedDuplex,
                                          .setLinkSpeedDuplex  = PHY_DP83867IR_SetLinkSpeedDuplex,
                                          .enableLoopback      = PHY_DP83867IR_EnableLoopback};
//                                          .enableLinkInterrupt = PHY_DP83867IR_EnableLinkInterrupt,
//                                          .clearInterrupt      = PHY_DP83867IR_ClearInterrupt};

/*******************************************************************************
 * Code
 ******************************************************************************/

status_t PHY_DP83867IR_Init(phy_handle_t *handle, const phy_config_t *config)
{
    uint32_t counter = PHY_READID_TIMEOUT_COUNT;
    status_t result;
    uint16_t regValue = 0U;
    uint16_t regValue1 = 0U;
    uint16_t regValue2 = 0U;

    /* Assign PHY address and operation resource. */
    handle->phyAddr  = config->phyAddr;
    handle->resource = config->resource;

    /* Check PHY ID. */
    do
    {
        result = PHY_DP83867IR_READ(handle, PHY_ID1_REG, &regValue1);
        if (result != kStatus_Success)
        {
            return result;
        }
        result = PHY_DP83867IR_READ(handle, PHY_ID2_REG, &regValue2);
        if (result != kStatus_Success)
        {
            return result;
        }
        counter--;
    } while ((regValue1 != PHY_CONTROL_ID1) && (regValue2 != PHY_CONTROL_ID2) && (counter != 0U));

    if (counter == 0U)
    {
        return kStatus_Fail;
    }

    /* Reset PHY. */
    result = PHY_DP83867IR_WRITE(handle, PHY_BASICCONTROL_REG, PHY_BCTL_RESET_MASK);
    if (result != kStatus_Success)
    {
        return result;
    }

    /* TX_FIFO DEPTH */
    result = PHY_DP83867IR_READ(handle, PHY_PHYCTRL_REG, &regValue);
    if (result != kStatus_Success)
    {
        return result;
    }

    regValue &= ~DP83867_PHYCR_TX_FIFO_DEPTH_MASK;
	regValue |= (DP83867_PHYCR_FIFO_DEPTH_MAX << DP83867_PHYCR_TX_FIFO_DEPTH_SHIFT);

    result = PHY_DP83867IR_WRITE(handle, PHY_PHYCTRL_REG, regValue);
    if (result != kStatus_Success)
    {
        return result;
    }

	/* internal delay */
	result = PHY_DP83867IR_MMD_Read(handle, DP83867_DEVADDR, DP83867_RGMIICTL, &regValue);
    if (result != kStatus_Success)
    {
        return result;
    }

	regValue &= ~(DP83867_RGMII_TX_CLK_DELAY_EN | DP83867_RGMII_RX_CLK_DELAY_EN);
	regValue |= (DP83867_RGMII_TX_CLK_DELAY_EN | DP83867_RGMII_RX_CLK_DELAY_EN);

	regValue |= DP83867_RGMII_TX_CLK_DELAY_EN;

	regValue |= DP83867_RGMII_RX_CLK_DELAY_EN;

	result = PHY_DP83867IR_MMD_Write(handle, DP83867_DEVADDR, DP83867_RGMIICTL, regValue);
    if (result != kStatus_Success)
    {
        return result;
    }

	regValue = 0;
	regValue |= DP83867_RGMII_RX_CLK_DELAY_MAX;
	regValue |= DP83867_RGMII_TX_CLK_DELAY_MAX <<
			 DP83867_RGMII_TX_CLK_DELAY_SHIFT;

	result = PHY_DP83867IR_MMD_Write(handle, DP83867_DEVADDR, DP83867_RGMIICTL, regValue);
    if (result != kStatus_Success)
    {
        return result;
    }
	result = PHY_DP83867IR_MMD_Write(handle, DP83867_DEVADDR, DP83867_RGMIICTL, regValue);
    if (result != kStatus_Success)
    {
        return result;
    }

    if (config->autoNeg)
    {
        /* Set the auto-negotiation. */
        result =
            PHY_DP83867IR_WRITE(handle, PHY_AUTONEG_ADVERTISE_REG,
                       PHY_100BASETX_FULLDUPLEX_MASK | PHY_100BASETX_HALFDUPLEX_MASK | PHY_10BASETX_FULLDUPLEX_MASK |
                           PHY_10BASETX_HALFDUPLEX_MASK | PHY_IEEE802_3_SELECTOR_MASK);
        if (result == kStatus_Success)
        {
            result = PHY_DP83867IR_WRITE(handle, PHY_1000BASET_CONTROL_REG,
                                PHY_1000BASET_FULLDUPLEX_MASK);
            if (result == kStatus_Success)
            {
                result = PHY_DP83867IR_READ(handle, PHY_BASICCONTROL_REG, &regValue);
                if (result == kStatus_Success)
                {
                    result = PHY_DP83867IR_WRITE(handle, PHY_BASICCONTROL_REG,
                                        (regValue | PHY_BCTL_AUTONEG_MASK | PHY_BCTL_RESTART_AUTONEG_MASK));
                }
            }
        }
    }
    else
    {
        /* Disable isolate mode */
        result = PHY_DP83867IR_READ(handle, PHY_BASICCONTROL_REG, &regValue);
        if (result != kStatus_Success)
        {
            return result;
        }
        regValue &= PHY_BCTL_ISOLATE_MASK;
        result = PHY_DP83867IR_WRITE(handle, PHY_BASICCONTROL_REG, regValue);
        if (result != kStatus_Success)
        {
            return result;
        }

        /* Disable the auto-negotiation and set user-defined speed/duplex configuration. */
        result = PHY_DP83867IR_SetLinkSpeedDuplex(handle, config->speed, config->duplex);
    }
    return result;
}

status_t PHY_DP83867IR_Write(phy_handle_t *handle, uint8_t phyReg, uint16_t data)
{
    return PHY_DP83867IR_WRITE(handle, phyReg, data);
}

status_t PHY_DP83867IR_Read(phy_handle_t *handle, uint8_t phyReg, uint16_t *pData)
{
    return PHY_DP83867IR_READ(handle, phyReg, pData);
}

status_t PHY_DP83867IR_GetAutoNegotiationStatus(phy_handle_t *handle, bool *status)
{
    assert(status);

    status_t result;
    uint16_t regValue;

    *status = false;

    /* Check auto negotiation complete. */
    result = PHY_DP83867IR_READ(handle, PHY_BASICSTATUS_REG, &regValue);
    if (result == kStatus_Success)
    {
        if ((regValue & PHY_BSTATUS_AUTONEGCOMP_MASK) != 0U)
        {
            *status = true;
        }
    }
    return result;
}

status_t PHY_DP83867IR_GetLinkStatus(phy_handle_t *handle, bool *status)
{
    assert(status);

    status_t result;
    uint16_t regValue;

    /* Read the basic status register. */
    result = PHY_DP83867IR_READ(handle, PHY_STATUS_REG, &regValue);
    if (result == kStatus_Success)
    {
        if ((PHY_SSTATUS_LINKSTATUS_MASK & regValue) != 0U)
        {
            /* Link up. */
            *status = true;
        }
        else
        {
            /* Link down. */
            *status = false;
        }
    }
    return result;
}

status_t PHY_DP83867IR_GetLinkSpeedDuplex(phy_handle_t *handle, phy_speed_t *speed, phy_duplex_t *duplex)
{
    assert(!((speed == NULL) && (duplex == NULL)));

    status_t result;
    uint16_t regValue;

    /* Read the status register. */
    result = PHY_DP83867IR_READ(handle, PHY_STATUS_REG, &regValue);
    if (result == kStatus_Success)
    {
        if (speed != NULL)
        {
            switch ((regValue & PHY_SSTATUS_LINKSPEED_MASK) >> PHY_SSTATUS_LINKSPEED_SHIFT)
            {
                case (uint16_t)kPHY_Speed10M:
                    *speed = kPHY_Speed10M;
                    break;
                case (uint16_t)kPHY_Speed100M:
                    *speed = kPHY_Speed100M;
                    break;
                case (uint16_t)kPHY_Speed1000M:
                    *speed = kPHY_Speed1000M;
                    break;
                default:
                    *speed = kPHY_Speed10M;
                    break;
            }
        }

        if (duplex != NULL)
        {
            if ((regValue & PHY_SSTATUS_LINKDUPLEX_MASK) != 0U)
            {
                *duplex = kPHY_FullDuplex;
            }
            else
            {
                *duplex = kPHY_HalfDuplex;
            }
        }
    }
    return result;
}

status_t PHY_DP83867IR_SetLinkSpeedDuplex(phy_handle_t *handle, phy_speed_t speed, phy_duplex_t duplex)
{
    status_t result;
    uint16_t regValue;

    result = PHY_DP83867IR_READ(handle, PHY_BASICCONTROL_REG, &regValue);
    if (result == kStatus_Success)
    {
        /* Disable the auto-negotiation and set according to user-defined configuration. */
        regValue &= ~PHY_BCTL_AUTONEG_MASK;
        if (speed == kPHY_Speed1000M)
        {
            regValue &= PHY_BCTL_SPEED0_MASK;
            regValue |= PHY_BCTL_SPEED1_MASK;
        }
        else if (speed == kPHY_Speed100M)
        {
            regValue |= PHY_BCTL_SPEED0_MASK;
            regValue &= ~PHY_BCTL_SPEED1_MASK;
        }
        else
        {
            regValue &= ~PHY_BCTL_SPEED0_MASK;
            regValue &= ~PHY_BCTL_SPEED1_MASK;
        }
        if (duplex == kPHY_FullDuplex)
        {
            regValue |= PHY_BCTL_DUPLEX_MASK;
        }
        else
        {
            regValue &= ~PHY_BCTL_DUPLEX_MASK;
        }
        result = PHY_DP83867IR_WRITE(handle, PHY_BASICCONTROL_REG, regValue);
    }
    return result;
}

status_t PHY_DP83867IR_EnableLoopback(phy_handle_t *handle, phy_loop_t mode, phy_speed_t speed, bool enable)
{
    /* This PHY only supports local loopback. */
    assert(mode == kPHY_LocalLoop);

    status_t result;
    uint16_t regValue;

    /* Set the loop mode. */
    if (enable)
    {
        if (speed == kPHY_Speed1000M)
        {
            regValue = PHY_BCTL_SPEED1_MASK | PHY_BCTL_DUPLEX_MASK | PHY_BCTL_LOOP_MASK;
        }
        else if (speed == kPHY_Speed100M)
        {
            regValue = PHY_BCTL_SPEED0_MASK | PHY_BCTL_DUPLEX_MASK | PHY_BCTL_LOOP_MASK;
        }
        else
        {
            regValue = PHY_BCTL_DUPLEX_MASK | PHY_BCTL_LOOP_MASK;
        }
        result = PHY_DP83867IR_WRITE(handle, PHY_BASICCONTROL_REG, regValue);
    }
    else
    {
        /* First read the current status in control register. */
        result = PHY_DP83867IR_READ(handle, PHY_BASICCONTROL_REG, &regValue);
        if (result == kStatus_Success)
        {
            regValue &= ~PHY_BCTL_LOOP_MASK;
            result = PHY_DP83867IR_WRITE(handle, PHY_BASICCONTROL_REG,
                                (regValue | PHY_BCTL_RESTART_AUTONEG_MASK));
        }
    }
    return result;
}
#if 0
status_t PHY_DP83867IR_EnableLinkInterrupt(phy_handle_t *handle, phy_interrupt_type_t type, bool enable)
{
    assert(type == kPHY_IntrActiveLow);

    status_t result;
    uint16_t regValue;

    result = PHY_Write(handle, PHY_PAGE_SELECT_REG, PHY_PAGE_INTR_ADDR);
    if (result != kStatus_Success)
    {
        return result;
    }

    /* Read operation will clear pending interrupt before enable interrupt. */
    result = PHY_DP83867IR_READ(handle, PHY_INER_REG, &regValue);
    if (result != kStatus_Success)
    {
        return result;
    }

    /* Enable/Disable link up+down interrupt. */
    if (enable)
    {
        regValue |= PHY_INER_LINKSTATUS_CHANGE_MASK;
    }
    else
    {
        regValue &= ~PHY_INER_LINKSTATUS_CHANGE_MASK;
    }
    result = PHY_DP83867IR_WRITE(handle, PHY_INER_REG, regValue);
    if (result != kStatus_Success)
    {
        return result;
    }

    /* Restore to default page 0 */
    result = PHY_Write(handle, PHY_PAGE_SELECT_REG, 0);
    if (result != kStatus_Success)
    {
        return result;
    }

    return result;
}

status_t PHY_DP83867IR_ClearInterrupt(phy_handle_t *handle)
{
    uint16_t regValue;

    /* Found both read reg 0x1D from page 0 or page 0xA42 are useful. But datasheet
       describes it's in page 0xA42. Here use simpler implementation. */
    return PHY_DP83867IR_READ(handle, PHY_INSR_REG, &regValue);
}
#endif

static status_t PHY_DP83867IR_MMD_SetDevice(phy_handle_t *handle,
                                           uint8_t device,
                                           uint16_t addr,
                                           phy_mmd_access_mode_t mode)
{
    status_t result = kStatus_Success;

    /* Set Function mode of address access(b00) and device address. */
    result = PHY_DP83867IR_WRITE(handle, PHY_MMD_ACCESS_CONTROL_REG, device);
    if (result != kStatus_Success)
    {
        return result;
    }

    /* Set register address. */
    result = PHY_DP83867IR_WRITE(handle, PHY_MMD_ACCESS_DATA_REG, addr);
    if (result != kStatus_Success)
    {
        return result;
    }

    /* Set Function mode of data access(b01~11) and device address. */
    result = PHY_DP83867IR_WRITE(handle, PHY_MMD_ACCESS_CONTROL_REG, (uint16_t)mode | (uint16_t)device);
    return result;
}

static inline status_t PHY_DP83867IR_MMD_ReadData(phy_handle_t *handle, uint16_t *pData)
{
    return PHY_DP83867IR_READ(handle, PHY_MMD_ACCESS_DATA_REG, pData);
}

static inline status_t PHY_DP83867IR_MMD_WriteData(phy_handle_t *handle, uint16_t data)
{
    return PHY_DP83867IR_WRITE(handle, PHY_MMD_ACCESS_DATA_REG, data);
}

static status_t PHY_DP83867IR_MMD_Read(phy_handle_t *handle, uint8_t device, uint16_t addr, uint16_t *pData)
{
    status_t result = kStatus_Success;
    result          = PHY_DP83867IR_MMD_SetDevice(handle, device, addr, kPHY_MMDAccessNoPostIncrement);
    if (result == kStatus_Success)
    {
        result = PHY_DP83867IR_MMD_ReadData(handle, pData);
    }
    return result;
}

static status_t PHY_DP83867IR_MMD_Write(phy_handle_t *handle, uint8_t device, uint16_t addr, uint16_t data)
{
    status_t result = kStatus_Success;

    result = PHY_DP83867IR_MMD_SetDevice(handle, device, addr, kPHY_MMDAccessNoPostIncrement);
    if (result == kStatus_Success)
    {
        result = PHY_DP83867IR_MMD_WriteData(handle, data);
    }
    return result;
}
