/*******************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of Airoha Technology Corp. (C) 2021
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AIROHA SOFTWARE")
*  RECEIVED FROM AIROHA AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. AIROHA EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES AIROHA PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE AIROHA SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. AIROHA SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY AIROHA SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND AIROHA'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE AIROHA SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT AIROHA'S OPTION, TO REVISE OR REPLACE THE AIROHA SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  AIROHA FOR SUCH AIROHA SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*******************************************************************************/

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <termios.h>
#include <signal.h>

#include <sdk_ref.h>
#include <parser/dsh_parser.h>

/* ----------------------------------------------------------------- macro */
#define SDK_REF_HISTORY_MAX_ENTRY   (10)
#define SDK_REF_HISTORY_MAX_LENGTH  (DSH_CMD_MAX_INPUT)

/* ----------------------------------------------------------------- history buf */
/* history buf */
typedef struct
{
    char                        buf[SDK_REF_HISTORY_MAX_ENTRY][SDK_REF_HISTORY_MAX_LENGTH];
    int                         wr_pos; /* write ring index */
    int                         rd_pos; /* read offset      */
    int                         cnt;    /* write counter    */

} SDK_REF_HISTORY_T;

/* ----------------------------------------------------------------- key parser */
/* return code */
typedef enum
{
    SDK_REF_KEY_RET_CONT = 0,   /* get next ch      */
    SDK_REF_KEY_RET_BREAK,      /* finish and break */
    SDK_REF_KEY_RET_NONE,       /* normal append ch */
    SDK_REF_KEY_RET_LAST
} SDK_REF_KEY_RET_T;

/* callback */
typedef void
(*SDK_REF_KEY_FUNC_T)(
    SDK_REF_HISTORY_T           *ptr_history,
    char                        *ptr_buf,
    int                         *ptr_buf_idx);

/* array of keycode */
typedef struct
{
    int                         num;
    char                        code[4];
    char                        *ptr_name;
    SDK_REF_KEY_FUNC_T          func;
    int                         ret;

} SDK_REF_KEY_VEC_T;

/* node for each character */
typedef struct SDK_REF_KEY_NODE_S
{
    char                        code;
    struct SDK_REF_KEY_NODE_S   *ptr_child;
    struct SDK_REF_KEY_NODE_S   *ptr_sibling;
    char                        *ptr_name;
    SDK_REF_KEY_FUNC_T          func;
    int                         ret;

} SDK_REF_KEY_NODE_T;

/* control block */
typedef struct
{
    /* history buf */
    SDK_REF_HISTORY_T           history;

    /* key parser tree */
    SDK_REF_KEY_NODE_T          *ptr_head;

    /* control terminal attributes */
    struct termios              termios;

} SDK_REF_PARSER_CB_T;

/* ----------------------------------------------------------------- history buf */
static void
_sdk_ref_displayBuf(
    SDK_REF_HISTORY_T   *ptr_history,
    char                *ptr_buf,
    int                 *ptr_buf_idx)
{
    int                 wr_pos = ptr_history->wr_pos;
    int                 rd_pos = ptr_history->rd_pos;
    int                 cnt    = ptr_history->cnt;

    int                 buf_idx = *ptr_buf_idx;
    int                 pos = 0;
    int                 idx = 0;

    /* get the relative position */
    if (wr_pos < rd_pos)
    {
        pos = wr_pos + cnt - rd_pos;
    }
    else
    {
        pos = wr_pos - rd_pos;
    }

    /* clean the command on the screen */
    for (idx = buf_idx; idx < strlen(ptr_buf); idx++)
    {
        printf(" ");
    }
    for (idx = 0; idx < strlen(ptr_buf); idx++)
    {
        printf("\b \b");
    }

    /* save the command to input buffer, and shift the buf index */
    memset(ptr_buf, 0x0, SDK_REF_HISTORY_MAX_LENGTH);
    if (rd_pos > 0)
    {
        if (strlen(ptr_history->buf[pos]) > 0)
        {
            memcpy(ptr_buf, ptr_history->buf[pos], strlen(ptr_history->buf[pos]));
        }

        buf_idx = strlen(ptr_history->buf[pos]);
    }
    else
    {
        buf_idx = 0;
    }

    /* print out */
    printf("%s", ptr_buf);

    /* update */
    *ptr_buf_idx = buf_idx;
}

/* ----------------------------------------------------------------- key parser */
static void
_sdk_ref_tab(
    SDK_REF_HISTORY_T   *ptr_history,
    char                *ptr_buf,
    int                 *ptr_buf_idx)
{
    /* do nothing for tab */
}

