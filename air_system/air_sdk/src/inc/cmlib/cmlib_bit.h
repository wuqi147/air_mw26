/* FILE NAME:   cmlib_bit.h
 * PURPOSE:
 *      This file is used to provide bit operation macro.
 * NOTES:
 */

#ifndef CMLIB_BIT_H
#define CMLIB_BIT_H

/* INCLUDE FILE DECLARATIONS
 */

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */
#ifndef BIT
#define BIT(nr) (1UL << (nr))
#endif /* End of BIT */

/* bits range: for example BITS(16,23) = 0xFF0000*/
#ifndef BITS
#define BITS(m, n) (~(BIT(m) - 1) & ((BIT(n) - 1) | BIT(n)))
#endif /* End of BITS */

/* bits range: for example BITS_RANGE(16,4) = 0x0F0000*/
#ifndef BITS_RANGE
#define BITS_RANGE(offset, range) BITS((offset), ((offset) + (range) - 1))
#endif /* End of BITS_RANGE */

/* bits offset right: for example BITS_OFF_R(0x1234, 8, 4) = 0x2 */
#ifndef BITS_OFF_R
#define BITS_OFF_R(val, offset, range) (((val) >> offset) & (BITS(0, (range) - 1)))
#endif /* End of BITS_OFF_R */

/* bits offset left: for example BITS_OFF_L(0x1234, 8, 4) = 0x400 */
#ifndef BITS_OFF_L
#define BITS_OFF_L(val, offset, range) (((val) & (BITS(0, (range) - 1))) << (offset))
#endif /* End of BITS_OFF_L */

/* get TRUE if the bit of the value is set or get FALSE */
#ifndef BIT_CHK
#define BIT_CHK(value, offset) (0 != ((value) & BIT(offset)))
#endif

/* set the bit to one in the value */
#ifndef BIT_ADD
#define BIT_ADD(value, offset) ((value) |= BIT(offset))
#endif

/* reset the bit to zero in the value */
#ifndef BIT_DEL
#define BIT_DEL(value, offset) ((value) &= ~(BIT(offset)))
#endif

/* DATA TYPE DECLARATIONS
 */

#endif /* CMLIB_BIT_H */
