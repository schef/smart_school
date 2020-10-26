#ifndef __LOG_HPP__
#define __LOG_HPP__

#include "mbed.h"
#include "stdarg.h"

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define logi(format, ...) LogPrintf("[%s] " format , __FILENAME__, ##__VA_ARGS__)
#define logif(format, ...) LogPrintf("[%s:%s] " format , __FILENAME__, __FUNCTION__, ##__VA_ARGS__)
#define logifl(format, ...) LogPrintf("[%s:%s:%d] " format , __FILENAME__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define logf(format, ...) LogPrintf(format, ##__VA_ARGS__)
#define logb(data, len, separator) LogPrintBytes(data, len, separator, false)
#define logbln(data, len, separator) LogPrintBytes(data, len, separator, true)

static inline void LogPrintf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
}

static inline void LogPrintBytes(const void *object, size_t size, char separator, bool newLine) {
    const unsigned char *const bytes = (const unsigned char *const) object;
    size_t i;

    LogPrintf("[");
    for (i = 0; i < size; i++) {
        LogPrintf("%02X", bytes[i]);
        if (i < size - 1) {
            LogPrintf("%c", separator);
        }
    }
    LogPrintf("]");

    if (newLine) {
        LogPrintf("\n");
    }
}

#endif