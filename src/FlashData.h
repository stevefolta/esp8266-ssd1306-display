#ifndef FlashData_h
#define FlashData_h

extern "C" {
#include "ets_sys.h"
}


inline uint8_t pgm_read_byte(const char* ptr)
{
	unsigned long offset = ((unsigned long) ptr) & 0x03;
	const unsigned long* addr = (unsigned long*) (ptr - offset);
	return *addr >> (offset << 3);
}



#endif	// !FlashData_h

