/*
        putchar is the only external dependency for this file,
        if you have a working putchar, leave it commented out.
        If not, uncomment the define below and
        replace outbyte(c) by your own function call.
*/

#define putchar(c) outbyte(c)

#include <stdarg.h>	/* va_list, va_arg() */
#include <stddef.h>
// #include <stdio.h>

#include "do_printf.h"

#ifdef AIR_SUPPORT_REMOTE_DEBUG
#include "FreeRTOS.h"
#include "mw_telnet.h"
#include "remote_debug_log.h"
#endif

struct vsnprintf_info {
    char *dst;
    unsigned int max_len;
    unsigned int cur_len;
};

/*****************************************************************************
 * PRINTF You must write your own putchar()
 *****************************************************************************/
static int vprintf_help(unsigned c, void **ptr){
    extern int putchar(int c);

    ptr = ptr; /* to avoid unused varible warning */
    putchar(c);
#if defined(AIR_SUPPORT_REMOTE_DEBUG) && !defined(AIR_SUPPORT_CLI)
    osapi_write_to_ram(c);
#endif
    return 0;
}


static int vsprintf_help(unsigned int c, void **ptr){

    char *dst = *ptr;
    *dst++ = c;
    *ptr = dst;

    return 0 ;
}

int vsprintf(char *buffer, const char *fmt, va_list args){

    int ret_val = do_printf(fmt, args, vsprintf_help, (void *)buffer);
    buffer[ret_val] = '\0';

    return ret_val;
}

int sprintf(char *buffer, const char *fmt, ...){

    va_list args;
    int ret_val;

    va_start(args, fmt);
    ret_val = vsprintf(buffer, fmt, args);
    va_end(args);

    return ret_val;
}

int vprintf(const char *fmt, va_list args){

    return do_printf(fmt, args, vprintf_help, (void *)0);
}


int printf(const char *fmt, ...){

    va_list args;
    int ret_val;

    va_start(args, fmt);
    ret_val = vprintf(fmt, args);
    va_end(args);

    return ret_val;
}

static int vsnprintf_help(unsigned int c, void **ptr) {

    struct vsnprintf_info *info = (struct vsnprintf_info *)*ptr;
    if (info->cur_len < info->max_len) {
        *(info->dst++) = c;
        info->cur_len++;
    }

    return 0;
}

int vsnprintf(char *buffer, size_t count, const char *fmt, va_list args) {
    struct vsnprintf_info info;
    info.dst = buffer;
    info.max_len = count - 1; // Reserve space for the null terminator
    info.cur_len = 0;

    int ret_val = do_printf(fmt, args, (fnptr_t)vsnprintf_help, (void *)&info);
    buffer[info.cur_len < count ? info.cur_len : count - 1] = '\0'; // Ensure null termination

    return ret_val;
}

int snprintf(char *buffer, size_t count, const char *fmt, ...) {
    va_list args;
    int ret_val;

    va_start(args, fmt);
    ret_val = vsnprintf(buffer, count, fmt, args);
    va_end(args);

    return ret_val;
}
