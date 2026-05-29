/*
 * FreeRTOS V202012.00
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

/* Standard includes. */
#include "stdlib.h"
#include "string.h"

/* lwIP core includes */
#include "lwip/opt.h"
#include "lwip/sockets.h"

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "cmd_interpreter.h"

/* Dimensions the buffer into which input characters are placed. */
#define cmdMAX_INPUT_SIZE	100

/* Dimensions the buffer into which string outputs can be placed. */
#define cmdMAX_OUTPUT_SIZE	1024

#define cmd_newline		"\r\n#"

#define IAC 255


/*-----------------------------------------------------------*/

/* structure for holding telnet option name and value */
typedef struct
{
	const char *name;
	u8_t value;
} telnet_option_t;

/* supported telnet option values */
static telnet_option_t telnet_options[] =
{
	{"WILL", 251},
	{"WONT", 252},
	{"DO", 253},
	{"DONT", 254},
	{"IAC", IAC},
	{"ECHO", 1},
	{"SGA", 3},
	{"LFLOW", 33},
	{"LINEMODE", 34},
	{NULL, 0}
	/* this list must end with {NULL, 0} */
};

/* Returns the name of a telnet option based on the value. */
static const char* telnet_option_name(int value)
{
	int i = 0;

	for ( i = 0; telnet_options[i].name != NULL; i++ )
	{
		if ( telnet_options[i].value == value )
			return telnet_options[i].name;
	}

	/* default value */
	return NULL;
}

/* Returns the value of a telnet option based on the name. */
static u8_t telnet_option_value(const char* name)
{
	int i = 0;

	for ( i = 0; telnet_options[i].name != NULL; i++ )
	{
		if ( 0 == strcmp(telnet_options[i].name, name) )
			return telnet_options[i].value;
	}

	/* default value */
	return 0;
}
static void telnetd_message_set_character_mode(long lClientFd)
{
	u8_t databuf[16] = {0};
	u8_t len = 0;
#define OPT_A(x)	databuf[x ++]

	memset(databuf, 0, sizeof(databuf));
	/* send a predefined set of commands proven to work */
	OPT_A(len) = telnet_option_value("IAC");
	OPT_A(len) = telnet_option_value("DO");
	OPT_A(len) = telnet_option_value("ECHO");

	OPT_A(len) = telnet_option_value("IAC");
	OPT_A(len) = telnet_option_value("WILL");
	OPT_A(len) = telnet_option_value("ECHO");

	OPT_A(len) = telnet_option_value("IAC");
	OPT_A(len) = telnet_option_value("WILL");
	OPT_A(len) = telnet_option_value("SGA");

	if ( len >= sizeof(databuf) )
	{
		printf("%s(): buffer overflow.\n");
		return;
	}

	send( lClientFd, databuf, len, 0 );
}

enum CONN_STATE
{
	ST_INIT = 0,
	ST_AUTH_U,
	ST_AUTH_P,
	ST_CMD
};

#define ECHO_MSG(SK, MSG, MSG_LEN) send(SK, MSG, MSG_LEN, 0)
#define AUTH_NAME		"Login: "
#define AUTH_PSW		BR"Password: "

#define ACCOUNT_U		"admin"
#define ACCOUNT_P		"1234"

#define BR				"\r\n"
#define AUTH_FAILED  	BR"auth fail!"

enum telnetd_auth_mode {
	TELNETD_AUTH_DEF = 0,
	TELNETD_AUTH_NAME,
	TELNETD_AUTH_NAME_OK,
	TELNETD_AUTH_PASSWORD,
	TELNETD_AUTH_PASSWORD_OK,
	TELNETD_AUTH_END
};


void telnetd_handle_auth_state
(long sk, uint8_t *auth_st, uint8_t *is_auth_ok, char *val)
{
	if ( NULL  == auth_st || NULL == is_auth_ok || NULL == val )
		return;

	if ( TELNETD_AUTH_DEF == *auth_st )
	{
		/* default state, show username: */
		ECHO_MSG(sk, AUTH_NAME, strlen(AUTH_NAME));
		*auth_st = TELNETD_AUTH_NAME;
	}
	else if ( TELNETD_AUTH_NAME == *auth_st )
	{
		if ( 0 == strcmp(val, ACCOUNT_U) )
			*is_auth_ok = TELNETD_AUTH_NAME_OK;
		else
			*is_auth_ok = 0;
		ECHO_MSG(sk, AUTH_PSW, strlen(AUTH_PSW));
		*auth_st = TELNETD_AUTH_PASSWORD;
	}
	else if ( TELNETD_AUTH_PASSWORD == *auth_st )
	{
		if ( TELNETD_AUTH_NAME_OK == *is_auth_ok
			&& 0 == strcmp(val, ACCOUNT_P) )
		{
			*is_auth_ok = TELNETD_AUTH_PASSWORD_OK;
			ECHO_MSG(sk, BR"#", 3);
		}
		else
		{
			*is_auth_ok = 0;
			ECHO_MSG(sk, AUTH_FAILED, strlen(AUTH_FAILED));
		}

		*auth_st = TELNETD_AUTH_END;
	}
}

