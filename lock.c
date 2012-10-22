// pA3, PA2 is In1 ,In2
//pa1 is midlock	<---PCINT1
//pa0 is outerlock  <---PCINT0


#ifndef SOR_UTILS_H
#include "SoR_Utils.h"
#endif


		 
ISR(PCINT0_vect)  // PA0 = OUT_LOCK or Sensor Locks
{	
	//compare before and after and take in account denounce logic
	//sense_state = SenseIsTrue;
	//block_state = BlockIsTrue;
	
	

}	//end ISR

/*
ISR(PCINT1_vect) // PA1 = MID_LOCK or Lock Block
{
	sense_state = SenseIsTrue;
	block_state = BlockIsTrue;
	
	
}
*/

void LockInit(void) {
	 PCMSK0 |= (1 << PCINT0) |  (1 << PCINT1); // motor pins
	 
	 PCICR |= (1 << PCIE0); // enable all pin interrupts on the PCMSK3 register
	
	 //config ports done in SoR_Utils.h
	 
	// sense_state = SenseIsTrue; //Returns true if outer locks are not pushed in
	 //block_state = BlockIsTrue; //Returns true if middle lock is not pushed in
}	


void Actuate(char dir) {
	if (dir == 2) { // Pull
		PORT_ON(PORTA,3);//IN1 output driven high
		PORT_OFF(PORTA,2);//IN2 output driven low
	}			
	else if (dir == 1) { // Push
		PORT_OFF(PORTA,3);//IN1 output driven low
		PORT_ON(PORTA,2);//IN2 output driven high
	}
	else { // Off
		PORT_OFF(PORTA,3);//IN1 output driven low
		PORT_OFF(PORTA,2);//IN2 output driven low
	}				
	
}	



void Engage_Lock(void) {
	PORT_ON(PORTA,3); //IN1 output driven high 
	PORT_OFF(PORTA,2); //IN2 output driven low
}
void Disengage_Lock(void) {
	PORT_OFF(PORTA,3); //IN1 output driven low
	PORT_ON(PORTA,2); //IN2 output driven high
}		
	
/*
	if ( SenseIsTrue != sense_state ) {	 // if read value is not the same as the current state
	sense_state_timer = 1;	// # of seconds it has to stay low
}
else {	// if its the same old state
sense_state_timer=0;
	}

	if ( BlockIsTrue != block_state ) {	 // if read value is not the same as the current state
		block_state_timer = 1;	// # of seconds it has to stay low
	}
	else {	// if its the same old state
		block_state_timer=0;
	}
*/