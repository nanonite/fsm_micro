//control.c 
//Implements the FSM of the uC 

#include "mc_message.h"
#include "SoR_Utils.h"



//comment this line out to enter normal operation mode
#define DEBUG 1

#ifdef DEBUG
//set test data arrays
int test;
enum fsm_state_t list_event [15] = {EV_STATE_DEFAULT, EV_KEY_PRESS, EV_STATE_DEFAULT, EV_STATE_UPDATE, EV_STATE_DEFAULT, EV_KEY_PRESS, EV_STATE_DEFAULT,
	 EV_UNLOCK ,EV_STATE_DEFAULT, EV_HOLD_PRESS , EV_STATE_DEFAULT ,  EV_LOCK ,EV_STATE_UPDATE};


#endif

//FSM defintions

enum fsm_state_t current_state;
enum fsm_state_t next_state;// = ST_RIDE;

enum fsm_event_t evnt;

typedef struct {
	enum fsm_state_t st; //STATE
	enum fsm_event_t ev; //EVENT
	enum fsm_state_t (*fn)(void); //FUNCTION
} tTransition; // Transition Table

//Server State Info
typedef struct state stateInfo; 
stateInfo present_state;

//typedef struct position positionInfo;

/*
struct position present_position[] = {
	{{0,1,2,3} , {0,1,2,3,4} , 82}
	{{4,3,2,1} , {5,4,3,2,1} , 28}  
		};
*/


//Table Driven FSM
tTransition trans[] = {
{ ST_AVAILABLE, EV_TIMER, &available_Timeout},
{ ST_AVAILABLE, EV_KEY_PRESS, &available_Key_Reserve}, //conditional
{ ST_AVAILABLE, EV_STATE_UPDATE, &available_Server}, //conditional
{ ST_AVAILABLE, EV_STATE_DEFAULT, &available_Default},
	
{ ST_ACNT_CONFIRM, EV_TIMER, &acntConfirm_Timeout},
{ ST_ACNT_CONFIRM, EV_STATE_UPDATE, &acntConfirm_Server}, //conditional
{ ST_ACNT_CONFIRM, EV_STATE_DEFAULT, &acntConfirm_Default},
	
{ ST_WAIT_PIN, EV_TIMER, &pinWait_Timeout},
{ ST_WAIT_PIN, EV_KEY_PRESS, &pinWait_Key_Pin}, //conditional 3 different states
{ ST_WAIT_PIN, EV_STATE_UPDATE, &pinWait_Server},
{ ST_WAIT_PIN, EV_STATE_DEFAULT, &pinWait_Default},
	
{ ST_WAIT_UNLOCK, EV_TIMER, &unlockWait_Timeout},
{ ST_WAIT_UNLOCK, EV_UNLOCK, &unlockWait_Unlock},
{ ST_WAIT_UNLOCK, EV_REPAIR_PRESS, &unlockWait_Key_Repair},
{ ST_WAIT_UNLOCK, EV_STATE_DEFAULT, &unlockWait_Default},
	
{ ST_RIDE, EV_LOCK, &ride_Lock},
{ ST_RIDE, EV_HOLD_PRESS, &ride_Key_Hold},
{ ST_RIDE, EV_REPAIR_PRESS, &ride_Key_Repair},
{ ST_RIDE, EV_STATE_DEFAULT, &ride_Default},
	
{ ST_WAIT_LOCK_HOLD, EV_HOLD_PRESS , &holdWait_Hold},
{ ST_WAIT_LOCK_HOLD, EV_LOCK, &holdWait_Lock},
{ ST_WAIT_LOCK_HOLD, EV_STATE_DEFAULT, &holdWait_Default},

{ST_WAIT_LOCK_REPAIR, EV_REPAIR_PRESS , &repairWait_Repair},
{ST_WAIT_LOCK_REPAIR, EV_LOCK , &repairWait_Lock },
{ST_WAIT_LOCK_REPAIR, EV_STATE_DEFAULT , &repairWait_Default},

	
{ST_HOLD , EV_TIMER, &hold_Timeout} , 	
{ST_HOLD , EV_KEY_PRESS, &hold_Key_Pin} , 
{ST_HOLD , EV_STATE_UPDATE, &hold_Server} , //transition in question
{ST_HOLD , EV_STATE_DEFAULT, &hold_Default} , 

{ST_BROKEN , EV_TIMER , &broken_Timeout},
{ST_BROKEN , EV_STATE_UPDATE , &broken_Server},
{ST_BROKEN , EV_STATE_DEFAULT , &broken_Default}
	
};

