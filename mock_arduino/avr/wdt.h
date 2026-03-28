#ifndef WDT_H
#define WDT_H

#include <stdint.h>

#define WDTO_2S 2000

inline void wdt_enable(int timeout) {}
inline void wdt_reset() {}

#endif
