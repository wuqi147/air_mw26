#ifndef ANSI_H
#define ANSI_H

#define CSI                 "\e["


#define ANSI_COLOR_RESET            CSI "0"     "m"
#define ANSI_COLOR_REVERSE          CSI "7"     "m"


#define ANSI_CUU(i)                 CSI "%d"    "A",    (i)
#define ANSI_CUD(i)                 CSI "%d"    "B",    (i)
#define ANSI_CUF(i)                 CSI "%d"    "C",    (i)
#define ANSI_CUB(i)                 CSI "%d"    "D",    (i)
#define ANSI_CNL(i)                 CSI "%d"    "E",    (i)
#define ANSI_CPL(i)                 CSI "%d"    "F",    (i)
#define ANSI_CHA(i)                 CSI "%d"    "G",    (i)
#define ANSI_CUP(x, y)              CSI "%d;%d" "H",    (x),(y)
#define ANSI_ED_ALL                 CSI "2"     "J"
#define ANSI_EL_ALL                 CSI "2"     "K"
#define ANSI_EL_FROM_CU_TO_END      CSI "0"     "K"

#define ANSI_DECTCEM_ENABLE         CSI "?25"   "h"
#define ANSI_DECTCEM_DISABLE        CSI "?25"   "l"

#endif

