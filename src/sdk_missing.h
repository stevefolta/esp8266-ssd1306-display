#ifndef sdk_missing_h
#define sdk_missing_h

// Espressif's SDK doesn't declare a lot of functions.  C doesn't really care,
// but C++ does.

extern int os_printf_plus(const char * format, ...);

extern void ets_timer_arm_new(ETSTimer* timer, uint32_t milliseconds, bool repeat_flag, int us);
extern void ets_timer_disarm(ETSTimer* timer);
extern void ets_timer_setfn(ETSTimer* timer, ETSTimerFunc* fn, void* arg);

extern void* pvPortZalloc(int size, const char* file, const char* line);
extern void* pvPortMalloc(int size, const char* file, const char* line);
extern void* pvPortRealloc(void* ptr, size_t size, const char* file, const char* line);
extern void vPortFree(void* ptr, const char* file, const char* line);
extern void ets_memset(void* ptr, int c, int size);


#endif


