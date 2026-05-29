/* FILE NAME:  mbedtls_porting.h
 * PURPOSE:
 * It provides mbedtls porting module API and definitions.
 *
 * NOTES:
 */

#ifndef MBEDTLS_PORTING_H
#define MBEDTLS_PORTING_H

/* INCLUDE FILE DECLARATIONS
 */

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* FUNCTION NAME: mbedtls_porting_increase_task_priority
 * PURPOSE:
 *      adjust the priority of task
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *      None
 */
void
mbedtls_porting_increase_task_priority(
    void);

/* FUNCTION NAME: mbedtls_porting_restore_task_priority
 * PURPOSE:
 *      restore the priority of task
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *      None
 */
void
mbedtls_porting_restore_task_priority(
    void);

/* FUNCTION NAME: rstp_get_task_priority
 * PURPOSE:
 *      Get the RSTP task priority
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      The RSTP task priority
 *
 * NOTES:
 *      None
 */

#endif /* End of MBEDTLS_PORTING_H */