//full size of FSM table is a multiple of TRANS_COUNT
#define TRANS_COUNT (sizeof(trans)/sizeof(*trans))

#define GPS_COUNT (sizeof(present_position)/sizeof(*present_position))




// todo: SendState ErrorCode - usually OK, ERROR means restart or reactivate
// todO: weird stuff where its Server Error 52 for a bit
//typename approach to remove "unkown type name" error?

//void control(int wake_Flag, SerialPort* telitPort, SerialPort* debugPort) {
void control(int wake_Flag){
	
	//conditional set init state due to what causes uC to wake up
	
	if(wake_Flag){
		current_state = ST_WAIT_PIN;
	}	
	else{
		//Starting from Keypad Press
		current_state = ST_AVAILABLE;		
		Engage_Lock();
		//init should read timestamp and fill position struct on data from GPS
		
		
		//init should read in SIM card info for BIKE ID , battery level
		present_state.repair_state = BIKE_WORKING;
		present_state.owner_state = BIKE_AVAILABLE;
		present_state.lock_state = BIKE_LOCKED;
		for(int i  = 0 ; i < BIKE_ID_LEN +1 ; i++){
			present_state.bike_id[i] = 0 ;
		}
		for(int i = 0 ; i < ACCOUNT_LEN + 1 ; i++){
			present_state.account[i] = 0 ;
		}
		for(int i = 0; i < PIN_LEN + 1 ; i++){
			present_state.pin[i] = 0;
		}
		
		
		present_state.timestamp = 0;
		present_state.battery_level = 0;
		present_state.positions_count = 2;
		//present_state.positions = *(struct position present_position); todo later
		pin_fail_flag = 0;
		//start sleep timeout
		sleep_timeout_flag = 1;
		sleep_timeout_counter = 0;
		sleep_timeout_ready = 0;
		//set locked state
		lock_flag = 1;
		
		turnOffLEDS();
	}
		
				
	
	//initialize gsm module
	
/*
	Toggle_PWRKEY();
	
	uart0SendByte('A');
	uart0SendByte('T');
	uart0SendByte('\n');
	while(1){
	uartFlag = uartReceiveByte(0 , &rData );
	uart1SendByte(rData);
		if(!uartFlag)
			break;
	}

*/
	//rprintfInit(DebugPrint);
	///rprintf("Starting");

			
	 while (current_state != ST_SLEEP) {
		 int i = 0;
		 evnt = getNextEvent();
		 //<----telitEventHandler();
		//sockethandler should  go here , and set flag
		 for (i = 0; i < TRANS_COUNT; i++) {
			 if ( (current_state == trans[i].st) ) {
				 if ( (evnt == trans[i].ev) ) {
					 next_state = (trans[i].fn)();
					 break;
				 }
			 }
		 }
		  current_state = next_state;
		
		  delay_ms(100);
	 }	
	 
	
	//setup sleep state and tell uC to loop in main

}


enum fsm_event_t getNextEvent(void){
	
	enum fsm_event_t EV_POINTER;
	int keyResult; 
	
