#include "mc_message.h"
#include "SoR_Utils.h"





//case is usually you disable all interrupts as soon as ISR is serviced,
//then and renable them at the end of ISR



void Clear_KeyArray(void) {
	key_isvalid=0;	// clear flag, key input entered is no longer valid
	key_count=0;	// clear count of keys
	for (unsigned int q=0;q<=KEY_INPUT_LEN;q++) { // clear values of all keys entered in ,added in <=
		key_input[q] = 0;
	}

} 

void Clear_LongKeyArray(void) {
	longkey_isvalid=0;	// clear flag, key input entered is no longer valid
	longkey_count=0;	// clear count of keys
	for (unsigned int q=0;q<=LONGKEY_INPUT_LEN;q++) { // clear values of all keys entered in
		longkey_input[q] = 0;
	}			
}	

//TODO : take this out , place somewhere else
void Cancel_Reservation(void) {	// clears the reservation timeout and all symbols for it
	//reservation_timeout_val=0;
	reservation_isvalid = 0; // Reservation no longer valid
//	pin_led(0); // turn off pin LED
	lite_flash_val=0; // turn off flashing LED
//	Lite(0);	// turn off Front and Back LED Lighting
	for (unsigned int w=0;w<9;w++) {	// Clear account information
		account[w] = 0;		
	}		
}


/*
For Timers, it important to service the ISR! b/c was resetting because it jumped nowhere
*/

//Timer0 ISR clocks at 
//fcn = (16MHz) / (2 * 64 * 0x7D ) =992.06 Hz
//approx every 1.008ms
ISR(TIMER0_OVF_vect){
	
	cli();
}

ISR(TIMER0_COMPA_vect){
	milicount++;
	cli(); 
}

//Timer1 ISR clocks at 
//fcn = (16MHz) / (2 * 256 * 0x7A13) = 1.00000032 Hz
// approx every 0.52s
	
ISR(TIMER1_OVF_vect){ 

	cli();
}	
	
ISR(TIMER1_COMPA_vect){
	
 
	  //todo: is there a timeout for how long to wait for user to unlock bike after reservation?
  
	 last_talk++; // increment the last_talk counter by 1. Last_talk is last time sent data to server 
	
		// Basic format is:
		//if timeout is set
			  // increment that guys counter
		// if overflow for timeout
			  //Do Timed_Out_true behavior

	if(sleep_timeout_flag){
		if(sleep_timeout_counter > SLEEP_TIMEOUT){
			
			sleep_timeout_ready = 1;
		}
		else{
			sleep_timeout_counter++;
		}
		
		
	}

	if(confirm_timeout_flag)
	{
		if(confirm_timeout_counter > CONFIRM_TIMEOUT){
			confirm_timeout_ready = 1;
		}
		else{
			confirm_timeout_counter++;
		}			
		
	}
	
	if(pin_timeout_flag){
		
		if(pin_timeout_counter > PIN_TIMEOUT){
			pin_timeout_ready = 1;
		}
		else{
			pin_timeout_counter++;
		}
	}
	
	if(unlock_timeout_flag){
		
		if(unlock_timeout_counter > UNLOCK_TIMEOUT ){
			unlock_timeout_ready = 1;
		}
		else{
			unlock_timeout_counter++;
		}
		
	}
	
	if(hold_timeout_flag){
		
		if(hold_timeout_counter > HOLD_TIMER){
			hold_timeout_ready = 1;
		}
		else{
			hold_timeout_counter++;
		}
	}
	
	if(broken_timeout_flag){
		
		if(broken_timeout_counter > BROKEN_TIMER){
			broken_timeout_ready = 1;
		}
		else{
			broken_timeout_counter++;
		}
	}
	
	cli(); //why??

} //end ISR

void EnableTimer(void) { 
	//16 bit TIMER1 = Timeout cases in FSM
	
	//CTC operation for 16 bit timer 1
	//prescale set to 256
	//fcn = (16MHz) / (2 * 256 * 0x7A13)
	TCNT1H = 0x00; //reset timer register
	TCNT1L = 0x00;
	
	//set to count 31,250 + 1 times
	OCR1AH = 0x7A;
	OCR1AL = 0x13;
	
	//sensitive to OCRnA , CTC mode
	TCCR1B |= (1 << WGM12);
	//TCCR1B &= (0 << WGM10) | (0 << WGM11) | (0 << WGM13); DONT CLEAR
	//prescale to 256
	TCCR1B |= (1 << CS12);
	//TCCR1B &= (0 << CS11) | (0 << CS10); //DONT CLEAR
	//set interrupts for TIMER1
	TIMSK1 |= (1 << OCIE1A)| (1 << TOIE1); 

	//8 bit Timer0 = getMillis() 
	
	TCNT0 = 0x00;
	//CTC operation
	//64 prescaler
	//1000 Hz =  (16MHz / (2 * 64 * 0x7D)
	OCR0A = 0x7D;
	TCCR0A |= (1<<WGM10); //ctc
	TCCR0B |= (1<<CS01) | (1<<CS00); //64
	//set interrupts for TIMER0
	TIMSK0 |= (1<<OCIE0A) | (1 << TOIE0);
	
	
	/*
	//normal timer with 
	TCCR1B |= (1 << CS12); //starts counter w/ prescale at 256
	
	
	TIMSK1 |= (1 << TOIE1);// if set for top - 0xFFFF and normal mode
	*/
	
}	   

void DisableTimer(void) {
	//todo: actually turn off timer counting!
	TIMSK1 &= (0 << TOIE1);
}
/*
uint32_t getMillis(void){
	
	return milicount;
}
	
*/