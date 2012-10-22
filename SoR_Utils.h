#ifndef SOR_UTILS_H
#define SOR_UTILS_H
//Standard Includes
#include <avr/io.h>		    // include I/O definitions (port names, pin names, etc)
#include <avr/interrupt.h>	// include interrupt support
#include <stdio.h>			// stuff
#include <stdlib.h>			// stuff
#include <string.h>			// allow strings to be used
//Custom Includes
#include "global.h"		// include global settings
#include "a2d.h"
#include "twi.h"
#include "keypad.h"
#include "mc_message_internal.h"

/*
#include "config.h" // custom configurations, very high level changes
//AVRlib includes

#include "uart2.h"
#include "rprintf.h"	// include rprintf function library
//GSM includes
#include "gsmTelitHardware.h"
*/

#ifdef __cplusplus

extern "C" {
	
#endif


//define port functions; example: PORT_ON( PORTD, 6);
#define PORT_ON( port_letter, number )			port_letter |= (1<<number)
#define PORT_OFF( port_letter, number )			port_letter &= ~(1<<number)
#define FLIP_PORT( port_letter, number )		port_letter ^= (1<<number)
#define PORT_IS_ON( port_letter, number )		( port_letter & (1<<number) )
#define PORT_IS_OFF( port_letter, number )		!( port_letter & (1<<number) )



//#define DebugPrint	uart1SendByte

// Lock Sensor Stuff
#define SenseIsTrue	PORT_IS_OFF(PINA,0)
#define BlockIsTrue	PORT_IS_OFF(PINA,1)

// LED Indicator codes
#define SOLID	0
#define SLOW	120
#define FAST	15
#define VERY_FAST 3


//replace all this , with enum types from mc+message.h

// Lock States
#define LOCKED			1
#define UNLOCKED		3 
#define RELEASED		2 

// Owner States
#define AVAILABLE		1
#define HOLD			2
#define OWNED			3

// Repair States
#define WORKING			0
#define BROKEN			1

//Key Event States
#define WAITING 100
#define ENTER_CONFUSED 105
#define CANCELD 110
#define VALID_INPUT 115
#define HOLD_INPUT 120
#define REPAIR_INPUT 125
// Keypad Length of Storage
#define KEY_INPUT_LEN			4   // 0-4 array, 5 keys, last one being ENTER key, length of Pin Code
#define LONGKEY_INPUT_LEN			6   // 0-5 array, 6 keys, last one being ENTER key, length of Account Number/ Override

// Communication Protocol Definitions
#define ACK			57  // 57 + (EXTRA*5)
#define NACKTRACK	45
#define BAD			40
#define REBOOT		35
#define ACK_DELAY		9000	// 30 sec delay, 40000 is actually 50 secs so 20000 is 25 secs
#define NACKTRACK_DELAY	240000	// 5 min delay, 30 secs * 2 * 5


//************CONFIGURE PORTS************
//

// Accelerometer Variables
unsigned int accel_int_flag; // accelerometer flag

unsigned int gps_issuspended_FLAG;
unsigned int gotDNS_FLAG;

// Keypad Variables
unsigned int key_input[5]; // pin code input
unsigned int key_count; // array counter
unsigned int key_isvalid; // is keypad input valid flag 

unsigned int longkey_input[7]; // pin code input for account or admin
unsigned int longkey_count; // array counter
unsigned int longkey_isvalid; // is keypad input valid flag

// Lock Input State

unsigned int sense_state, block_state;
unsigned int sense_state_counter, block_state_counter;
int temp_state;


// Reservation Packet Defines -  account, pin, reservation_timeout, hold_timeout
unsigned int account[9];
unsigned int pin[4];	// default values are impossible to be inputted
unsigned int reservation_timeout_serverdefined, hold_timeout_serverdefined;
unsigned int reservation_isvalid;

// Time Now Variables
unsigned int t_sec,t_min,t_hour;
// Last_Talk counter
unsigned long last_talk;

// Flags
unsigned int unlock_flag,lock_flag, pin_fail_flag;
unsigned int waitforRemoveLock_FLAG,toSendState_FLAG; //remove when serve comm removed

// State Packet Defines - owner state, lock state, repair state, account, reservation_timeout, batt level, timestamp, lat, long, speed
unsigned int Lock_STATE, Owner_STATE, Repair_STATE;

// Bike ID and Account Number
signed int SIM[20];
unsigned int compressed_SIM[8];
unsigned int compressed_account_num[5]; 
	
// GPS Arrays
char fix_quality;
signed int lat[9];  // 9 chars
signed int lat_heading;
signed int lng[10]; // ten chars
signed int long_heading;
signed int compressed_lat[4];
signed int compressed_lng[5];

// Battery Variables
unsigned int batt_level;

// SMS stuff 
unsigned int index_val;

//MilliSecond Counter
uint32_t milicount; //overflows every 8 years
//Configure uC
void configure_ports(void);


//************DELAY FUNCTIONS************
//wait for X amount of cycles (234 cycles is about 1.003 milliseconds)//incorrect
//to calculate: 234/1.003*(time in milliseconds) = number of cycles
void delay_cycles(volatile unsigned long int cycles);


void delay_us(unsigned long int microseconds);


void delay_ms(unsigned long int milliseconds);

//************TIMER FUNCTIONS************
void EnableTimer(void);
void DisableTimer(void);
void Clear_KeyArray(void);
void Clear_LongKeyArray(void);

void Cancel_Reservation(void);
void GPS_Suspend(unsigned int state);

//************LOCK FUNCTIONS************
void Engage_Lock(void);
void Disengage_Lock(void);
void LockInit(void);
void Actuate(char dir);

//************LED FUNCTIONS************
void Lite(unsigned int val);
void toggle_Lite(void);
void pin_led(unsigned int val);
void hold_led(unsigned int val);
void toggle_hold_led(void);
void unlocked_led(unsigned int val);
void locked_led(unsigned int val);
void toggle_locked_led(void);
void repair_led(unsigned int val);
void toggle_repair_led(void);
void noconn_led(unsigned int val);
void toggle_noconn_led(void);
void ledInit(void);
void turnOffLEDS(void);

//************CONTROL FUNCTIONS************
void control(int wake_Flag);
void talk(void);

//uint32_t getMillis(void);

#ifdef __cplusplus

	}	
	
#endif

#endif