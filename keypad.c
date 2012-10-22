

#include "keypad.h"
#include "SoR_Utils.h"
	



void Enable_KeypadTimeout(void) {
	//keypad_timeout_val = 20; // 10 second key timeout, .5 second steps
	//key_isvalid = 1; // keys are valid before timed out  ////you want to reimplement this
	//keypad_timeout_counter=0; // reset counting variable for timeout
}

void KeypadInit(void) {
	//PCINT7 has enabled pull-up resistor configure_ports()
	
	 //Enable Pin Change Interrupt on PCINT7 - verified working
	 PCMSK0 |= (1 << PCINT7); //set (enable) interrupts on pin 30 (KEY_INT/PCINT7)
	 
	 //This interrupt is flagged by PCI0
	 //EFIR are flags which are cleared when an interrupt has occurred
	 //EIMSK allows external interrupt to be posted
	 
	 PCICR |= (1 << PCIE0); // enable all pin interrupts level based
	 //why not rising edge to avoid power glitch issues
	 
	 //does EICRA need to be cleared for low level interrupt sense control?
	 
	PCIFR = (1 << PCIE0); //interrupt flag register, set when interrupt is posted , occurs auto
	
	 //depending on interrupt configuration , you might have to do some timing analysis
	 //based on the expected time frame such that the uC can wake up AND handle interrupt
	 
	 
	 sei(); // enable interrupt via I bit in SREG for TWI Interrupts

	//Configure Key Pad Controller
	
	TWISend(KEYPAD_ADDR,0x01,0x01); 
	//TWISend(KEYPAD_ADDR,0x01,0b01111001); // Config for overflow and 50uS interrupt length
	TWISend(KEYPAD_ADDR,0x02,0x01); // Clear All Interrupts	
//	TWISend(KEYPAD_ADDR,0x03,0x00); // key lock and event counter register
	

//	Enable Keypad matrix -- setting enables key matrix element
	TWISend(KEYPAD_ADDR,0x1D,0b00001111); // row 0 1 2 3
	TWISend(KEYPAD_ADDR,0x1E,0b00001111); // col 0 1 2 3

/*	
	// Clear all GPIO Interrupts
	TWISend(KEYPAD_ADDR,0x1A,0x00);
	TWISend(KEYPAD_ADDR,0x1B,0x00);
	TWISend(KEYPAD_ADDR,0x1C,0x00);
*/	

/*
	// Debounce Enabled 
	TWISend(KEYPAD_ADDR,0x29,0x00);
	TWISend(KEYPAD_ADDR,0x2A,0x00);
	TWISend(KEYPAD_ADDR,0x2B,0x00);
	
	// Pullup Enabled
	TWISend(KEYPAD_ADDR,0x2C,0x00);
	TWISend(KEYPAD_ADDR,0x2D,0x00);
	TWISend(KEYPAD_ADDR,0x2E,0x00); 
*/
	cli(); // clear interrupt for init purposes
	
	//if interrupt occurs call readKeypad()
}


unsigned char Key2Dec(unsigned char raw) {	// Key to Decimal Conversions for Key Presses
	if (raw <= 131 ) {	//1st row
		return raw-128;	
	}
	else if (raw == 132) {	// Hold key
		return 132;		
	}		
	else if (raw <=141) { //2nd row
		return raw-135; 	
	}
	else if (raw <= 151) { //3rd row
		return raw-142;
	}
	else if (raw == 160) { // Zero Key
		return 0;	
	}
	else {
		return raw;	// else its a special key - Hold, Repair, Enter, Cancel 	
	}		
							 	
}	


unsigned char IsCorrectCode(void) {
	for (unsigned char w=0;w< KEY_INPUT_LEN; w++ ) {
		if ( key_input[w] != pin[w] )
			return FALSE;	
	}
	return TRUE;	
}		

void Clear_KeypadInterrupt(void) {
	TWISend(KEYPAD_ADDR,KEY_INT_REG,0x01); // Clear All Interrupts	, clearing keypad interrupt pin	 	
}	

unsigned char IsActive_KeypadInterrupt(void) {
	return PORT_IS_OFF( PINA,7);
}	


	//keypad_addr  = 0x68 = lsb is r/w bit , rest is address defined by component
	//key_reg = 0x04 = key_event_register_A
	//store FIFO byte buffer into reg
	//readup on byte format
	//key presses have msb set to '1' , releases have msb set to '0'

