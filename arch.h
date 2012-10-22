#ifndef ARCH_H_
#define ARCH_H_

#ifdef __i386__ /* Linux */

#include <stdint.h>

#define NEED_POSITION_DECOMPRESSION

#else /* AVR microcontroller */

#include "avrlibtypes.h"

#ifdef __cplusplus

extern "C" {
	
#endif


/*

#ifdef
#undef NEED_POSITION_DECOMPRESSION
#endif
typedef u08 uint8_t;
typedef u16 uint16_t;
typedef u32 uint32_t;

*/

#ifdef __cplusplus

	}
	
#endif
#endif /* AVR microcontroller */

#endif /* ARCH_H_ */
