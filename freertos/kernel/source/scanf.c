#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>

enum parse {
    parse_int = 0,
    parse_ptr            /* Special value used for pointers */
};

enum errno {
    e_ok = 0,           /* No error condition */
    e_eof,              /* Hit EOF */
    e_match             /* Conversion mismatch */
};

enum state {
    state_normal,       /* Ground state */
    state_modify        /* Length or conversion modifiers */
};

#define UMAX            (0xffffffffUL)

static inline int to_int(int ch)
{
    if (ch >= '0' && ch <= '9')
        return ch - '0';
    if (ch >= 'A' && ch <= 'Z')
        return ch - 'A' + 10;
    if (ch >= 'a' && ch <= 'z')
        return ch - 'a' + 10;

    return -1;
}

static inline unsigned int parse_num(const char *ptr_start, char **ptr_end, int base, size_t n)
{
    int minus = 0, digit;
    unsigned int val = 0;

    while (n && isspace((unsigned char)*ptr_start)) {
        ptr_start++;
        n--;
    }

    if (n) {
        char c = *ptr_start;
        if (c == '-' || c == '+') {
            minus = (c == '-');
            ptr_start++;
            n--;
        }
    }

    if (base == 0) {
        if (n >= 2 && ptr_start[0] == '0' &&
            (ptr_start[1] == 'x' || ptr_start[1] == 'X')) {
            n -= 2;
            ptr_start += 2;
            base = 16;
        } else {
            base = 10;
        }
    } else if (base == 16) {
        if (n >= 2 && ptr_start[0] == '0' &&
            (ptr_start[1] == 'x' || ptr_start[1] == 'X')) {
            n -= 2;
            ptr_start += 2;
        }
    }

    while (n && (digit = to_int(*ptr_start)) >= 0 && digit < base) {
        val = val * base + digit;
        n--;
        ptr_start++;
    }

    if (ptr_end)
        *ptr_end = (char *)ptr_start;

    return minus ? -val : val;
}

static inline const char *skip_space(const char *p)
{
    while (isspace((unsigned char)*p))
        p++;
    return p;
}

/*******************************************
 * function: vsscanf
 * support:
 * %d: decimal
 * %x %X: hex
 * %s: string
 * %u: unsigned number
 * %p %P: address
 *******************************************/
int vsscanf(const char *buffer, const char *format, va_list ap) {
    enum state state = state_normal;
    enum errno errno = e_ok;
    enum parse  rank = parse_int;

    const char *p = format;     /* pointer point to format */
    const char *q = buffer;     /* pointer point to buffer */
    const char *qq;             /* used in parse integer, end pointer */

    char ch;                    /* current character in format */
    char *sarg = NULL;          /* %s string argument */

    int base;                   /* value base, may be decimal or hex */
    int count = 0;              /* Successful conversions */

    unsigned char uc;           /* character for parse string */
    unsigned int val = 0;       /* the value we parse from string */
    unsigned int n = UMAX;

    while ((ch = *p++) && !errno) {
        if(state == state_normal) {
            if (ch == '%') {
                state = state_modify;
                rank = parse_int;
                n = UMAX;
            } else if (isspace((unsigned char)ch)) {
                q = skip_space(q);
            } else {
                if (*q == ch)
                    q++;
                else
                    errno = e_match; /* Match failure */
            }
        } else if(state == state_modify) {
            state = state_normal;

            switch (ch) {
            case 'P':    /* pointer */
            case 'p':
                rank = parse_ptr;
                base = 0;
                goto parse;

            case 'd':    /* decimal integer */
            case 'u':    /* unsigned decimal integer */
                base = 10;
                goto parse;

            case 'x':    /* hexadecimal integer */
            case 'X':
                base = 16;

            parse:
                q = skip_space(q);
                if (!*q) {
                    errno = e_eof;
                    break;
                }
                val = parse_num(q, (char **)&qq, base, n);
                if (qq == q) {
                    errno = e_match;
                    break;
                }
                q = qq;
                count++;

                switch (rank) {
                case parse_int:
                    *va_arg(ap, unsigned int *) = val;
                    break;
                case parse_ptr:
                    *va_arg(ap, void **) = (void *)(unsigned int)val;
                    break;
                }
                break;

            case 's':
                uc = 1;
                char *sp;
                sp = sarg = va_arg(ap, char *);
                while (n-- && (uc = *q) && !isspace(uc)) {
                    *sp++ = uc;
                    q++;
                }
                if (sarg != sp) {
                    *sp = '\0';
                    count++;
                }
                if (!uc)
                    errno = e_eof;
                break;

            default:
                errno = e_match;
                break;
            }
        }
    }

    if (errno == e_eof && !count)
        count = -1;

    return count;
}

int sscanf(const char *str, const char *format, ...)
{
    va_list ap;
    int rv;

    va_start(ap, format);
    rv = vsscanf(str, format, ap);
    va_end(ap);

    return rv;
}