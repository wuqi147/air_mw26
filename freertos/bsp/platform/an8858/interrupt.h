#ifndef __INTERRUPT__H__
#define __INTERRUPT__H__

typedef void (*isr_t)(void);
void set_interrupt_priority (unsigned int intSrcNum, unsigned int priority);
void enable_interrupt_mask (unsigned int intSrcNum);
void register_isr (unsigned int intSrcNum, isr_t isr);
void unregister_isr (unsigned int intSrcNum);
void disable_interrupt_mask (unsigned int intSrcNum);
void air_trigger_swint(void);
int isIntrEdgeTriggered (unsigned int intSrc);
void clearIntrPendingBit (unsigned int intSrc);

#endif
