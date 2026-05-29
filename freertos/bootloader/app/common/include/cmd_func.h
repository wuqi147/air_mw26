#define E_SYNTAX_OK            (0)
#define E_SYNTAX_ERROR         (1)

int isDmemAddr(unsigned int addr);
int isRegAddr(unsigned int addr);
int write_flash_data(const char *input);
int read_flash_data(const char *input);
