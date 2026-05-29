/* Standard includes. */
#include <string.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"

/* Utils includes. */
#include "cmd_interpreter.h"

const char * const hidden_cmd[] = {"echo", "fpga-init", "memory", "cpu_reg_dump", "kill_wdog", "xmodem_rcv", "tftp_get", "upgrade",
                             "pdma_init", "pdma_debug", "lwip_stats", "httpclient_start", "sshd"};

static portBASE_TYPE help_cmd(signed char *buf, size_t len, const signed char *cmd);
static portBASE_TYPE debug_help_cmd(signed char *buf, size_t len, const signed char *cmd);

static signed char cmd_get_parameter_num(const signed char * cmd);

static const CMD_LINE_INPUT_T cmd_help =
{
    (const signed char * const) "help",
    (const signed char * const) "help: Lists all the registered commands\r\n",
    help_cmd,
    0
};

static const CMD_LINE_INPUT_T debug_cmd_help =
{
    (const signed char * const) "help",
    (const signed char * const) "help: Lists all the registered commands\r\n",
    debug_help_cmd,
    0
};

typedef struct CMD_LIST_S
{
    const CMD_LINE_INPUT_T  *cmd_default;
    const CMD_LINE_INPUT_T  *cmd_combo;
} CMD_LIST_T;

static CMD_LIST_T g_registered_cmds =
{
    &cmd_help,
    NULL
};

static CMD_LIST_T g_registered_debug_cmds =
{
    &debug_cmd_help,
    NULL
};

portBASE_TYPE cmd_register(const CMD_LINE_INPUT_T* const cmd, CMD_PRIVILEGE_MODE_TYPE_T mode)
{
    taskENTER_CRITICAL();
    {
        if (CMD_PRIVILEGE_MODE_DEBUG == mode)
        {
            g_registered_debug_cmds.cmd_combo = cmd;
        }
        else
        {
            g_registered_cmds.cmd_combo = cmd;
        }
    }
    taskEXIT_CRITICAL();

    return pdPASS;
}

portBASE_TYPE cmd_process(const signed char * const input, signed char * buf, size_t len, CMD_PRIVILEGE_MODE_TYPE_T mode)
{
    CMD_LIST_T* p_cmd_list;

    if(CMD_PRIVILEGE_MODE_DEBUG == mode)
    {
        p_cmd_list = &g_registered_debug_cmds;
    }
    else
    {
        p_cmd_list = &g_registered_cmds;
    }

    CMD_LINE_INPUT_T* p_cmd_line = p_cmd_list->cmd_default;

    int n = 0;
    while(1)
    {
        if( strncmp( input, p_cmd_line->cmd, strlen(p_cmd_line->cmd)) == 0)
        {
            if( p_cmd_line->parameter_num > 0 )
            {
                if( cmd_get_parameter_num(input) != p_cmd_line->parameter_num)
                {
                    strncpy((char *) buf, "Incorrect command parameter(s).  Enter \"help\" to view a list of available commands.", len );
                    return pdFALSE;
                }
            }

            return p_cmd_line->callback(buf, len, input);
        }

        p_cmd_line = &(p_cmd_list->cmd_combo[n++]);
        if( p_cmd_line->callback == NULL )
        {
            break;
        }
    }

    strncpy((char *)buf, ( const char * const ) "Command not recognised.  Enter \"help\" to view a list of available commands.", len );
    return pdFALSE;
}

static portBASE_TYPE help_cmd(signed char *buf, size_t len, const signed char *input)
{
    ( void ) input;

    static uint8_t s_idx = 0xff;

    CMD_LIST_T* p_cmd_list = &g_registered_cmds;
    CMD_LINE_INPUT_T* p_cmd_line;
    if (s_idx == 0xff)
    {

        p_cmd_line = p_cmd_list->cmd_default;
    }
    else
    {
        p_cmd_line = &(p_cmd_list->cmd_combo[s_idx]);
    }

    strncpy((char *)buf, p_cmd_line->help, len);

    // peak next cmd line
    while(1)
    {
        ++s_idx;

        p_cmd_line = &(p_cmd_list->cmd_combo[s_idx]);

        if( p_cmd_list->cmd_combo[s_idx].callback == NULL )
        {
            s_idx = 0xff;
            return pdFALSE;
        }

        if( !strcmp((const char *)p_cmd_line->cmd, "debug"))
        {
            continue;
        }
        return pdTRUE;
    }
}

static portBASE_TYPE debug_help_cmd(signed char *buf, size_t len, const signed char *input)
{
    ( void ) input;

    static uint8_t s_idx = 0xff;

    CMD_LIST_T* p_cmd_list = &g_registered_debug_cmds;
    CMD_LINE_INPUT_T* p_cmd_line;
    if (s_idx == 0xff)
    {
        p_cmd_line = p_cmd_list->cmd_default;
    }
    else
    {
        p_cmd_line = &(p_cmd_list->cmd_combo[s_idx]);
    }
    strncpy((char *)buf, p_cmd_line->help, len);

    // peak next cmd line
    while(1)
    {
        ++s_idx;
        p_cmd_line = &(p_cmd_list->cmd_combo[s_idx]);
        if( p_cmd_list->cmd_combo[s_idx].callback == NULL )
        {
            s_idx = 0xff;
            return pdFALSE;
        }

        int i;
        for (i = 0; i < (sizeof(hidden_cmd) / sizeof(hidden_cmd[0])); i++)
        {
            if (!strcmp((const char *)p_cmd_line->cmd, hidden_cmd[i]))
            {
                continue;
            }
        }

        return pdTRUE;
    }
}

static signed char cmd_get_parameter_num( const signed char * cmd )
{
    signed char num = 0;
    portBASE_TYPE last_character_was_space = pdFALSE;

    while( *cmd != 0x00 )
    {
        if( ( *cmd ) == ' ' )
        {
            if( last_character_was_space != pdTRUE )
            {
                num++;
                last_character_was_space = pdTRUE;
            }
        }
        else
        {
            last_character_was_space = pdFALSE;
        }

        cmd++;
    }

    return num;
}

const signed char *cmd_get_parameter( const signed char *buf, unsigned portBASE_TYPE wanted_para_num, portBASE_TYPE *length )
{
    unsigned portBASE_TYPE para_found = 0;
    const signed char *p_ret = NULL;

    *length = 0;

    while( para_found < wanted_para_num )
    {
        while( ( ( *buf ) != 0x00 ) && ( ( *buf ) != ' ' ) )
        {
            buf++;
        }

        while( ( ( *buf ) != 0x00 ) && ( ( *buf ) == ' ' ) )
        {
            buf++;
        }

        if( *buf != 0x00 )
        {
            para_found++;

            if( para_found == wanted_para_num )
            {
                p_ret = buf;
                while( ( ( *buf ) != 0x00 ) && ( ( *buf ) != ' ' ) )
                {
                    ( *length )++;
                    buf++;
                }

                break;
            }
        }
        else
        {
            break;
        }
    }

    return p_ret;
}
/*-----------------------------------------------------------*/
