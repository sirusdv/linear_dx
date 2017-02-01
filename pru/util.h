
#ifndef __UTIL_H_
#define __UTIL_H_

#define LOG(a) debug_log a

void debug_log(const char* fmt, ...)
        __attribute__((format(printf, 1, 2)));


#endif
