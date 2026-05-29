#define SPI_DRIVER_DEBUG_MODE 0
#define VPint   *(volatile unsigned int *)

#if SPI_DRIVER_DEBUG_MODE
#define SPI_DEBUG(...) simple_printf(__VA_ARGS__)
#else
#define SPI_DEBUG(...)
#endif

unsigned int onram_io_read32(unsigned int addr);
void onram_io_write32(unsigned int addr, unsigned int value);
int onram_outbyte(char c);
int simple_printf(const char *fmt, ...);
void onram_memcpy(void *dst, void *src, unsigned int n);