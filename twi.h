#ifndef TWI_H
#define TWI_H

#ifdef __cplusplus

	extern "C" {
		
#endif

void mcycles(volatile unsigned long int cycles);

void i2c_delay(void);

void TWIInit(void);
void TWIWrite(uint8_t u8data);
void TWIStart(void);
void TWIStop(void);
uint8_t TWIReadACK(void);
uint8_t TWIReadNACK(void);
uint8_t TWIGetStatus(void);
uint8_t TWIGet(uint8_t deviceAddr, uint8_t sub_addr, uint8_t *u8data);
uint8_t TWIMultiGet(unsigned char deviceAddr, unsigned char sub_addr, signed char length, unsigned char *u8data);
uint8_t TWISend(uint8_t deviceAddr, uint8_t sub_addr, uint8_t u8data);



#ifdef __cplusplus

	}	
	
#endif


#endif