//for short key PIN 	
void ReadKeypad(void) {
	unsigned char reg = 0; // input register from Keypad Controller (byte size)
	TWIGet(KEYPAD_ADDR,KEY_REG,&reg);	// load first byte from Keypad Controller's buffer

	while(reg != 0 ) { // while there is some key press/release in the buffer
		if (reg > 127) {		// Key Presses ( as opposed to key releases) have 7th bit set (>=127)
			unsigned char val =  Key2Dec(reg);	// Convert raw key input to decimal value
			//rprintf("Press %d\n",val) ;  // Pressed a Key
			
			//where is key_count update and key_input_len is defined constant depending on state? or independent of state?
			if ( key_count > KEY_INPUT_LEN ) {	// if Overflow of keys pressed
				for (unsigned char w=0;w< KEY_INPUT_LEN; w++ ) { // from key#0 to key#3, shift all over to the left
					key_input[w] = key_input[w+1];
				}				
				key_count=KEY_INPUT_LEN;
			}	// end if overflow keys	
			
			//possible memory leakage?
					
			key_input[key_count] = val;	// Load up key_input array
			key_count++;	// increment number of keys in key_input array
			
	
		}	
		else {//Key Released
		//		rprintf("Release %d\n", reg);	// Released a Key
		} 
			
		TWIGet(KEYPAD_ADDR,KEY_REG,&reg);	//load next from Keypad Controller's buffer
				
	} //  while there is something in keypad controller buffer					
				
} 


// for 6 digit account and enter key, 7 digits
void ReadLongKeypad(void) {
	unsigned char reg = 0; // input register from Keypad Controller (byte size)
	TWIGet(KEYPAD_ADDR,KEY_REG,&reg);	// load first byte from Keypad Controller's buffer

	while(reg != 0 ) { // while there is some key press/release in the buffer
		if (reg > 127) {		// Key Presses ( as opposed to key releases) have 7th bit set (>=127)
			unsigned char val =  Key2Dec(reg);	// Convert raw key input to decimal value
			//rprintf("Press %d\n",val) ;  // Pressed a Key
			
			if ( longkey_count > LONGKEY_INPUT_LEN ) {	// if Overflow of keys pressed
					for (unsigned char w=0;w< LONGKEY_INPUT_LEN; w++ ) { // from key#0 to key#3, shift all over to the left
						longkey_input[w] = longkey_input[w+1];
					}
					longkey_count=LONGKEY_INPUT_LEN;
				}	
				longkey_input[longkey_count] = val;	// Load up longkey_input array
				longkey_count++;	// increment number of keys in longkey_input array
			
		}
		else{ //Key Release
			//rprintf("Release %d\n", reg);	// Released a Key
		}			
		TWIGet(KEYPAD_ADDR,KEY_REG,&reg);	//load next from Keypad Controller's buffer
		
	} //  while there is something in keypad controller buffer		
}


/////////////// BELOW IS CODE FOR THE 4 DIGIT KEY INPUT - FOR PIN ENTRY /////////////   
unsigned char IsIn_KeyArray(unsigned char val) {
	for (unsigned char w=0;w<=KEY_INPUT_LEN;w++) {
		if ( key_input[w] == val) 
			return TRUE;	
	}
	return FALSE;			
}	

 // returns Action Code, of what to do
unsigned char getKeypadAction(void) {
	
	if ( IsIn_KeyArray(CANCEL_KEY) ) {// if it contains a Cancel Key, clear all before cancel key and shift to left
		return CANCEL_KEY;
	}		
	else if ( IsIn_KeyArray(HOLD_KEY) ) {// if it contains a Hold Key
		return HOLD_KEY;
	}		
	else if ( IsIn_KeyArray(REPAIR_KEY) ) { // if it contains a Repair Key
		return REPAIR_KEY;
	}
	else if ( key_input[KEY_INPUT_LEN] == ENTER_KEY ) { // if last key input is an Enter Key
		return CHECK_PIN;
	}	
	else if ( key_count == KEY_INPUT_LEN+1 ) { // Long Enough, but improper Enter Key
		return BAD_ENTER;
	}
	else {
		return TOO_SHORT;
	}		 		
	
}	

