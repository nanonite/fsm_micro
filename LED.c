#include "SoR_Utils.h"




 
 
//Front and Back LEDS
void Lite(unsigned int val) { 
	if (val)
		PORT_ON(PORTD,5);
	else
		PORT_OFF(PORTD,5);
}

void toggle_Lite(void) {
	FLIP_PORT(PORTD,5);
}

void hold_led(unsigned int val) {
	if (val)
		PORT_ON(PORTC,2);
	else
		PORT_OFF(PORTC,2);
}

void toggle_hold_led(void) {
	FLIP_PORT(PORTC,2);	
}	

void pin_led(unsigned int val) {
	if (val)
		PORT_ON(PORTC,3);
	else
		PORT_OFF(PORTC,3);
}

void toggle_pin_led(void){
	FLIP_PORT(PORTC,3);
}

void repair_led(unsigned int val) {
	if (val)
		PORT_ON(PORTC,7);
	else
		PORT_OFF(PORTC,7);
}

void toggle_repair_led(void){
	FLIP_PORT(PORTC,7);
}

void unlocked_led(unsigned int val) {
	if (val)
		PORT_ON(PORTC,4);
	else
		PORT_OFF(PORTC,4);
}

void toggle_unlocked_led(void){
	FLIP_PORT(PORTC,4);	
}

void locked_led(unsigned int val) {
	if (val)
	PORT_ON(PORTC,5);
	else
	PORT_OFF(PORTC,5);
}

void toggle_locked_led(void){
	FLIP_PORT(PORTC,5);
}

void noconn_led(unsigned int val) {
	if (val)
	PORT_ON(PORTC,6);
	else
	PORT_OFF(PORTC,6);
}

void toggle_noconn_led(void){
	FLIP_PORT(PORTC,6);
}

void flash_locked_led(void) {
	unlocked_led(0);	// turn off unlocked led
	locked_led(1);	// Turn on Locked led
	for (unsigned int w=0;w<=5;w++) {	// Flash Locked led
		FLIP_PORT(PORTC,5);	 delay_ms(150);
	}//end for
	locked_led(1);	// Turn on Locked led , make sure its on	
}	

	


void ledInit(void) {
	locked_led(1);		// turn on Locked led
	unlocked_led(0);	// turn off unlocked
	hold_led(1);    // hold on
	repair_led(1); // repair on
	pin_led(0); // pin off
	noconn_led(1); // no conn off
	Lite(0); // front and back led lighting off
}

void turnOffLEDS(void) {
	locked_led(0);		// turn off Locked led
	unlocked_led(0);	// turn off unlocked
	hold_led(0);    // hold off
	repair_led(0); // repair off
	pin_led(0); // pin off
	noconn_led(0); // no conn off
	Lite(0); // front and back led lighting off
	
}
	
	
