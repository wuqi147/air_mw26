#ifndef INTERNAL_FEATURE_H
#define INTERNAL_FEATURE_H

#ifndef AIR_SUPPORT_CMD
    #define DPRINTF(...)
#else
    #define DPRINTF printf
#endif

#define DBG DPRINTF

#endif // INTERNAL_FEATURE_H