	switch(current_state){
		
		case ST_AVAILABLE:
				// *************** Check Keypad for Account Input ****************** //
				if ( IsActive_KeypadInterrupt()  )	{
					keyResult = LongKeypad_MainFunction(); //test long key
					while(IsActive_KeypadInterrupt() ){} 
						
					switch(keyResult){
						case WAITING:
							EV_POINTER = EV_STATE_DEFAULT;
							break;
							
						case ENTER_CONFUSED:
							Clear_LongKeyArray();
							EV_POINTER = EV_STATE_DEFAULT;
							break;
						case CANCELD:
							Clear_LongKeyArray();
							EV_POINTER = EV_STATE_DEFAULT;
							break;
							
						case VALID_INPUT:
							EV_POINTER = EV_KEY_PRESS;
							break;
						default:
							EV_POINTER = EV_STATE_DEFAULT;
							break;
							
					}
					
				}
				else if(sleep_timeout_ready){
					EV_POINTER = EV_TIMER;
				}//test did server communicate?
				else{
					EV_POINTER = EV_STATE_DEFAULT;
				}
				
				//obvo include the events that are sensitive in this state (transitions from the state to another state)
				//eg
				//else if ( serverQuery the uC ? or check if Packet has been transmitted?)
				//dont really think due to server comm, idea is that uC always transmits current_state
				//and the server replies with stateUpdate if it wants the uC to change state
				
				//eg
				//if ( timer interrupt has been posted and serviced (set flag at the end of isr and disable Timer)
				//handle timeout case :
				//1) Case that you want to wait for less than 8 sec?
				//2) Case that you want to wait for more than 8 sec?
				
				break; // end ST_AVAILABLE
				
		case ST_ACNT_CONFIRM:

		//just used to fake a server response
				if ( IsActive_KeypadInterrupt()  )	{
					keyResult  = LongKeypad_MainFunction(); //test long key??
					while(IsActive_KeypadInterrupt() ){}
					
					switch(keyResult){
						
						case WAITING:
						
						EV_POINTER = EV_STATE_UPDATE;

						//#else
					//	EV_POINTER = EV_STATE_DEFAULT;
						break;
						
						case ENTER_CONFUSED:
						EV_POINTER = EV_STATE_DEFAULT;
						break;
						case CANCELD:
						EV_POINTER = EV_STATE_DEFAULT;
						break;
						
						case VALID_INPUT:
						EV_POINTER = EV_STATE_DEFAULT;
						break;
						
						default:
						EV_POINTER = EV_STATE_DEFAULT;
						break;					
					}						
					
				}// *************** Check Timer for Timeout ****************** //
				else if (confirm_timeout_ready){
					EV_POINTER = EV_TIMER;
					
				}
				else{ //timer not active, start?
					EV_POINTER = EV_STATE_DEFAULT;
				}
				
				//implement timeout case				
		
				break; // end ST_ACNT_CONFIRM
				
		case ST_WAIT_PIN:
		// *************** Check Keypad for Pin Input ****************** //
				if ( IsActive_KeypadInterrupt()  )	{
					keyResult  = Keypad_MainFunction(); // use short key version
					while(IsActive_KeypadInterrupt() ){}
					
					switch(keyResult){
					case WAITING:
					EV_POINTER = EV_STATE_DEFAULT;
					break;
				
					case ENTER_CONFUSED:
					
					Clear_KeyArray();
					EV_POINTER = EV_STATE_DEFAULT;
					break;
					case CANCELD:
					Clear_KeyArray();
					EV_POINTER = EV_STATE_DEFAULT;
					break;
				
					case VALID_INPUT:
					EV_POINTER = EV_KEY_PRESS;
					break;
				
					default:
					EV_POINTER = EV_STATE_DEFAULT;
					break;
				}
			}// *************** Check Timer for Timeout ****************** //
			else if(pin_timeout_ready){
				EV_POINTER = EV_TIMER;
			}
			else{
				EV_POINTER = EV_STATE_DEFAULT;
			}
			
		
		
			break; //end ST_WAIT_PIN
				
		case ST_WAIT_UNLOCK: 
			// *************** Check if Fully Unlocked ****************** //
			if(unlock_flag)
			{				
				EV_POINTER = EV_UNLOCK;
			}// *************** Check Timer for Timeout ****************** //
			else if(unlock_timeout_ready){
				EV_POINTER = EV_TIMER;
			}
			else{
				EV_POINTER = EV_STATE_DEFAULT;
			}
			//setup a function which polls a flag that is set from EV_STATE_DEFAULT
						
			break; // end ST_WAIT_UNLOCK
			
		case ST_RIDE :
			
			if ( IsActive_KeypadInterrupt()  )	{
				keyResult  = Keypad_MainFunction(); // use short key version
				while(IsActive_KeypadInterrupt() ){}
			
				switch(keyResult){
					case REPAIR_INPUT:
					EV_POINTER = EV_REPAIR_PRESS;
					break;
				
					case HOLD_INPUT:
					EV_POINTER = EV_HOLD_PRESS;
					break;
					
					default:
					EV_POINTER = EV_STATE_DEFAULT;
					break;
				}
			}
			else if(lock_flag){
				EV_POINTER = EV_LOCK;
			}
			else{
				EV_POINTER = EV_STATE_DEFAULT;
			}
		
			break; //end ST_RIDE
			
		case ST_WAIT_LOCK_HOLD:
		
			if ( IsActive_KeypadInterrupt()  )	{
				keyResult  = Keypad_MainFunction(); // use short key version
				while(IsActive_KeypadInterrupt() ){}
			
				switch(keyResult){
					case HOLD_INPUT:
					EV_POINTER = EV_HOLD_PRESS;
					break;
				
					default:
					EV_POINTER = EV_STATE_DEFAULT;
					break;
				}
			}
			else if(lock_flag)
			{
				EV_POINTER = EV_LOCK;
				
			}
			else{
				EV_POINTER = EV_STATE_DEFAULT;
			}
		
			break; //end ST_WAIT_LOCK_HOLD
			
		case ST_WAIT_LOCK_REPAIR:
		
			if ( IsActive_KeypadInterrupt()  )	{
				keyResult  = Keypad_MainFunction(); // use short key version
				while(IsActive_KeypadInterrupt() ){}
		
				switch(keyResult){
					case REPAIR_INPUT:
					EV_POINTER = EV_REPAIR_PRESS;
					break;
					
					default:
					EV_POINTER = EV_STATE_DEFAULT;
					break;
				}
			}
			else if(lock_flag){
				EV_POINTER = EV_LOCK;
			}
			else{
				EV_POINTER = EV_STATE_DEFAULT;
			}
		
			break; //end ST_WAIT_LOCK_REPAIR
		
		case ST_HOLD:
		
			if ( IsActive_KeypadInterrupt()  )	{
				keyResult  = Keypad_MainFunction(); // use short key version
				
				while(IsActive_KeypadInterrupt() ){}
			
				switch(keyResult){
					case WAITING:
					EV_POINTER = EV_STATE_DEFAULT;
					break;
				
					case ENTER_CONFUSED:				
					Clear_KeyArray();
					EV_POINTER = EV_STATE_DEFAULT;
					break;
				
					case CANCELD:
					Clear_KeyArray();
					EV_POINTER = EV_STATE_DEFAULT;
					break;
				
					case VALID_INPUT:
					EV_POINTER = EV_KEY_PRESS;
					break;
				
					default:
					EV_POINTER = EV_STATE_DEFAULT;
					break;
					}
			}	//else if StateUpdate
			else if(hold_timeout_ready) {
					EV_POINTER = EV_TIMER;
			}
			else{
				EV_POINTER = EV_STATE_DEFAULT;
			}
		
			break;
		
		case ST_BROKEN:
		
			if(broken_timeout_ready){
				EV_POINTER = EV_TIMER;
			}
			else{
				EV_POINTER = EV_STATE_DEFAULT;
			}
			//else if StateUpdate()
			break;
			
		case ST_SLEEP:
			//sleep is handled in Sobi.c
			break;
		
		
	} // end swtich(current_state)
	return EV_POINTER;
} 

