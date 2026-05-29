#ifndef TYPEDEFS_H
#define TYPEDEFS_H

#include <stdint.h>

#define FIELD           uint32_t

#define WIDTH1_MSK      (0x1)
#define WIDTH2_MSK      (0x3)
#define WIDTH3_MSK      (0x7)
#define WIDTH4_MSK      (0xF)
#define WIDTH5_MSK      (0x1F)
#define WIDTH6_MSK      (0x3F)
#define WIDTH7_MSK      (0x7F)
#define WIDTH8_MSK      (0xFF)

#define WIDTH16_MSK     (0xFFFF)

#define MASK(n)         (~((~0U << (n))))

#endif