#ifndef KEYPAD_H
#define KEYPAD_H

#ifdef __cplusplus

extern "C" {
	
#endif

#define KEYPAD_ADDR				0x68
#define KEY_REG					0x04
#define KEY_INT_REG				0x02

#define REPAIR_KEY				162
#define HOLD_KEY				132
#define ENTER_KEY				161
#define CANCEL_KEY				159

#define CHECK_PIN			220
#define TOO_SHORT			221
#define BAD_ENTER			222

void Enable_KeypadTimeout(void) ;

void KeypadInit(void) ;

unsigned char Key2Dec(unsigned char raw) ;

unsigned char IsCorrectCode(void);

void Clear_KeypadInterrupt(void) ;

unsigned char IsActive_KeypadInterrupt(void) ;


void ReadKeypad(void);

void ReadLongKeypad(void);

#ifdef __cplusplus

}
	
#endif

#endif