static void do_back(long sk)
{
	u8_t databuf[16] = {0};
	u8_t len = 0;

	memset(databuf, 0, sizeof(databuf));
	OPT_A(len) = 0x08;
	OPT_A(len) = 0x20;
	OPT_A(len) = 0x08;

	if ( len >= sizeof(databuf) )
	{
		printf("%s(): buffer overflow.\n");
		return;
	}

	send(sk, databuf, len, 0);
}

typedef struct telnetd_param_s
{
	long socket_id;
} telnet_cb_info;

typedef int (*callback)( const unsigned char *databuf, int len, void *param);


int telnet_send( const unsigned char *databuf, int len, void *param)
{
	telnet_cb_info *p_data = (telnet_cb_info *)param;
		
	if ( NULL == p_data || NULL == databuf || 0 == len )
		return;

	send(p_data->socket_id, databuf, len, 0);
	return 0;
}

static void do_LR(/*long sk, */int isLeft, const callback cb, void *param)
{
	u8_t databuf[16] = {0};
	u8_t len = 0;

	memset(databuf, 0, sizeof(databuf));
	OPT_A(len) = 0x1B;
	OPT_A(len) = 0x5B;
	OPT_A(len) = isLeft ? 0x44 : 0x43;

	if ( len >= sizeof(databuf) )
	{
		printf("%s(): buffer overflow.\n");
		return;
	}

	if(cb)
		cb(databuf, len, param);
}


#define TELNET_HISTORY_MAX_NUM	10
static char *g_history_telnet_cmd[TELNET_HISTORY_MAX_NUM];
static int current_history_telnet_index = 0;

void init_history_cmd(char *his_cmd, int cmd_len)
{
	memset(his_cmd, 0, cmd_len);
}

int add_cmd_to_history(char *cmd, char **store_his_cmd, int *store_his_cmd_index, int max_his_cnt)
{
	int cmd_len = 0;
	char *hs_cmd = NULL;

	if ( NULL == cmd || NULL == store_his_cmd_index )
		return -1;

	cmd_len = strlen(cmd);
	if ( cmd_len <= 0 )
		return -2;
	hs_cmd = malloc(cmd_len + 1);
	if ( NULL == hs_cmd )
		return -3;
	memcpy(hs_cmd, cmd, cmd_len);
	hs_cmd[cmd_len] = 0;
	if ( *store_his_cmd_index >= max_his_cnt )
		*store_his_cmd_index = 0;
	if ( NULL != store_his_cmd[*store_his_cmd_index] )
	{
		free(store_his_cmd[*store_his_cmd_index]);
		store_his_cmd[*store_his_cmd_index] = NULL;
	}
	store_his_cmd[(*store_his_cmd_index) ++] = hs_cmd;

	return 0;
}

int restore_histry_cmd
(/*long sk, */char *curr_cmd, int *curr_cmd_len, int cmd_total_len, int is_up, const callback cb, void *param, char **store_his_cmd, int *store_his_cmd_index, int max_his_cnt)
{
	int chk_history_idx = 0, his_cmd_len = 0, back_len = 0;
	u8_t *databuf = NULL;

	if ( NULL == curr_cmd || NULL == curr_cmd_len )
		return -1;

	/* check current history, do nothing if no history. */
	if ( is_up )
	{
		if ( 0 == *store_his_cmd_index )
			chk_history_idx = max_his_cnt - 1;
		else
			chk_history_idx = *store_his_cmd_index - 1;
	}
	else
	{
		chk_history_idx = *store_his_cmd_index + 1;
		if ( chk_history_idx >= max_his_cnt )
			chk_history_idx = 0;
	}

	if ( NULL == store_his_cmd[chk_history_idx] )
		return -2;

	/* remove current command. */
	databuf = malloc(*curr_cmd_len);
	if ( NULL == databuf )
		return -3;
	memset(databuf, 0x08, *curr_cmd_len);
	/* do back */
	if(cb)
		cb(databuf, *curr_cmd_len, param);
	free(databuf);
	databuf = NULL;

	/* send current history cmd */
	his_cmd_len = strlen(store_his_cmd[chk_history_idx]);
	if(cb)
		cb(store_his_cmd[chk_history_idx], his_cmd_len, param);

	/* go back if histroy cmd is short. */
	if ( his_cmd_len < *curr_cmd_len )
	{
		back_len = *curr_cmd_len - his_cmd_len;
		databuf = malloc(back_len);
		if ( NULL == databuf )
			return -4;
		memset(databuf, 0x20, back_len);
		/* space overwrite */
		if(cb)
			cb(databuf, back_len, param);
		memset(databuf, 0x08, back_len);
		/* do back */
		if(cb)
			cb(databuf, back_len, param);
		free(databuf);
		databuf = NULL;
	}

	/* change history index and store current cmd. */
	*store_his_cmd_index = chk_history_idx;
	memset( curr_cmd, 0x00, cmd_total_len );
	*curr_cmd_len= snprintf(curr_cmd, cmd_total_len, "%s", store_his_cmd[chk_history_idx]);

	return 0;
}

