extern "C" {
#include "ets_sys.h"
#include "sdk_missing.h"
#include "mem.h"
}

void* operator new(size_t size)
{
	return os_malloc(size);
}


void operator delete(void* ptr)
{
	os_free(ptr);
}


// These aren't needed, once I turned on "-fno-exceptions":
/*
extern "C"
void abort(void)
{
	while (true) {}
}


extern "C"
void* malloc(size_t size)
{
	return os_malloc(size);
}

extern "C"
void free(void* ptr)
{
	os_free(ptr);
}

extern "C"
void* realloc(void* ptr, size_t size)
{
	return os_realloc(ptr, size);
}
*/



