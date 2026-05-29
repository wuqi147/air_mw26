#include <stdarg.h>
#include "compiler.h"

#define SPI_DRIVER_DEBUG_MODE 0
#define VPint   *(volatile unsigned int *)

#if SPI_DRIVER_DEBUG_MODE
#define SPI_DEBUG(...) simple_printf(__VA_ARGS__)
#else
#define SPI_DEBUG(...)
#endif

int __noex9__ onram_outbyte(char c);
int __noex9__ simple_printf(const char *fmt, ...);
unsigned int __noex9__ onram_strlen(const char *s);
void __noex9__  onram_memcpy(void *dst, void *src, unsigned int n);