static void
_sdk_ref_cr(
    SDK_REF_HISTORY_T   *ptr_history,
    char                *ptr_buf,
    int                 *ptr_buf_idx)
{
    if (strlen(ptr_buf) > 0)
    {
        memset(ptr_history->buf[ptr_history->wr_pos], 0x0, SDK_REF_HISTORY_MAX_LENGTH);
        memcpy(ptr_history->buf[ptr_history->wr_pos], ptr_buf, strlen(ptr_buf));

        /* update wr */
        ptr_history->wr_pos++;
        ptr_history->wr_pos %= SDK_REF_HISTORY_MAX_ENTRY;

        /* update cnt */
        if (ptr_history->cnt < SDK_REF_HISTORY_MAX_ENTRY)
        {
            ptr_history->cnt++;
        }
    }

    /* update rd */
    ptr_history->rd_pos = 0;

    /* this character should be output */
    printf("\n");
}

static void
_sdk_ref_up(
    SDK_REF_HISTORY_T   *ptr_history,
    char                *ptr_buf,
    int                 *ptr_buf_idx)
{
    if (ptr_history->rd_pos < ptr_history->cnt)
    {
        ptr_history->rd_pos++;
    }

    _sdk_ref_displayBuf(ptr_history, ptr_buf, ptr_buf_idx);
}

static void
_sdk_ref_down(
    SDK_REF_HISTORY_T   *ptr_history,
    char                *ptr_buf,
    int                 *ptr_buf_idx)
{
    if (ptr_history->rd_pos > 0)
    {
        ptr_history->rd_pos--;
    }

    _sdk_ref_displayBuf(ptr_history, ptr_buf, ptr_buf_idx);
}

static void
_sdk_ref_right(
    SDK_REF_HISTORY_T   *ptr_history,
    char                *ptr_buf,
    int                 *ptr_buf_idx)
{
    if (*ptr_buf_idx < strlen(ptr_buf))
    {
        printf("%c", ptr_buf[*ptr_buf_idx]);
        (*ptr_buf_idx)++;
    }
}

static void
_sdk_ref_left(
    SDK_REF_HISTORY_T   *ptr_history,
    char                *ptr_buf,
    int                 *ptr_buf_idx)
{
    if (*ptr_buf_idx > 0)
    {
        printf("\b");
        (*ptr_buf_idx)--;
    }
}

static void
_sdk_ref_end(
    SDK_REF_HISTORY_T   *ptr_history,
    char                *ptr_buf,
    int                 *ptr_buf_idx)
{
    int                 idx = 0;
    int                 cursor = 0;

    /* move the cursor */
    cursor = strlen(ptr_buf) - (*ptr_buf_idx);
    for (idx = 0; idx < cursor; idx++)
    {
        _sdk_ref_right(ptr_history, ptr_buf, ptr_buf_idx);
    }
}

static void
_sdk_ref_home(
    SDK_REF_HISTORY_T   *ptr_history,
    char                *ptr_buf,
    int                 *ptr_buf_idx)
{
    int                 idx = 0;
    int                 cursor = 0;

    /* move the cursor */
    cursor = (*ptr_buf_idx);
    for (idx = 0; idx < cursor; idx++)
    {
        _sdk_ref_left(ptr_history, ptr_buf, ptr_buf_idx);
    }
}

static void
_sdk_ref_del(
    SDK_REF_HISTORY_T   *ptr_history,
    char                *ptr_buf,
    int                 *ptr_buf_idx)
{
    int                 buf_idx = *ptr_buf_idx;
    int                 buf_last = strlen(ptr_buf);
    int                 idx = 0;
    int                 cursor = 0;

    if (buf_idx < buf_last)
    {
        /* move ch forward */
        for (idx = buf_idx + 1; idx < buf_last; idx++)
        {
            ptr_buf[idx - 1] = ptr_buf[idx];
        }
        ptr_buf[idx - 1] = '\0';

        /* print the buffer on the screen */
        buf_last--;
        printf("%s", &ptr_buf[buf_idx]);
        printf(" ");

        /* move the cursor */
        cursor = buf_last - buf_idx + 1;
        for (idx = 0; idx < cursor; idx++)
        {
            printf("\b");
        }

        /* update */
        *ptr_buf_idx = buf_idx;
    }
}

