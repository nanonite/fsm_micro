#include "SoR_Utils.h"



#define ERROR	0
#define SUCCESS 1




void mcycles(volatile unsigned long int cycles)
{
	//cycles=cycles;//doubled frequency but too lazy to change times
	//WHYNOT: while (cycles--);
	while(cycles > 0)
	cycles--;
}

void i2c_delay(void) {
	mcycles(100*CYCLES_PER_US/30); // 100 uS
}

/*
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
*/

void TWIInit(void)
{
	//set SCL to 400kHz
	TWSR = 0x00;
	TWBR = 0x0C;
	//enable TWI
	TWCR = (1<<TWEN); //not interrupt based
	
	//init state
	sbi(PORTC, 0);	// i2c SCL on 1284P
	sbi(PORTC, 1);	// i2c SDA on 1284P
	
}

void TWIWrite(uint8_t u8data)
{
	TWDR = u8data;
	TWCR = (1<<TWINT)|(1<<TWEN);
	while ((TWCR & (1<<TWINT)) == 0);
}

void TWIStart(void)
{
	TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
	while ((TWCR & (1<<TWINT)) == 0);
}
//send stop signal
void TWIStop(void)
{
	TWCR = (1<<TWINT)|(1<<TWSTO)|(1<<TWEN);
}

uint8_t TWIReadACK(void)
{
	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA);
	while ((TWCR & (1<<TWINT)) == 0);
	return TWDR;
}

//read byte with NACK
uint8_t TWIReadNACK(void)
{
	TWCR = (1<<TWINT)|(1<<TWEN);
	while ((TWCR & (1<<TWINT)) == 0);
	return TWDR;
}

uint8_t TWIGetStatus(void)
{
	uint8_t status;
	//mask status
	status = TWSR & 0xF8;
	return status;

}


uint8_t TWISend(uint8_t deviceAddr, uint8_t sub_addr, uint8_t u8data) {
	TWIStart();
	if (TWIGetStatus() != 0x08)
	return ERROR;
	
	TWIWrite(deviceAddr & 0xFE );
	if (TWIGetStatus() != 0x18)
	return ERROR;
	
	TWIWrite(sub_addr);
	if (TWIGetStatus() != 0x28)
	return ERROR;
	
	TWIWrite(u8data);
	//	if (TWIGetStatus() != 0x28)
	//	return ERROR;
	
	TWIStop();
	i2c_delay();
	return SUCCESS;
	
}

uint8_t TWIGet(uint8_t deviceAddr, uint8_t sub_addr, uint8_t *u8data)
{
	//uint8_t databyte;
	TWIStart();
	if (TWIGetStatus() != 0x08)
	return ERROR;
	TWIWrite(deviceAddr & 0xFE );
	if (TWIGetStatus() != 0x18)
	return ERROR;
	
	TWIWrite(sub_addr);
	if (TWIGetStatus() != 0x28)
	return ERROR;
	
	//send repeated start
	TWIStart();
	if (TWIGetStatus() != 0x10)
	return ERROR;
	
	//select device and send read bit
	TWIWrite( deviceAddr | 0x01  );
	if (TWIGetStatus() != 0x40)
	return ERROR;
	
	*u8data = TWIReadNACK();
	if (TWIGetStatus() != 0x58)
	return ERROR;
	
	TWIStop();
	
	i2c_delay();
	return SUCCESS;
}

uint8_t TWIMultiGet(unsigned char deviceAddr, unsigned char sub_addr, signed char length, unsigned char *u8data){
	//uint8_t databyte;
	TWIStart();
	if (TWIGetStatus() != 0x08)
	return ERROR;
	TWIWrite(deviceAddr & 0xFE );
	if (TWIGetStatus() != 0x18)
	return ERROR;
	
	TWIWrite(sub_addr);
	if (TWIGetStatus() != 0x28)
	return ERROR;
	
	//send repeated start
	TWIStart();
	if (TWIGetStatus() != 0x10)
	return ERROR;
	
	//select devise and send read bit
	TWIWrite( deviceAddr | 0x01  );
	if (TWIGetStatus() != 0x40)
	return ERROR;
	
	while(length >= 1) {
		*u8data++ = TWIReadNACK();
		if (TWIGetStatus() != 0x58)
		return ERROR;
		// decrement length
		length--;
	}

	TWIStop();
	return SUCCESS;

}