int ProcessKeypad (unsigned char KeyAction) {	// do what needs to be done, after keypad is processed
int keypadIs;			
			switch ( KeyAction ) {
					case TOO_SHORT:	// not enough number keys inputted
						// Do Nothing
						//rprintf(" Too Short \n");
						keypadIs = WAITING;
						break;
					case BAD_ENTER:	// long enough, but enter key at wrong place
						//rprintf(" Bad Enter : %d %d %d %d\n",key_input[0],key_input[1],key_input[2],key_input[3]);
						//flash_locked_led();
						Clear_KeyArray();	// Clear Key Array
						// rprintf(" No Enter Key \n");
						keypadIs = ENTER_CONFUSED;
						break;
					case CANCEL_KEY:
						// rprintf(" Cancel Key \n");
						Clear_KeyArray();	// Clear Key Array
						keypadIs = CANCELD;
						break;
					case HOLD_KEY:
						// rprintf(" Hold Key \n");
						/*
						if ( Owner_STATE == OWNED ) {
							//hold_led(1);
							waitforHoldLock_FLAG =1;
							hold_flash_timeout_val = 240; // 120 seconds to lock bike
						}
						*/
						keypadIs = HOLD_INPUT;
						break;
					
					case REPAIR_KEY:
						// rprintf(" Repair Key \n");
						/*
						if( Owner_STATE == OWNED) {	// if Bike is Owned
							Repair_STATE = BROKEN;
							//repair_led(1); 
							//toSendState_FLAG=1;
						}	
						*/
						keypadIs = REPAIR_INPUT;						
						break;
					
					case CHECK_PIN:
						//rprintf(" Check Pin Code : %d %d %d %d\n",key_input[0],key_input[1],key_input[2],key_input[3]);
						// todo: some check in code to make sure its a real secret code, not just default initialized
						//default initialized should be set to a secret init state
						//Here handling should not occur, state function pointers must work with this
						
						
						//key_isvalid = 1;
						// Clear Key Array	and Key Timeout
						keypadIs = VALID_INPUT;
						break;
		} //end case switch	
		return keypadIs;
}	




//////////////////////// BELOW IS CODE FOR 9 DIGIT KEY INPUT -- ACCOUNT NUMBER AND OVERRIDE /////////////

unsigned char IsIn_LongKeyArray(unsigned char val) {
	for (unsigned char w=0;w<=LONGKEY_INPUT_LEN;w++) {
		if ( longkey_input[w] == val)
		return TRUE;
	}
	return FALSE;
}

// returns Action Code, of what to do
unsigned char getLongKeypadAction(void) { 
	
	if ( IsIn_LongKeyArray(CANCEL_KEY) ) {// if it contains a Cancel Key, clear all before cancel key and shift to left
		return CANCEL_KEY;
	}			
	else if ( longkey_input[LONGKEY_INPUT_LEN] == ENTER_KEY ) { // if last key input is an Enter Key
			return CHECK_PIN;	//core part lol
	}	
	else if ( longkey_count == LONGKEY_INPUT_LEN+1 ) { // Long Enough, but improper Enter Key
		return BAD_ENTER;
	}		
	else {
		return TOO_SHORT;
	}		 		
	
}	



// do what needs to be done, after keypad is processed
int ProcessLongKeypad (unsigned char KeyAction) {	
			int keypadIs;
			switch ( KeyAction ) {
					case TOO_SHORT:	// not enough number keys inputted
						// Do Nothing
						//rprintf(" Too Short \n");
						keypadIs = WAITING;
						break;
					case BAD_ENTER:	// long enough, but enter key at wrong place
						//rprintf(" Bad Enter : %d %d %d %d %d %d \n",longkey_input[0],longkey_input[1],longkey_input[2],longkey_input[3],longkey_input[4],longkey_input[5],longkey_input[6]);
					//	flash_locked_led();
						Clear_LongKeyArray();	// Clear Key Array
						// rprintf(" No Enter Key \n");
						keypadIs = ENTER_CONFUSED;
						break;
					case CANCEL_KEY:
						//rprintf(" Cancel Key \n");
						Clear_LongKeyArray();	// Clear Key Array
						keypadIs = CANCELD;
						break;
					case CHECK_PIN: // means its an account
											
						keypadIs = VALID_INPUT;
						//longkey_isvalid = 1;
						break;
		} 
		
		return keypadIs;
}	

