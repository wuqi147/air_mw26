#ifndef CMLIB_HW_UTIL_H
#define CMLIB_HW_UTIL_H
/* INCLUDE FILE DECLARATIONS
 */
#include <air_error.h>
#include <air_types.h>
#include <hal/common/hal_phy.h>

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* Macros for phy related information */

/* FUNCTION NAME: cmlib_hw_util_version
 * PURPOSE:
 *      Get cmlib_hw_util library version
 * INPUT:
 *      None
 * OUTPUT:
 *      None
 * RETURN:
 *      String of hardware library version
 *
 * NOTES:
 */
C8_T *
cmlib_hw_util_version();

/* FUNCTION NAME: cmlib_hw_util_triggerCableDiag
 * PURPOSE:
 *      Get cable diagnostic result.
 *
 * INPUT:
 *      unit            --  Device id
 *      port            --  Select port number
 *      pair            --  Select test pair
 * OUTPUT:
 *      ptr_result      --  Pointer of cable diag result
 * RETURN:
 *      AIR_E_OK
 *
 * NOTES:
 */
AIR_ERROR_NO_T
cmlib_hw_util_triggerCableDiag(
    const UI32_T               unit,
    const UI32_T               port,
    UI32_T                     pair,
    HAL_PHY_CABLE_TEST_RSLT_T *ptr_result);

/* FUNCTION NAME: cmlib_hw_util_getCableDiagRawData
 * PURPOSE:
 *      Get cable diagnostic ec train raw data.
 *
 * INPUT:
 *      unit            --  Device id
 *      port            --  Select port number
 *      pair            --  Select test pair
 *
 * OUTPUT:
 *      ptr_data        --  Pointer of cable diag raw data
 * RETURN:
 *      AIR_E_OK
 *
 * NOTES:
 */
AIR_ERROR_NO_T
cmlib_hw_util_getCableDiagRawData(
    const UI32_T unit,
    const UI32_T port,
    UI32_T       pair,
    UI32_T      *ptr_data);

/* FUNCTION NAME: cmlib_hw_util_triggerLinkDownCableDiag
 * PURPOSE:
 *      Get fast mode cable diagnostic result.
 *
 * INPUT:
 *      unit            --  Device id
 *      port            --  Select port number
 * OUTPUT:
 *      ptr_result      --  Pointer of cable diag result
 * RETURN:
 *      AIR_E_OK
 *
 * NOTES:
 */
AIR_ERROR_NO_T
cmlib_hw_util_triggerLinkDownCableDiag(
    const UI32_T               unit,
    const UI32_T               port,
    HAL_PHY_CABLE_TEST_RSLT_T *ptr_result);

#endif
