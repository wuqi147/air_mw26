#ifndef CMD_INTERPRETER_H
#define CMD_INTERPRETER_H

#include "queue.h"

typedef enum
{
    CMD_PRIVILEGE_MODE_EXEC = 0,
    CMD_PRIVILEGE_MODE_SDK,
    CMD_PRIVILEGE_MODE_PERIPHERAL,
#ifdef AIR_MW_SUPPORT
    CMD_PRIVILEGE_MODE_MW,
#endif
    CMD_PRIVILEGE_MODE_DEBUG,
    CMD_PRIVILEGE_MODE_LAST,
} CMD_PRIVILEGE_MODE_TYPE_T;

typedef portBASE_TYPE (*cmd_line_callback)(signed char *buf, size_t len, const signed char * input);

typedef struct CMD_LINE_INPUT_S
{
    const signed char * const   cmd;
    const signed char * const   help;
    const cmd_line_callback     callback;
    signed char                 parameter_num;
} CMD_LINE_INPUT_T;

portBASE_TYPE cmd_register(const CMD_LINE_INPUT_T * const cmd, CMD_PRIVILEGE_MODE_TYPE_T mode);

portBASE_TYPE cmd_process(const signed char * const input, signed char * buf, size_t len, CMD_PRIVILEGE_MODE_TYPE_T mode);


const signed char *cmd_get_parameter(const signed char *buf, unsigned portBASE_TYPE wanted_para_num, portBASE_TYPE *length);

QueueHandle_t cmd_queue_get();

#endif /* CMD_INTERPRETER_H */

