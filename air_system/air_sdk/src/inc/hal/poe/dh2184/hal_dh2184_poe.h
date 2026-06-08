/*******************************************************************************
 *  Copyright Statement:
 *  --------------------
 *  This software is protected by Copyright and the information contained
 *  herein is confidential. The software may not be copied and the information
 *  contained herein may not be used or disclosed except with the written
 *  permission of Airoha Technology Corp. (C) 2021
 *
 *******************************************************************************/
/* FILE NAME:  hal_dh2184_poe.h
 * PURPOSE:
 *      Define DH2184 driver function.
 * NOTES:
 */
#ifndef HAL_DH2184_POE_H
#define HAL_DH2184_POE_H

#include <air_error.h>
#include <hal/common/hal_poe.h>

#define DH2184_TOTAL_PORTS (4)

#define DH2184_INTERRUPT_MASK        (0x01)
#define DH2184_POWER_EVENT           (0x02)
#define DH2184_POWER_EVENT_COR       (0x03)
#define DH2184_DETECT_EVENT          (0x04)
#define DH2184_ERROR_EVENT           (0x06)
#define DH2184_ERROR_EVENT_COR       (0x07)
#define DH2184_START_EVENT           (0x08)
#define DH2184_START_EVENT_COR       (0x09)
#define DH2184_SUPPLY_EVENT          (0x0A)
#define DH2184_SUPPLY_EVENT_COR      (0x0B)
#define DH2184_PORT_STATUS_BASE      (0x0C)
#define DH2184_POWER_STATUS          (0x10)
#define DH2184_OPERATING_MODE        (0x12)
#define DH2184_DISCONNECT_ENABLE     (0x13)
#define DH2184_DETECT_CLASS_ENABLE   (0x14)
#define DH2184_POWER_ENABLE_BUTTON   (0x19)
#define DH2184_GLOBAL_BUTTON         (0x1A)
#define DH2184_ID                    (0x1B)
#define DH2184_PORT_CURRENT_BASE     (0x30)
#define DH2184_WATCHDOG              (0x42)
#define DH2184_SCRATCH               (0x43)
#define DH2184_HIGH_POWER_ENABLE     (0x44)
#define DH2184_PORT_ICUT_BASE        (0x47)
#define DH2184_PORT_ILIM_BASE        (0x48)
#define DH2184_PORT_CFG_STRIDE       (0x05)

#define DH2184_ID_CODE_MASK          (0xF8)
#define DH2184_ID_CODE               (0xD0)
#define DH2184_MODE_SHUTDOWN         (0x0)
#define DH2184_MODE_MANUAL           (0x1)
#define DH2184_MODE_SEMI_AUTO        (0x2)
#define DH2184_MODE_AUTO             (0x3)
#define DH2184_ALL_PORTS_MASK        (0x0F)
#define DH2184_ALL_CLASS_DET_MASK    (0xFF)
#define DH2184_INT_CLR               (0x80)
#define DH2184_WATCHDOG_DISABLE      (0x16)
#define DH2184_DEFAULT_SCRATCH_VALUE (0xA5)

AIR_ERROR_NO_T
hal_dh2184_poe_getDriver(
    HAL_POE_DRIVER_T **pptr_poe_driver);

#endif /* HAL_DH2184_POE_H */