int direction_handle
(uint8_t is_ESC, unsigned char cInChar, char *cmd, long *cmd_len, int cmd_total_len, const callback cb, void *param, char **g_history_cmd, int *current_history_index, int max_his_cnt)
{
	/* arrow up / down/ left / right */
	if ( is_ESC == 1)
	{
		if ( 0x5B == cInChar )
		{
			return 1; /* for NEXT. */
		}
		else if ( 0x41 == cInChar ) /* up */
		{
			restore_histry_cmd(cmd, cmd_len, cmd_total_len, 1, cb, param, g_history_cmd, current_history_index, max_his_cnt);
		}
		else if ( 0x42 == cInChar ) /* down */
		{
			restore_histry_cmd(cmd, cmd_len, cmd_total_len, 0, cb, param, g_history_cmd, current_history_index, max_his_cnt);
		}
		else if ( 0x43 == cInChar ) /* right */
		{
			if( *cmd_len >= 0
				&& cmd+(*cmd_len) != NULL )
			{
				(*cmd_len) ++;
				do_LR(0, cb, param);
			}
		}
		else if ( 0x44 == cInChar ) /* left */
		{
			if( *cmd_len > 0 )
			{
				(*cmd_len) --;
				do_LR(1, cb, param);
			}
		}
		
	}
	return 0;
}