static void
_sdk_ref_back(
    SDK_REF_HISTORY_T   *ptr_history,
    char                *ptr_buf,
    int                 *ptr_buf_idx)
{
    int                 buf_idx = *ptr_buf_idx;
    int                 buf_last = strlen(ptr_buf);
    int                 idx = 0;
    int                 cursor = 0;

    if (buf_idx > 0)
    {
        /* move ch forward */
        for (idx = buf_idx; idx < buf_last; idx++)
        {
            ptr_buf[idx - 1] = ptr_buf[idx];
        }
        ptr_buf[idx - 1] = '\0';
        printf("\b");

        /* print the buffer on the screen */
        buf_idx--;
        buf_last--;
        printf("%s", &ptr_buf[buf_idx]);
        printf(" ");

        /* move the cursor */
        cursor = buf_last - buf_idx + 1;
        for (idx = 0; idx < cursor; idx++)
        {
            printf("\b");
        }

        /* update */
        *ptr_buf_idx = buf_idx;
    }
}

static void
_sdk_ref_normal(
    char                *ptr_buf,
    int                 *ptr_buf_idx,
    char                ch)
{
    int                 buf_idx = *ptr_buf_idx;
    int                 buf_last = strlen(ptr_buf);
    int                 idx = 0;
    int                 cursor = 0;

    if (buf_last < SDK_REF_HISTORY_MAX_LENGTH)
    {
        /* move ch backward */
        for (idx = buf_last; idx > buf_idx; idx--)
        {
            ptr_buf[idx] = ptr_buf[idx - 1];
        }

        /* print the buffer on the screen */
        ptr_buf[buf_idx] = ch;
        buf_idx++;
        buf_last++;
        printf("%s", &ptr_buf[buf_idx - 1]);

        /* move the cursor */
        cursor = buf_last - buf_idx;
        for (idx = 0; idx < cursor; idx++)
        {
            printf("\b");
        }

        /* update */
        *ptr_buf_idx = buf_idx;
    }
}

static char
_sdk_ref_getCh()
{
    char ch = '\0';

    if (1 == read(STDIN_FILENO, &ch, 1))
    {
        return ch;
    }

    /* there was some problem */
    return 0;
}

static int
_sdk_ref_searchNode(
    SDK_REF_HISTORY_T   *ptr_history,
    SDK_REF_KEY_NODE_T  *ptr_node,
    char                *ptr_buf,
    int                 *ptr_buf_idx,
    char                ch)
{
    if (NULL == ptr_node)
    {
        /* may be normal or undefined */
        return SDK_REF_KEY_RET_NONE;
    }

    if (ptr_node->code == ch)
    {
        if (NULL != ptr_node->func)
        {
            /* fully-match */
            ptr_node->func(ptr_history, ptr_buf, ptr_buf_idx);
            return ptr_node->ret;
        }

        /* partially-match, see if there was a child  */
        ch = _sdk_ref_getCh();
        return _sdk_ref_searchNode(ptr_history, ptr_node->ptr_child, ptr_buf, ptr_buf_idx, ch);
    }
    else
    {
        /* miss, see if there was a sibling */
        return _sdk_ref_searchNode(ptr_history, ptr_node->ptr_sibling, ptr_buf, ptr_buf_idx, ch);
    }
}

static void
_sdk_ref_popNode(
    SDK_REF_KEY_NODE_T  *ptr_node)
{
    if (NULL == ptr_node)
    {
        return;
    }

    /* traverse the childs and siblings */
    _sdk_ref_popNode(ptr_node->ptr_child);
    _sdk_ref_popNode(ptr_node->ptr_sibling);

    /* delete node */
    free(ptr_node);
}

static SDK_REF_KEY_NODE_T *
_sdk_ref_pushNode(
    SDK_REF_KEY_NODE_T  *ptr_node,
    SDK_REF_KEY_VEC_T   *ptr_vec,
    int                 depth)
{
    if (NULL == ptr_node)
    {
        /* append node */
        ptr_node = malloc(sizeof(SDK_REF_KEY_NODE_T));
        memset(ptr_node, 0x0, sizeof(SDK_REF_KEY_NODE_T));
        ptr_node->code = ptr_vec->code[depth];

        if (depth == (ptr_vec->num - 1))
        {
            /* fully-match, hook func */
            ptr_node->ptr_name = ptr_vec->ptr_name;
            ptr_node->func     = ptr_vec->func;
            ptr_node->ret      = ptr_vec->ret;
        }
        else
        {
            /* partially-match, append child */
            ptr_node->ptr_child =
                _sdk_ref_pushNode(ptr_node->ptr_child, ptr_vec, depth + 1);
        }
        return ptr_node;
    }

    /* traverse the childs and siblings */
    if (ptr_node->code == ptr_vec->code[depth])
    {
        ptr_node->ptr_child =
            _sdk_ref_pushNode(ptr_node->ptr_child, ptr_vec, depth + 1);
    }
    else
    {
        ptr_node->ptr_sibling =
            _sdk_ref_pushNode(ptr_node->ptr_sibling, ptr_vec, depth);
    }
    return ptr_node;
}

