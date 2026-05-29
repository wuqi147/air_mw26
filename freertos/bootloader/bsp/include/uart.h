void air_uart_set_isr_enable(unsigned char enable);
void serial_outc(char c);
char serial_inc(void);
int serial_tstc(void);
void air_uart_isr (void);
void uart_init(void);
int outbyte(int c);
