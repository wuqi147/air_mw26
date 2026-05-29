#ifndef __ERPS_HW_INIT__H__
#define __ERPS_HW_INIT__

#include <platform.h>

typedef void timer_callback_t(void *);

#ifdef CONFIG_ERPS_API_DEMO
#define PSR_P3_P0_REG               (0x10218030)
#define PSR_P7_P4_REG               (0x10218034)
#define PER_PSR_PORTS               (4)
#define PER_PSR_OFFSET              (8)
#define PSR_LINK_STS_MASK           (0x00000001)
#define GET_PSR_LINK_STS(data, mac_port)    ((data & (PSR_LINK_STS_MASK << ((mac_port % PER_PSR_PORTS) * PER_PSR_OFFSET))) >> ((mac_port % PER_PSR_PORTS) * PER_PSR_OFFSET))

#define MAX_PORT                    (8)
#define LINK_DOWN                   (0)
#define LINK_UP                     (1)
#endif

#define ERPS_TIMER                  (1)
#define ERPS_1MS_TICK               (CPUTMRCLK / KHz)
#define ERPS_1US_TICK               (CPUTMRCLK / MHz)

extern void erps_gsw_isr_port_set(unsigned char port_bitmap, unsigned char enable);

extern void erps_timer_init(unsigned int tick, unsigned int jitter);
extern void erps_timer_deinit(void);

extern void erps_gsw_isr_init(void);
extern void erps_timer_start(unsigned int ms, void *callback_data, timer_callback_t *callback_func);
extern void erps_timer_stop(void);
extern unsigned int erps_get_current_tick(void);
#endif