#ifndef CMD_INTERPRETER_H
#define CMD_INTERPRETER_H

typedef portBASE_TYPE (*cmd_line_callback)(char *buf, size_t len, const char *input);

typedef struct CMD_LINE_INPUT_S
{
	const char *cmd;
	const char *help;
	const cmd_line_callback     callback;
	signed char                 parameter_num;
} CMD_LINE_INPUT_T;

portBASE_TYPE cmd_register(const CMD_LINE_INPUT_T * const cmd);

portBASE_TYPE cmd_process(const char *input, char *buf, size_t len);

signed char *cmd_get_output_buf(void);
unsigned portBASE_TYPE cmd_get_output_buf_size(void);

const signed char *cmd_get_parameter(const signed char *buf, unsigned portBASE_TYPE wanted_para_num, portBASE_TYPE *length);

#endif /* CMD_INTERPRETER_H */

