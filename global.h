
#ifndef GLOBAL_H
#define GLOBAL_H

// global AVRLIB defines
#include "avrlibdefs.h"
// global AVRLIB types definitions
#include "avrlibtypes.h"
#include <inttypes.h> 

#ifdef __cplusplus

extern "C" {
	
#endif

// back-door way to enable floating-point print support
#define RPRINTF_FLOAT


// CPU clock speed
//#define F_CPU			16000000               		// 16MHz processor
//#define F_CPU			8000000               		// 8MHz processor // 8.5 cycles per microsecond
#define F_CPU			16000000               		// 16MHz processor
// this is 8.5 cycles per microseconds
#define CYCLES_PER_US ((F_CPU+500000)/1000000) 	// cpu cycles per microsecond
//#define RPRINTF_COMPLEX

#ifdef __cplusplus

}
	
#endif

#endif
