/*******************************************************************************
 *  Copyright Statement:
 *  --------------------
 *  This software is protected by Copyright and the information contained
 *  herein is confidential. The software may not be copied and the information
 *  contained herein may not be used or disclosed except with the written
 *  permission of Airoha Technology Corp. (C) 2024
 *
 *******************************************************************************/
/* FILE NAME:  hal_sk49145b_poe.h
 * PURPOSE:
 *      Define SK49145B driver function.
 * NOTES:
 *      Register layout derived from DH2184; all values must be verified against
 *      SK49145B datasheet during hardware bring-up.
 */
#ifndef HAL_SK49145B_POE_H
#define HAL_SK49145B_POE_H

#include <air_error.h>
#include <hal/common/hal_poe.h>

#define SK49145B_TOTAL_PORTS (4)

/* Event registers */
#define SK49145B_INTERRUPT_MASK        (0x01)
#define SK49145B_POWER_EVENT           (0x02)
#define SK49145B_POWER_EVENT_COR       (0x03)
#define SK49145B_DETECT_EVENT          (0x04)
#define SK49145B_ERROR_EVENT           (0x06)
#define SK49145B_ERROR_EVENT_COR       (0x07)
#define SK49145B_START_EVENT           (0x08)
#define SK49145B_START_EVENT_COR       (0x09)
#define SK49145B_SUPPLY_EVENT          (0x0A)
#define SK49145B_SUPPLY_EVENT_COR      (0x0B)
#define SK49145B_PORT_STATUS_BASE      (0x0C)
#define SK49145B_POWER_STATUS          (0x10)
#define SK49145B_OPERATING_MODE        (0x12)
#define SK49145B_DISCONNECT_ENABLE     (0x13)
#define SK49145B_DETECT_CLASS_ENABLE   (0x14)
#define SK49145B_POWER_ENABLE_BUTTON   (0x19)
#define SK49145B_GLOBAL_BUTTON         (0x1A)
#define SK49145B_ID                    (0x1B)
#define SK49145B_PORT_CURRENT_BASE     (0x30)
#define SK49145B_WATCHDOG              (0x42)
#define SK49145B_SCRATCH               (0x43)
#define SK49145B_HIGH_POWER_ENABLE     (0x44)
#define SK49145B_PORT_ICUT_BASE        (0x47)
#define SK49145B_PORT_ILIM_BASE        (0x48)
#define SK49145B_PORT_CFG_STRIDE       (0x05)

/* ID register: SK49145B uses 0x1B, 1-byte ID with different code than DH2184 */
#define SK49145B_ID_CODE_MASK          (0xF8)
#define SK49145B_ID_CODE               (0x90)

/* Operating modes */
#define SK49145B_MODE_SHUTDOWN         (0x0)
#define SK49145B_MODE_MANUAL           (0x1)
#define SK49145B_MODE_SEMI_AUTO        (0x2)
#define SK49145B_MODE_AUTO             (0x3)
#define SK49145B_ALL_PORTS_MASK        (0x0F)
#define SK49145B_ALL_CLASS_DET_MASK    (0xFF)
#define SK49145B_INT_CLR               (0x80)
#define SK49145B_WATCHDOG_DISABLE      (0x16)
#define SK49145B_DEFAULT_SCRATCH_VALUE (0xA5)

AIR_ERROR_NO_T
hal_sk49145b_poe_getDriver(
    HAL_POE_DRIVER_T **pptr_poe_driver);

#endif /* HAL_SK49145B_POE_H */
