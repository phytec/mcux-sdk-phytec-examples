/*
 * Copyright 2020-2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*****************************************************************************
 * PHY DP83867IR driver change log
 *****************************************************************************/

/*!
@page driver_log Driver Change Log

@section phydp83867 PHYDP83867IR
  The current PHYDP83867IR driver version is 2.0.0.

  - 2.0.0
    - Initial version.
*/

#ifndef _FSL_PHYDP83867IR_H_
#define _FSL_PHYDP83867IR_H_

#include "fsl_phy.h"

#define DP83867_PHY_ID		0x2000a231
#define DP83867_DEVADDR		0x1f

#define PHY_BASICCONTROL_REG        0x00U /*!< The PHY basic control register. */
#define PHY_BASICSTATUS_REG         0x01U /*!< The PHY basic status register. */
#define PHY_ID1_REG                 0x02U /*!< The PHY ID one register. */
#define PHY_ID2_REG                 0x03U /*!< The PHY ID two register. */
#define PHY_AUTONEG_ADVERTISE_REG   0x04U /*!< The PHY auto-negotiate advertise register. */
#define PHY_AUTONEG_LINKPARTNER_REG 0x05U /*!< The PHY auto negotiation link partner ability register. */
#define PHY_AUTONEG_EXPANSION_REG   0x06U /*!< The PHY auto negotiation expansion register. */
#define PHY_1000BASET_CONTROL_REG   0x09U /*!< The PHY 1000BASE-T control register. */
#define PHY_MMD_ACCESS_CONTROL_REG  0x0DU /*!< The PHY MMD access control register. */
#define PHY_MMD_ACCESS_DATA_REG     0x0EU /*!< The PHY MMD access data register. */

#define PHY_PHYCR_REG				0x10U
#define PHY_STS_REG					0x11U
#define PHY_CFG2_REG				0x14U
#define PHY_BIST_REG				0x16U
#define PHY_STS2_REG				0x17U
#define PHY_LED1_REG				0x18U
#define PHY_LED2_REG				0x19U
#define PHY_LED3_REG				0x1AU
#define PHY_CFG3_REG				0x1EU
#define PHY_CTRL_REG				0x1FU
#define PHY_TMCH_CTRL_REG			0x25U
#define PHY_CFG4_REG				0x31U
#define PHY_RGMIICTRL_REG			0x32U
#define PHY_RGMIICTRL2_REG			0x33U
#define PHY_100CR_REG				0x43U
#define PHY_STRAP_STS1_REG			0x6EU
#define PHY_STRAP_STS2_REG			0x6FU
#define PHY_RGMIIDCTL_REG			0x86U
#define PHY_PLLCTL_REG				0xC6U
#define PHY_SYNC_FIFO_CTRL_REG		0xE9U
#define PHY_LOOPCR_REG				0xFEU
#define PHY_IO_MUX_CFG_REG			0x170U
#define PHY_PROG_GAIN_REG			0x1D5U		// Removed from Revision G

/*!
 * @addtogroup phy_driver
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*! @brief PHY driver version */
#define FSL_PHY_DRIVER_VERSION (MAKE_VERSION(2, 0, 0))

typedef struct _phy_dp83867ir_resource
{
    mdioWrite write;
    mdioRead read;
} phy_dp83867ir_resource_t;

/*! @brief PHY operations structure. */
extern const phy_operations_t phydp83867ir_ops;

