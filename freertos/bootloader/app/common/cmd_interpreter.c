/* Standard includes. */
#include <string.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"

/* Utils includes. */
#include "cmd_interpreter.h"

typedef struct CMD_INPUT_LIST_S
{
	const CMD_LINE_INPUT_T *cmd_line;
	struct CMD_INPUT_LIST_S *next;
} CMD_INPUT_LIST_T;

static portBASE_TYPE help_cmd(char *buf, size_t len, const char *cmd);

static signed char cmd_get_parameter_num(const char *cmd);

static const CMD_LINE_INPUT_T cmd_help =
{
	"help",
	"help: lists all the registered commands\r\n",
	help_cmd,
	0
};

static CMD_INPUT_LIST_T g_registered_cmds =
{
	&cmd_help,
	NULL
};

static signed char g_output_buf[configCOMMAND_INT_MAX_OUTPUT_SIZE];

const char *hidden_cmd[] = {"raw_memrl",
                            "raw_memwl",
                            "raw_memory",
                            "kill_wdog",
                            "xmodem_rcv",
#if defined(AIR_8851_SUPPORT)
                            "sif-read",
                            "sif-write",
                            "gpio-request",
                            "gpio-free",
                            "gpio-set-dir",
                            "gpio-get-dir",
                            "gpio-set-data",
                            "gpio-get-data",
#endif /* AIR_8851_SUPPORT */
                            "flash-show-partition",
                            "port-stats"};

portBASE_TYPE cmd_register(const CMD_LINE_INPUT_T* const cmd)
{
    static CMD_INPUT_LIST_T *last_list = &g_registered_cmds;
    CMD_INPUT_LIST_T *new_list;

	if (!cmd)
    {
        return pdFAIL;
    }

	new_list = (CMD_INPUT_LIST_T *)pvPortMalloc(sizeof(CMD_INPUT_LIST_T));
	if (!new_list)
    {
        return pdFAIL;
    }

	taskENTER_CRITICAL();
	{
		/* Reference the command being registered from the newly created
		list item. */
		new_list->cmd_line = cmd;

		/* The new list item will get added to the end of the list, so
		pxNext has nowhere to point. */
		new_list->next = NULL;

		/* Add the newly created list item to the end of the already existing
		list. */
		last_list->next = new_list;

		/* Set the end of list marker to the new list item. */
		last_list = new_list;
	}
	taskEXIT_CRITICAL();

	return pdPASS;
}

#if 0//TreyDbg
#define strncmp(s1,s2,n) air_strncmp(s1,s2,n)
int air_strncmp (char *s1, char *s2, unsigned int n)
{
    int i;

    if (n==0) return -1;

    printf("[%s][%s](%d)\n", s1, s2, n);

    for (i=0; i<n; i++,s1++,s2++) {
        if ((VPchar(s1))!=(VPchar(s2))) {
                printf("s1[%d]:%c(at 0x%x) != s2[%d]:%c(at 0x%x)\n", i, VPchar(s1), s1, i, VPchar(s2), s2);
                return (VPchar(s1))-(VPchar(s2));
        }
    }

    printf("%s OK\n", __func__);
    return 0;
}
#endif

portBASE_TYPE cmd_process(const char *input, char *buf, size_t len)
{
    static const CMD_INPUT_LIST_T *cmd = NULL;
    portBASE_TYPE ret = pdTRUE;
    const char *str;

    if (cmd == NULL)
    {
    	for(cmd = &g_registered_cmds; cmd != NULL; cmd = cmd->next)
    	{
    		str = cmd->cmd_line->cmd;
    		if(strncmp((const char *)input, (const char *)str, strlen((const char *)str)) == 0)
    		{
    			if(cmd->cmd_line->parameter_num > 0)
    			{
    				if( cmd_get_parameter_num(input) != cmd->cmd_line->parameter_num)
    				{
    					ret = pdFALSE;
    				}
    			}

    			break;
    		}
    	}
    }

	if((cmd != NULL ) && (ret == pdFALSE))
	{
		strncpy(buf, "Incorrect command parameter(s).  Enter \"help\" to view a list of available commands.", len);
		cmd = NULL;
	}
	else if(cmd != NULL)
	{
		ret = cmd->cmd_line->callback(buf, len, input);
		/* If xReturn is pdFALSE, then no further strings will be returned
		after this one, and	pxCommand can be reset to NULL ready to search
		for the next entered command. */
		if( ret == pdFALSE )
		{
			cmd = NULL;
		}
	}
	else
	{
		strncpy((char *)buf, ( const char * const ) "Command not recognised.  Enter \"help\" to view a list of available commands.", len );
		ret = pdFALSE;
	}

	return ret;
}

signed char *cmd_get_output_buf( void )
{
	return g_output_buf;
}

unsigned portBASE_TYPE cmd_get_output_buf_size( void )
{
	return configCOMMAND_INT_MAX_OUTPUT_SIZE;
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

static portBASE_TYPE help_cmd(char *buf, size_t len, const char *input)
{
    static const CMD_INPUT_LIST_T * cmd = NULL;
    signed portBASE_TYPE ret;
	unsigned char i = 0;

	( void ) input;

    if (cmd == NULL)
    {
        cmd = &g_registered_cmds;
    }

	strncpy((char *)buf, (const char *)cmd->cmd_line->help, len);
	cmd = cmd->next;

	if( cmd == NULL )
	{
		ret = pdFALSE;
	}
	else
	{
		for(i = 0; i < sizeof(hidden_cmd)/sizeof(hidden_cmd[0]); i++)
		{
			if(strcmp((const char *)hidden_cmd[i], (const char *)cmd->cmd_line->cmd) == 0)
			{
				cmd = cmd->next;
				if(cmd == NULL)
				    return pdFALSE;
			}
		}
		ret = pdTRUE;
	}

	return ret;
}

static signed char cmd_get_parameter_num(const char *cmd)
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