/* ----------------------------------------------------------------- static data */
static SDK_REF_PARSER_CB_T  _sdk_ref_parser_cb;

/* e.g.
 * 0x0A ->0x1B ->0x7F
 * F      |      F
 *        ->0x5B
 *          |
 *          ->0x31 ->0x33 ->0x34 ->0x41 ->0x42 ->0x43 ->0x44
 *            |      |      |      F      F      F      F
 *            ->0x7E ->0x7E ->0x7E
 *              F      F      F
 */
static SDK_REF_KEY_VEC_T    _sdk_ref_key_vec[] =
{
    {1, {0x9                    }, "TAB",   _sdk_ref_tab,   SDK_REF_KEY_RET_CONT  },
    {1, {0xA                    }, "CR",    _sdk_ref_cr,    SDK_REF_KEY_RET_BREAK },
    {3, {0x1B, 0x5B, 0x41       }, "Up",    _sdk_ref_up,    SDK_REF_KEY_RET_CONT  },
    {3, {0x1B, 0x5B, 0x42       }, "Down",  _sdk_ref_down,  SDK_REF_KEY_RET_CONT  },
    {3, {0x1B, 0x5B, 0x43       }, "Right", _sdk_ref_right, SDK_REF_KEY_RET_CONT  },
    {3, {0x1B, 0x5B, 0x44       }, "Left",  _sdk_ref_left,  SDK_REF_KEY_RET_CONT  },
    {4, {0x1B, 0x5B, 0x34, 0x7E }, "End",   _sdk_ref_end,   SDK_REF_KEY_RET_CONT  },
    {4, {0x1B, 0x5B, 0x31, 0x7E }, "Home",  _sdk_ref_home,  SDK_REF_KEY_RET_CONT  },
    {1, {0x7F                   }, "Del",   _sdk_ref_del,   SDK_REF_KEY_RET_CONT  },
    {1, {0x8                    }, "Back",  _sdk_ref_back,  SDK_REF_KEY_RET_CONT  },
};

/* ----------------------------------------------------------------- API */
AIR_ERROR_NO_T
sdk_ref_deinitParser(void)
{
    /* delete each keycode */
    _sdk_ref_popNode(_sdk_ref_parser_cb.ptr_head);

    /* recover terminal */
    tcsetattr(STDIN_FILENO, TCSANOW, &_sdk_ref_parser_cb.termios);

    return AIR_E_OK;
}

void
sdk_ref_sigHandler(
    int                 sig)
{
    sdk_ref_deinitParser();
    printf("\n");
    exit(0);
}

AIR_ERROR_NO_T
sdk_ref_initParser(void)
{
    int                 vec = 0;
    int                 vec_size = sizeof(_sdk_ref_key_vec) / sizeof(SDK_REF_KEY_VEC_T);
    SDK_REF_KEY_NODE_T  *ptr_head = NULL;
    struct termios      new_term;
    struct sigaction    action;

    /* reset control block */
    memset(&_sdk_ref_parser_cb, 0x0, sizeof(SDK_REF_PARSER_CB_T));

    /* set terminal */
    tcgetattr(STDIN_FILENO, &_sdk_ref_parser_cb.termios);
    new_term = _sdk_ref_parser_cb.termios;
    new_term.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_term);

    /* register ctrl-C action */
    memset(&action, 0, sizeof(action));
    action.sa_handler = sdk_ref_sigHandler;
    sigaction(SIGINT, &action, NULL);

    /* add each keycode */
    for (vec = 0; vec < vec_size; vec++)
    {
        ptr_head = _sdk_ref_pushNode(ptr_head, &_sdk_ref_key_vec[vec], 0);
    }

    _sdk_ref_parser_cb.ptr_head = ptr_head;
    return AIR_E_OK;
}

AIR_ERROR_NO_T
sdk_ref_getInput(
    char                *ptr_buf)
{
    int                 ret = 0;
    int                 buf_idx = 0;
    char                ch = '\0';

    while (1)
    {
        ch = _sdk_ref_getCh();
        ret = _sdk_ref_searchNode(&_sdk_ref_parser_cb.history, _sdk_ref_parser_cb.ptr_head, ptr_buf, &buf_idx, ch);

        if (SDK_REF_KEY_RET_CONT == ret)
            continue;

        if (SDK_REF_KEY_RET_BREAK == ret)
            break;

        if (SDK_REF_KEY_RET_NONE == ret)
            _sdk_ref_normal(ptr_buf, &buf_idx, ch);
    }

    return AIR_E_OK;
}