/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @name PHY Driver
 * @{
 */

/*!
 * @brief Initializes PHY.
 * This function initializes PHY.
 *
 * @param handle       PHY device handle.
 * @param config       Pointer to structure of phy_config_t.
 * @retval kStatus_Success  PHY initialization succeeds
 * @retval kStatus_Fail  PHY initialization fails
 * @retval kStatus_Timeout  PHY MDIO visit time out
 */
status_t PHY_DP83867IR_Init(phy_handle_t *handle, const phy_config_t *config);

/*!
 * @brief PHY Write function.
 * This function writes data over the MDIO to the specified PHY register.
 *
 * @param handle  PHY device handle.
 * @param phyReg  The PHY register.
 * @param data    The data written to the PHY register.
 * @retval kStatus_Success     PHY write success
 * @retval kStatus_Timeout  PHY MDIO visit time out
 */
status_t PHY_DP83867IR_Write(phy_handle_t *handle, uint16_t phyReg, uint16_t data);

/*!
 * @brief PHY Read function.
 * This interface reads data over the MDIO from the specified PHY register.
 *
 * @param handle  PHY device handle.
 * @param phyReg  The PHY register.
 * @param pData   The address to store the data read from the PHY register.
 * @retval kStatus_Success  PHY read success
 * @retval kStatus_Timeout  PHY MDIO visit time out
 */
status_t PHY_DP83867IR_Read(phy_handle_t *handle, uint16_t phyReg, uint16_t *pData);

/*!
 * @brief Gets the PHY auto-negotiation status.
 *
 * @param handle  PHY device handle.
 * @param status  The auto-negotiation status of the PHY.
 *         - true the auto-negotiation is over.
 *         - false the auto-negotiation is on-going or not started.
 * @retval kStatus_Success  PHY gets status success
 * @retval kStatus_Timeout  PHY MDIO visit time out
 */
status_t PHY_DP83867IR_GetAutoNegotiationStatus(phy_handle_t *handle, bool *status);

/*!
 * @brief Gets the PHY link status.
 *
 * @param handle   PHY device handle.
 * @param status   The link up or down status of the PHY.
 *         - true the link is up.
 *         - false the link is down.
 * @retval kStatus_Success   PHY gets link status success
 * @retval kStatus_Timeout  PHY MDIO visit time out
 */
status_t PHY_DP83867IR_GetLinkStatus(phy_handle_t *handle, bool *status);

/*!
 * @brief Gets the PHY link speed and duplex.
 *
 * @brief This function gets the speed and duplex mode of PHY. User can give one of speed
 * and duplex address paramter and set the other as NULL if only wants to get one of them.
 *
 * @param handle   PHY device handle.
 * @param speed    The address of PHY link speed.
 * @param duplex   The link duplex of PHY.
 * @retval kStatus_Success   PHY gets link speed and duplex success
 * @retval kStatus_Timeout  PHY MDIO visit time out
 */
status_t PHY_DP83867IR_GetLinkSpeedDuplex(phy_handle_t *handle, phy_speed_t *speed, phy_duplex_t *duplex);

/*!
 * @brief Sets the PHY link speed and duplex.
 *
 * @param handle   PHY device handle.
 * @param speed    Specified PHY link speed.
 * @param duplex   Specified PHY link duplex.
 * @retval kStatus_Success   PHY gets status success
 * @retval kStatus_Timeout  PHY MDIO visit time out
 */
status_t PHY_DP83867IR_SetLinkSpeedDuplex(phy_handle_t *handle, phy_speed_t speed, phy_duplex_t duplex);

/*!
 * @brief Enables/Disables PHY loopback.
 *
 * @param handle   PHY device handle.
 * @param mode     The loopback mode to be enabled, please see "phy_loop_t".
 * All loopback modes should not be set together, when one loopback mode is set
 * another should be disabled.
 * @param speed    PHY speed for loopback mode.
 * @param enable   True to enable, false to disable.
 * @retval kStatus_Success  PHY loopback success
 * @retval kStatus_Timeout  PHY MDIO visit time out
 */
status_t PHY_DP83867IR_EnableLoopback(phy_handle_t *handle, phy_loop_t mode, phy_speed_t speed, bool enable);

/*!
 * @brief Enables/Disables PHY link management interrupt.
 *
 * This function controls link status change interrupt.
 *
 * @param handle  PHY device handle.
 * @param type    PHY interrupt type.
 * @param enable  True to enable, false to disable.
 * @retval kStatus_Success  PHY enables/disables interrupt success
 * @retval kStatus_Timeout  PHY MDIO visit time out
 */
status_t PHY_DP83867IR_EnableLinkInterrupt(phy_handle_t *handle, phy_interrupt_type_t type, bool enable);

/*!
 * @brief Clears PHY interrupt status.
 *
 * @param handle  PHY device handle.
 * @retval kStatus_Success  PHY read and clear interrupt success
 * @retval kStatus_Timeout  PHY MDIO visit time out
 */
status_t PHY_DP83867IR_ClearInterrupt(phy_handle_t *handle);

/* @} */

#if defined(__cplusplus)
}
#endif

/*! @}*/

#endif /* _FSL_PHY_H_ */