int LongKeypad_MainFunction (void){
	cli(); //disable interrupts , maybe not necessary
	
	ReadLongKeypad();
	//Enable_KeypadTimeout(); //sets certain counters / flags
	//startTimer();	// I guess idea would be to set a very quick timer , sleep timer
	Clear_KeypadInterrupt();	//neccesary for some reason
	// returns action code - TOO_SHORT, NO_ENTER, CHECK_PIN , CANCEL_KEY, HOLD_KEY, REPAIR_KEY
	unsigned char KeyAction=0;
	KeyAction =	getLongKeypadAction();
	int processReturn;
	processReturn = ProcessLongKeypad(KeyAction);// Process the keypad
	
	sei(); // enable
	return processReturn;
}

						
int Keypad_MainFunction (void) {
		cli();
	
		ReadKeypad();	// read keypad, store it into key_input[] array
		//Enable_KeypadTimeout(); //sets certain counters / flags	, pin timer
		//startTimer();	// I guess idea would be to set a very quick timer				
		Clear_KeypadInterrupt();	//what would be the point of this within the read?
		unsigned char KeyAction=0;
		// returns action code - TOO_SHORT, NO_ENTER, CHECK_PIN , CANCEL_KEY, HOLD_KEY, REPAIR_KEY	
		KeyAction =	getKeypadAction(); 
		int processReturn;
		processReturn = ProcessKeypad(KeyAction);// Process the keypad	
		
		sei();
		return processReturn;
					
}	 




	
	
	/*	if ( Owner_STATE == AVAILABLE  && !(reservation_isvalid) ) {   // If no reservation and Bike is Available
							 if ( !IsIn_LongKeyArray(HOLD_KEY) && !IsIn_LongKeyArray(REPAIR_KEY) ) {	// sanitize only numeric								 
								 for (unsigned char w=0; w< LONGKEY_INPUT_LEN;w++) {
									 account[w] = longkey_input[w];	// Load up Account Number
								 }
								 rprintfInit(DebugPrint); rprintf("Account Number Entered\n");
								 rprintf("Account #: %d %d %d %d %d %d %d %d %d\n",longkey_input[0],longkey_input[1],longkey_input[2],longkey_input[3],longkey_input[4],longkey_input[5]);
								 
								 //todo: If I send State now, with Owner_STATE = AVAILABLE and an account_number field, isn't that a good reservation info?
								
								 // Server should reply with verification and turn on hello pin led, like a regular reservation
								 // todo: remember to send state twice to receive reservation data
								 toSendState_FLAG=1; //todo: maybe set this to two or something and then decerement									  
							 }	//endif sanitation							 
							
						} // endif bike is available 
					
						if ( IsOverride() == TRUE ) {	// Override Number
							rprintfInit(DebugPrint); rprintf("Override Active!");
							locked_led(0); unlocked_led(1);
							Lock_STATE = RELEASED;
							Disengage_Lock();
							
						//	toSendState_FLAG=1;
						}									
					//	else {	// Wrong Code!
					//	}
			
						Clear_LongKeyArray();	// Clear Key Array	and Key Timeout
					*/		
	
	/*
						if ( IsCorrectCode() == TRUE && ( Owner_STATE == AVAILABLE || Owner_STATE == HOLD) && reservation_isvalid ) {   // Correct Code!
							locked_led(0);	// turn off locked led
							unlocked_led(1); // turn on unlock led
							hold_led(0); // turn off hold led , should be off in all cases
							pin_led(0); // turn off pin led, also should be off in all cases
							Owner_STATE = OWNED;
							Lock_STATE = RELEASED;
							Cancel_Reservation();
							Disengage_Lock();
							toSendState_FLAG = 1;	
							
							// new added
							waitforRemoveLock_FLAG =0;
							unlocked_butnotremoved_timer = 240; // 2 minutes, 120 seconds * 2 because of 1/2 second resolution
							
							// Reset pin code to 255,255,255,255 ? no need, reservation_isvalid takes care
							// wait for RemoveLock Flag -- want to know user removed the lock
							// Enable waitforRemoveLock Timeout -- maybe relock if user doesnt remove in time
						}
						else if ( Owner_STATE != AVAILABLE) {
							flash_locked_led();
							rprintf("Owner State Is not AVAILABLE");
						}							
						else {	// Wrong Code!
							flash_locked_led();
							rprintf(" Wrong Pin! Should be %d %d %d %d\n",pin[0],pin[1],pin[2],pin[3]);
						}
						*/