/*
	enum fsm_event_t active_Event;
	enum fsm_state_t check_State;
	// *************** Check Keypad for Input ****************** //
	if ( IsActive_KeypadInterrupt()  ) {	// If a Key was pressed
		
		//develop case for hold and repair keys..
		
		check_State =  current_state; 
		if(check_State == ST_AVAILABLE){
			rprintf("Type in Account Number\n");
			Keypad_MainFunction(0); //test long key
			while(IsActive_KeypadInterrupt() ){}
		}
		else if( (check_State == ST_WAIT_PIN) || (check_State == ST_HOLD) ){
			rprintf("Type in PIN\n");
			Keypad_MainFunction(1); //test short key
			while(IsActive_KeypadInterrupt() ){}
		}	
		
		if(key_isvalid)
			active_Event = EV_KEY_PRESS;
		if(longkey_isvalid)
			active_Event = EV_KEY_PRESS;
		//if hold is pressed during ride , if repair is pressed during ride, or repair pressed during unlock_wait etc etc.
		
	} // *************** Check Timer for Timeouts ****************** //
	else if(current_state == ST_ACNT_CONFIRM){
		if(confirm_timeout_flag)
			active_Event = EV_TIMER;
		else
			active_Event = EV_STATE_DEFAULT; //I think this might have to be a communicate state
	}		
	else{
		active_Event = EV_STATE_DEFAULT;
	}
	//poll for confirm timeout ,case if server could not be reached
	
	return active_Event;
	//return ev_state_default if nothing has occured (i.e no interrupts have been serviced
	//return list_event[test]; DEBUG test for event seq
	*/