void TelnetD_CommandInterpreterTask( void *pvParameters )
{
	long lSocket, lClientFd, lBytes, lAddrLen = sizeof( struct sockaddr_in ), lInputIndex;
	struct sockaddr_in sLocalAddr;
	struct sockaddr_in client_addr;
	const char *pcWelcomeMessage = "FreeRTOS command server - connection accepted.\r\nType Help to view a list of registered commands.\r\n\r\n#";
	unsigned char cInChar;
	static char cInputString[ cmdMAX_INPUT_SIZE ], cOutputString[ cmdMAX_OUTPUT_SIZE ];
	portBASE_TYPE xReturned;
	uint8_t auth_st = 0, is_auth_ok = 0, is_ESC = 0;
	telnet_cb_info cb_data;

	init_history_cmd(g_history_telnet_cmd, sizeof(g_history_telnet_cmd));
	lSocket = socket(AF_INET, SOCK_STREAM, 0);

	if( lSocket >= 0 )
	{
		memset((char *)&sLocalAddr, 0, sizeof(sLocalAddr));
		sLocalAddr.sin_family = AF_INET;
		sLocalAddr.sin_len = sizeof(sLocalAddr);
		sLocalAddr.sin_addr.s_addr = htonl(INADDR_ANY);
		sLocalAddr.sin_port = ntohs( ( ( unsigned short ) 23 ) );

		if( bind( lSocket, ( struct sockaddr *) &sLocalAddr, sizeof( sLocalAddr ) ) < 0 )
		{
			close( lSocket );
			vTaskDelete( NULL );
		}

		if( listen( lSocket, 20 ) != 0 )
		{
			close( lSocket );
			vTaskDelete( NULL );
		}

		for( ;; )
		{

			lClientFd = accept(lSocket, ( struct sockaddr * ) &client_addr, ( u32_t * ) &lAddrLen );

			if( lClientFd > 0L )
			{
				telnetd_message_set_character_mode( lClientFd );
				
				memset(&cb_data, 0, sizeof(cb_data));
				cb_data.socket_id = lClientFd;

				//send( lClientFd, pcWelcomeMessage, strlen( ( const char * ) pcWelcomeMessage ), 0 );

				lInputIndex = 0;
				memset( cInputString, 0x00, cmdMAX_INPUT_SIZE );
				auth_st = 0;
				is_auth_ok = 0;
				telnetd_handle_auth_state(lClientFd, &auth_st, &is_auth_ok, "");
				do
				{
					lBytes = recv( lClientFd, &cInChar, sizeof( cInChar ), 0 );

					if( lBytes > 0L )
					{
						if( cInChar == '\n' )
						{
							if ( TELNETD_AUTH_PASSWORD_OK != is_auth_ok )
							{
								telnetd_handle_auth_state(lClientFd, &auth_st, &is_auth_ok, cInputString);
								if ( TELNETD_AUTH_END == auth_st
									&& 0 == is_auth_ok )
									lBytes = 0L; /* close */
								lInputIndex = 0;
								memset( cInputString, 0x00, cmdMAX_INPUT_SIZE );
							}
							else
							{
								/* The input string has been terminated.  Was the
								input a quit command? */
								if( strcmp( "exit", ( const char * ) cInputString ) == 0 )
								{
									/* Set lBytes to 0 to close the connection. */
									lBytes = 0L;
								}
								else if ( 0 == cInputString[0] )
								{
									ECHO_MSG(lClientFd, BR"#", 3);
								}
								else
								{
									ECHO_MSG(lClientFd, BR, 2);
									/* The input string was not a quit command.
									Pass the string to the command interpreter. */
									do
									{
										/* Get the next output string from the command interpreter. */
										xReturned = cmd_process(cInputString, cOutputString, sizeof(cOutputString), CMD_PRIVILEGE_MODE_EXEC);
										send( lClientFd, cOutputString, strlen( ( const char * ) cOutputString ), 0 );

									} while( xReturned != pdFALSE );
									add_cmd_to_history(cInputString, g_history_telnet_cmd, &current_history_telnet_index, TELNET_HISTORY_MAX_NUM );

									/* All the strings generated by the input
									command have been sent.  Clear the input
									string ready to receive the next command. */
									lInputIndex = 0;
									memset( cInputString, 0x00, cmdMAX_INPUT_SIZE );
									ECHO_MSG(lClientFd, cmd_newline, strlen( cmd_newline ));
								}
							}
						}
						else
						{
							if( cInChar == '\r' )
							{
								/* Ignore the character. */
							}
							else if( cInChar == '\b' )
							{
								/* Backspace was pressed.  Erase the last
								character in the string - if any. */
								if( lInputIndex > 0 && strlen(cInputString) <= lInputIndex )
								{
									do_back(lClientFd);
									lInputIndex--;
									cInputString[ lInputIndex ] = '\0';
								}
							}
							/* add for IAC etc. */
							else if ( IAC == cInChar )
							{
								/*ignore it */
								unsigned char tmp_buf[2];
								recv( lClientFd, tmp_buf, sizeof( tmp_buf ), 0 );
							}
							/* ESC */
							else if ( 0x1B == cInChar )
							{
								is_ESC = 1;
								continue; /* for NEXT. */
							}
							/* arrow up / down/ left / right */
							else if ( is_ESC )
							{
								if(direction_handle(is_ESC, cInChar, cInputString, &lInputIndex, sizeof(cInputString), telnet_send, 
									&cb_data, g_history_telnet_cmd, &current_history_telnet_index, TELNET_HISTORY_MAX_NUM) == 1)
									continue; /* for NEXT. */
								
								/*if ( 0x5B == cInChar )
								{
									continue; 
								}
								else if ( 0x41 == cInChar ) 
								{
									restore_histry_cmd(lClientFd, cInputString, &lInputIndex, sizeof(cInputString), 1);
								}
								else if ( 0x42 == cInChar ) 
								{
									restore_histry_cmd(lClientFd, cInputString, &lInputIndex, sizeof(cInputString), 0);
								}
								else if ( 0x43 == cInChar ) 
								{
									if( lInputIndex >= 0
										&& cInputString[ lInputIndex ] )
									{
										lInputIndex ++;
										do_LR(lClientFd, 0);
									}
								}
								else if ( 0x44 == cInChar ) 
								{
									if( lInputIndex > 0 )
									{
										lInputIndex --;
										do_LR(lClientFd, 1);
									}
								}*/
								
							}
							else
							{
								/* A character was entered.  Add it to the string
								entered so far.  When a \n is entered the complete
								string will be passed to the command interpreter. */
								if( lInputIndex < cmdMAX_INPUT_SIZE )
								{
									cInputString[ lInputIndex ] = cInChar;
									lInputIndex++;
								}

								if ( TELNETD_AUTH_PASSWORD == auth_st )
									ECHO_MSG(lClientFd, "*", 1);
								else
									ECHO_MSG(lClientFd, &cInChar, 1);
							}

							is_ESC = 0;
						}
					}

				} while( lBytes > 0L );

				 close( lClientFd );
			}
		}
	}

	/* Will only get here if a listening socket could not be created. */
	vTaskDelete( NULL );
}

