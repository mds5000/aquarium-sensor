#ifndef DEBUG_H
#define DEBUG_H

#ifdef RELEASE
    #define debugf(...) {}
#else

    extern "C" {
        #include "SEGGER_RTT.h"
    }

    #define debug(fmt, ...) SEGGER_RTT_printf(0, fmt "\r\n", ##__VA_ARGS__)
    #define trace(fmt, ...) SEGGER_RTT_printf(0, "%s[%d]: " fmt "\r\n", __FILE__, __LINE__, ##__VA_ARGS__)

#endif /* RELEASE */

#endif /* DEBUG_H */