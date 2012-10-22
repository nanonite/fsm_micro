

#include "mc_message.h"

#include "SoR_Utils.h"

// Maximum size of the state message.
#define MIN_STATE_SIZE (                                                \
    sizeof(uint16_t) + /* message size */                               \
    sizeof(uint8_t) +  /* repair_state*100 + owner_state*10 + lock_state) */ \
    sizeof(uint32_t) + /* timestamp */                                  \
    BIKE_ID_LEN +                                                       \
    ACCOUNT_LEN +                                                       \
    PIN_LEN +                                                           \
    sizeof(uint8_t) +  /* battery level */                              \
    sizeof(uint8_t) /* positions count */)


/*
 * All functions encoding integer types output data in big endian
 * (network-byte order).
 *
 * Caller must make sure that the output buffer is large enough to
 * hold the input number, otherwise buffer will overflow.
 */

//State functionality
enum fsm_state_t available_Timeout(void){
	
	sleep_timeout_flag = 0;
	sleep_timeout_counter = 0;
	sleep_timeout_ready = 0;
	ReadLongKeypad();
	Clear_LongKeyArray(); //reset keypad array
	//Serial1.write("Timeout: Going to Sleep\n");
	return ST_SLEEP;
}
//conditional state
enum fsm_state_t available_Server(void){
	//Serial1.write("Next State : ST_WAIT_PIN\n");
	return ST_WAIT_PIN;
}
//Server returned valid Account ID
enum fsm_state_t available_Key_Reserve(void){
	
	//set configure timer
	confirm_timeout_ready = 0; //timeout flag for when timeout occurs
	confirm_timeout_counter = 0; //reset counter
	confirm_timeout_flag = 1; //flag for timer to start counting
	
	//reset sleep timer
	sleep_timeout_counter = 0;
	sleep_timeout_flag = 0;
	sleep_timeout_ready = 0;
	//longkey_isvalid = 0; //prevent button presses
	
	//Serial1.write("Next State : ST_ACNT_CONFIRM\n");
	//functions to update stateInfo struct?
	return ST_ACNT_CONFIRM;
}
enum fsm_state_t available_Default(void){
	locked_led(1);
	//possibly establish connection with GSM?
	//Serial1.write("Inside ST_AVAILABLE\n");
	return ST_AVAILABLE;
}


enum fsm_state_t acntConfirm_Timeout(void){
	ReadLongKeypad();
	Clear_LongKeyArray(); //reset array vector
	
	//reset confirm timer
	confirm_timeout_flag = 0;
	confirm_timeout_ready = 0;
	confirm_timeout_counter = 0; 
	
	//set sleep timer
	sleep_timeout_counter = 0 ;
	sleep_timeout_flag = 1;
	sleep_timeout_ready = 0;
	
	//Serial1.write("Server could not be reached\n");
	//Serial1.write("Next State : ST_AVAILABLE\n");
	
	return ST_AVAILABLE;
}	
//conditional case
enum fsm_state_t acntConfirm_Server(void){
	
	//set timer flags for pin wait
	pin_timeout_counter = 0;
	pin_timeout_flag = 1;
	pin_timeout_ready = 0;
	//reset timer flags for Account Confirm
	confirm_timeout_counter = 0;
	confirm_timeout_flag = 0;
	confirm_timeout_ready = 0;
	//LEDs
	locked_led(1);
	pin_led(1);
	//Reset Keypad Buffer
	ReadKeypad();
	Clear_KeyArray();
	//Serial1.write("Next State : ST_WAIT_PIN\n");
	return ST_WAIT_PIN;
}

enum fsm_state_t acntConfirm_Default(void){
	locked_led(1);
	
	ReadLongKeypad();
	Clear_LongKeyArray();
	//Serial1.write("Inside ST_ACNT_CONFIRM\n");
	return ST_ACNT_CONFIRM;
}

enum fsm_state_t pinWait_Timeout(void){
	//Serial1.write("Took too long to enter PIN\n");
	
	//set sleep timer
	sleep_timeout_counter = 0 ;
	sleep_timeout_flag = 1;
	sleep_timeout_ready = 0;
	
	//clear Pin Timer
	pin_timeout_counter = 0;
	pin_timeout_flag = 0;
	pin_timeout_ready = 0;
	
	//LEDs
	locked_led(1);
	pin_led(0);
	
	//Serial1.write("Next State : ST_AVAILABLE\n");
	return ST_AVAILABLE;
}
//Web Cancelation 
enum fsm_state_t pinWait_Server(void){
	
	//LEDs
	locked_led(1);
	pin_led(0);
	
	//Serial1.write("Next State2 : ST_AVAILABLE\n");
	return ST_AVAILABLE;
}
//conditional case 3
enum fsm_state_t pinWait_Key_Pin(void){	
	//Serial1.write("PIN entered correctly\n");
	//setup unlock stage conditions
	unlock_flag = 0; 
	temp_state = 1; 
	//set unlock timer
	unlock_timeout_counter = 0;
	unlock_timeout_flag = 1;
	unlock_timeout_ready = 0;
	//LEDs
	locked_led(0);
	pin_led(0);
	unlocked_led(1);
	
	//Serial1.write("Next State :  ST_WAIT_UNLOCK\n");
	return ST_WAIT_UNLOCK;
}
enum fsm_state_t pinWait_Default(void){
	locked_led(1);
	pin_led(1);
	//Serial1.write("Inside ST_WAIT_PIN\n");
	return ST_WAIT_PIN;
}

enum fsm_state_t unlockWait_Timeout(void){
	//reset to lock state
	unlock_flag = 0;
	lock_flag = 1;
	temp_state = 0;
	//reset unlock timer
	unlock_timeout_counter = 0;
	unlock_timeout_flag = 0;
	unlock_timeout_ready = 0;	
	//set sleep timer
	sleep_timeout_counter = 0 ;
	sleep_timeout_flag = 1;
	sleep_timeout_ready = 0;
	//Serial1.write("Next State :  ST_AVAILABLE\n");
	return ST_AVAILABLE;
}

//Bike has been unlocked, going to ride state
enum fsm_state_t unlockWait_Unlock(void){
	//Set lock states
	unlock_flag = 1; //for state struct
	lock_flag = 0;
	//reset unlock timer
	unlock_timeout_counter = 0;
	unlock_timeout_flag = 0;
	unlock_timeout_ready = 0;
	temp_state = 1;	
	//LEDs
	Lite(1);
	
	Engage_Lock();
	delay_ms(1000);
	
	//Serial1.write("Next State : ST_RIDE\n");
	return ST_RIDE;
}

//TODO : remove this case
enum fsm_state_t unlockWait_Key_Repair(void){
	//Serial1.write("Next State : ST_WAIT_LOCK_REPAIR\n");
	return ST_WAIT_LOCK_REPAIR;
}

enum fsm_state_t unlockWait_Default(void){
	
		//LEDs
		unlocked_led(1);
		locked_led(0);
		//Sense = 1 = Closed Circuit
		//Sense = 0 = Open Circuit
		
		//Block = 1 = free air, no bar / locked position
		//Block = 0 = pushed, bar present
	
		//unlock stage
		switch(temp_state)
		{
			//init
			// Block = 1 ; Sense  = 1
			//Block is not pressed ( in groove) = 1 / Is pressed = 0
			//Sense is closed circuit = 1 // Sense is open circuit = 0
			case 1:
			Disengage_Lock();
			/*
			Serial1.writeInit(DebugPrint);
			Serial1.write("Sensor is : %d\n", sense_state);
			Serial1.writeInit(DebugPrint);
			Serial1.write("Block is : %d\n", block_state);
			*/
			if(!sense_state){
				temp_state = 2;
				
			}
			//if Sensor series connection is broken, Bar must engage lock and poll block
			break;
			
			case 2:
			
			delay_ms(1000); // wait a bit
			//if sensor series connection still broken return to case 1
			Engage_Lock();
			/*
			Serial1.write("Sensor is : %d\n", sense_state);
			Serial1.writeInit(DebugPrint);
			Serial1.write("Block is : %d\n", block_state);
			*/
			if(!block_state) // if mid lock being pushed and open circuit
			temp_state = 3; //else possible error handeling??
			else{
				delay_ms(2000);
				if(block_state)
				temp_state = 4;
			}
			break;
			
			case 3:
			if(block_state){
				
				temp_state = 4;
			}
			
			case 4:
			unlock_flag = 1;
			//Serial1.write("outta here!\n");			
			break;
				
			
		}
		delay_ms(1000);
	//Serial1.write("Inside ST_WAIT_UNLOCK\n");
	return ST_WAIT_UNLOCK;
}

enum fsm_state_t ride_Lock(void){
	//Serial1.write("The lock was placed\n");
	
	//Flags for Lock State
	unlock_flag = 0;
	lock_flag = 1;
	
	//set sleep timer
	sleep_timeout_counter = 0 ;
	sleep_timeout_flag = 1;
	sleep_timeout_ready = 0;	
	
	//LEDs
	unlocked_led(0);
	locked_led(1);
	Lite(0);
	
	//clear keypads
	Clear_KeyArray();
	Clear_LongKeyArray();
		
	//Serial1.write("Next State :  ST_AVAILABLE\n");
	return ST_AVAILABLE;
}
enum fsm_state_t ride_Key_Hold(void){
	//Lock FSM Flags
	temp_state = 1;
	unlock_flag = 1;
	lock_flag = 0;
	hold_key_count = 0;
	//LEDs
	unlocked_led(1);
	hold_led(1);
	
	//delay_ms(3000); // wait debounce
	//Serial1.write("Next State :  ST_WAIT_LOCK_HOLD\n");
	return ST_WAIT_LOCK_HOLD;
}
enum fsm_state_t ride_Key_Repair(void){
	//Lock FSM flags
	temp_state = 1;
	unlock_flag = 1;
	lock_flag = 0;
	repair_key_count = 0;
	//LEDs
	unlocked_led(1);
	repair_led(1);
	
	delay_ms(3000); // wait debounce
	//Serial1.write("Next State :  ST_WAIT_LOCK_REPAIR\n");
	return ST_WAIT_LOCK_REPAIR;
}

enum fsm_state_t ride_Default(void){
	
	if(led_blink > BLINK_TIME){
		toggle_unlocked_led();
		toggle_Lite();
		led_blink = 0;
	}
	else
	{
		led_blink++;
		
	}
	
	Clear_LongKeyArray();
	Clear_KeyArray();
	
	switch(temp_state)
			{
				case 1://wait for middle lock
				
					//issue is that block_state jumps from 1 (not pressed) to 0 (pressed) alot
					if(!block_state) {
						temp_state = 2;
					}					
					
			
						
					break;
				
				case 2: //wait for sense 
					
					block_state = BlockIsTrue;
					if(!block_state && sense_state) // wait until circuit is closed
						temp_state = 3;
				
					break;
				
				case 3://wait for middle part to settle in
				
					//Serial1.writeInit(DebugPrint);
					//Serial1.write("Is the middle lock free?\n");
					if(block_state && sense_state)
							temp_state = 4;
					break;				
						
				case 4:
					//Serial1.writeInit(DebugPrint);
					//Serial1.write("Here it is!");
					lock_flag = 1;
					break;							
			}
	
	//Serial1.write("Inside ST_RIDE\n");
	return ST_RIDE;
}

//use case if someone wants to return to RIDE state
enum fsm_state_t holdWait_Hold(void){
	
	unlock_flag = 1;
	lock_flag = 0;
	//Serial1.write("Debounce HOLD_KEY 1 \n");
	if(hold_key_count > 1){
		//Serial1.write("Next State :  ST_RIDE\n");
		return ST_RIDE;
	}
	else{
		//Serial1.write("Debounce HOLD_KEY 2 \n");
		hold_key_count++;
		return ST_WAIT_LOCK_HOLD;
	}	
}

enum fsm_state_t holdWait_Lock(void){
	//set hold timers
	hold_timeout_counter = 0;
	hold_timeout_flag = 1;
	hold_timeout_ready = 0;
	//LEDs
	unlocked_led(0);
	locked_led(1);
	hold_led(1);
	Lite(0);
	//Keypad Reset
	ReadKeypad();
	Clear_KeyArray();
	
	//Serial1.write("Next State :  ST_HOLD\n");
	return ST_HOLD;
}

enum fsm_state_t holdWait_Default(void){
	
	if(led_blink > BLINK_TIME){
			toggle_hold_led();
			toggle_Lite();
			led_blink = 0;
		}
	else
		{
			led_blink++;
			
		}
	switch(temp_state)
	{
		case 1://wait for middle lock
		
		//issue is that block_state jumps from 1 (not pressed) to 0 (pressed) alot
		if(!block_state) {
			temp_state = 2;
		}
		
		
		
		break;
		
		case 2: //wait for sense
		
		block_state = BlockIsTrue;
		if(!block_state && sense_state) // wait until circuit is closed
		temp_state = 3;
		
		break;
		
		case 3://wait for middle part to settle in
		
		//Serial1.writeInit(DebugPrint);
		//Serial1.write("Is the middle lock free?\n");
		if(block_state && sense_state)
		temp_state = 4;
		break;
		
		case 4:
		//Serial1.writeInit(DebugPrint);
		//Serial1.write("Here it is!");
		lock_flag = 1;
		break;
	}
	
	
	//Serial1.write("Inside ST_WAIT_LOCK_HOLD\n");
	return ST_WAIT_LOCK_HOLD;
}

enum fsm_state_t repairWait_Repair(void){
	
	//Serial1.write("Debounce REPAIR_KEY 1\n");
	if(repair_key_count > 1){
		//Serial1.write("Next State :  ST_RIDE\n");
		return ST_RIDE;
		
	}
	else{
		repair_key_count++;
		//Serial1.write("Debounce REPAIR_KEY 2\n");
		return ST_WAIT_LOCK_REPAIR;
	}

	};

enum fsm_state_t repairWait_Lock(void){
	//set sleep timer for broken
	sleep_timeout_ready = 0;
	sleep_timeout_counter = 0;
	sleep_timeout_flag = 1;
	//set broken timer
	broken_timeout_ready = 0;
	broken_timeout_counter = 0;
	broken_timeout_flag = 1;
	//LEDs
	unlocked_led(0);
	locked_led(1);
	repair_led(1);
	//Serial1.write("Next State :  ST_BROKEN\n");
	return ST_BROKEN;
}
enum fsm_state_t repairWait_Default(void){
	
		if(led_blink > BLINK_TIME){
			toggle_repair_led(); //6_S
			toggle_Lite();
			led_blink = 0;
		}
		else
		{
			led_blink++;
			
		}
		
	switch(temp_state)
	{
		case 1://wait for middle lock
		
		//issue is that block_state jumps from 1 (not pressed) to 0 (pressed) alot
		if(!block_state) {
			temp_state = 2;
		}
		
		
		
		break;
		
		case 2: //wait for sense
		
		block_state = BlockIsTrue;
		if(!block_state && sense_state) // wait until circuit is closed
		temp_state = 3;
		
		break;
		
		case 3://wait for middle part to settle in
		
		//Serial1.writeInit(DebugPrint);
		//Serial1.write("Is the middle lock free?\n");
		if(block_state && sense_state)
		temp_state = 4;
		break;
		
		case 4:
		//Serial1.writeInit(DebugPrint);
		//Serial1.write("Here it is!");
		lock_flag = 1;
		break;
	}
	
	//Serial1.write("Inside ST_REPAIR_LOCK_HOLD\n");
	return ST_WAIT_LOCK_REPAIR;
}

enum fsm_state_t hold_Timeout(void){
	//set sleep timer
	sleep_timeout_ready = 0;
	sleep_timeout_counter = 0;
	sleep_timeout_flag = 1;
	//reset hold timer
	hold_timeout_counter = 0;
	hold_timeout_flag = 0;
	hold_timeout_ready = 0;
	//LEDs
	hold_led(0);
	unlocked_led(0);
	locked_led(1);
	
	//Serial1.write("Next State : ST_AVAILABLE\n");
	return ST_AVAILABLE;
}
//conditional  3 cases
enum fsm_state_t hold_Key_Pin(void){
	//todo: implement logic for retry case
	
	//FSM flags
	lock_flag = 1;
	unlock_flag = 0;
	temp_state = 1;
	//LEDs
	unlocked_led(0);
	hold_led(0);
	
	//Serial1.write("Correct Pin !\n");
	//Serial1.write("Next State : ST_WAIT_UNLOCK\n");
	return ST_WAIT_UNLOCK;
}
//clarify case
enum fsm_state_t hold_Server(void){
	//Serial1.write("Next State : ST_AVAILABLE\n");
	return ST_AVAILABLE;
}

enum fsm_state_t hold_Default(void){
	//LEDs
	hold_led(1);
	locked_led(1);
	
	//Serial1.write("Inside  ST_HOLD\n");
	return ST_HOLD;
}

enum fsm_state_t broken_Timeout(void){
	//reset broken timers
	broken_timeout_counter = 0;
	broken_timeout_flag = 0;
	broken_timeout_ready = 0;
	//LEDs
	repair_led(0);
	unlocked_led(0);
	locked_led(0);
	return ST_SLEEP;
}
enum fsm_state_t broken_Server(void){
	//reset broken timers
	broken_timeout_counter = 0;
	broken_timeout_flag = 0;
	broken_timeout_ready = 0;
	//Serial1.write("Next State : ST_AVAILABLE\n");
	return ST_AVAILABLE;
}
enum fsm_state_t broken_Default(void){
	//LEDs
	repair_led(1);
	locked_led(1);
	unlocked_led(0);
	
	//Serial1.write("Inside ST_BROKEN\n");
	return ST_BROKEN;
}


void encode_uint16(uint16_t input, uint8_t* output) {
	// This outputs big endian no matter on what architecture it is
	// run.
	output[0] = (input >> 8) & 0xFF;
	output[1] = input & 0xFF;
}

void decode_uint16(const uint8_t* input, uint16_t* output) {
	*output = input[0];
	*output <<= 8;
	*output |= input[1];
}

void encode_uint32(uint32_t input, uint8_t* output) {
	output[0] = (input >> 24) & 0xFF;
	output[1] = (input >> 16) & 0xFF;
	output[2] = (input >> 8) & 0xFF;
	output[3] = input & 0xFF;
}

void decode_uint32(const uint8_t* input, uint32_t* output) {
	int i;
	*output = input[0];
	for (i = 0; i < sizeof(uint32_t); ++i) {
		*output <<= 8;
		*output |= input[i];
	}
}

static void copy_chars(const char* input,
int count,
char* output) {
	int i;
	for (i = 0; i < count; ++i, ++output, ++input) {
		*output = *input;
	}
}


int encode_state(const struct state* input, uint8_t* output, int output_len) {
	uint8_t* output_p = output;
	uint8_t i;

	if (output_len < MAX_STATE_SIZE) {
		// Message with few gps coordinates could fit into a smaller
		// buffer, but be conservative.
		return ERR_FULL;
	}

	// Reserve space for the message size which is now unknown.
	output_p += sizeof(uint16_t);
	
	 *output_p = (uint8_t)input->repair_state * 100
	  + (uint8_t)input->owner_state * 10
	  + (uint8_t)input->lock_state;
	
	output_p += 1;

	encode_uint32(input->timestamp, output_p);
	output_p += sizeof(uint32_t);

	copy_chars(input->bike_id, BIKE_ID_LEN, (char*)output_p);
	output_p += BIKE_ID_LEN;

	copy_chars(input->account, ACCOUNT_LEN, (char*)output_p);
	output_p += ACCOUNT_LEN;

	copy_chars(input->pin, PIN_LEN, (char*)output_p);
	output_p += PIN_LEN;

	*output_p = input->battery_level;
	output_p += 1;

	*output_p = input->positions_count;
	output_p += 1;

	for (i = 0; i < input->positions_count; ++i) {
		copy_chars(input->positions[i].compressed_lat, 4, (char*)output_p);
		output_p += 4;
		copy_chars(input->positions[i].compressed_lng, 5, (char*)output_p);
		output_p += 5;
		encode_uint32(input->positions[i].timestamp, output_p);
		output_p += sizeof(uint32_t);
	}

	// Finally output the message size at the start of the buffer.
	encode_uint16(output_p - output, output);
	return output_p - output;
}




/* older
int encode_state(const struct state* input, uint8_t* output, int output_len) {
	 uint8_t* output_p = output;
	 uint8_t i;

	 if (output_len < MAX_STATE_SIZE) {
		 // Message with few gps coordinates can fit into a smaller
		 // buffer, but be conservative.
		 return ERR_FULL;
	 }

	 // Reserve space for the message size which is now unknown.
	 output_p += sizeof(uint16_t);
	 *output_p = input->repair_state * 100
	 + input->owner_state * 10
	 + input->lock_state;
	 output_p += 1;

	 encode_uint32(input->timestamp, output_p);
	 output_p += sizeof(uint16_t);

	 copy_chars(input->bike_id, BIKE_ID_LEN, (char*)output_p);
	 output_p += BIKE_ID_LEN;

	 copy_chars(input->account, ACCOUNT_LEN, (char*)output_p);
	 output_p += ACCOUNT_LEN;

	 copy_chars(input->pin, PIN_LEN, (char*)output_p);
	 output_p += PIN_LEN;

	 *output_p = input->battery_level;
	 output_p += 1;

	 *output_p = input->positions_count;
	 output_p += 1;

	 for (i = 0; i < input->positions_count; ++i) {
		 copy_chars(input->positions[i].compressed_lat, 4, (char*)output_p);
		 output_p += 4;
		 copy_chars(input->positions[i].compressed_lng, 5, (char*)output_p);
		 output_p += 5;
		 encode_uint32(input->positions[i].timestamp, output_p);
		 output_p += sizeof(uint32_t);
	 }

	 // Finally output the message size at the start of the buffer.
	 encode_uint16(output_p - output, output);
	 return output_p - output;
 }
*/



//change when you get here
int decode_state(const uint8_t* input, int input_len, struct state* output) {
    const uint8_t* input_p = input;
    uint8_t i;
    uint16_t state_len;

    if (input_len <= sizeof(uint16_t)) {
        return ERR_TOO_SHORT;
    }
    // All checks below could be assertions if supported.
    decode_uint16(input_p, &state_len);
    input_p += sizeof(uint16_t);

    if (state_len > input_len) {
        return ERR_TOO_SHORT;
    }
    output->repair_state = (*input_p) / 100;
    output->owner_state = ((*input_p) / 10) % 10;
    output->lock_state = (*input_p) % 10;
    input_p += 1;

    decode_uint32(input_p, &output->timestamp);
    input_p += sizeof(uint16_t);

    copy_chars((char *)input_p, BIKE_ID_LEN, output->bike_id);
    input_p += BIKE_ID_LEN;
    output->bike_id[BIKE_ID_LEN] = 0;

    copy_chars((char *)input_p, ACCOUNT_LEN, output->account);
    input_p += ACCOUNT_LEN;
    output->account[ACCOUNT_LEN] = 0;

    copy_chars((char *)input_p, PIN_LEN, output->pin);
    input_p += PIN_LEN;
    output->pin[PIN_LEN] = 0;

    output->battery_level = *input_p;
    input_p += 1;

    output->positions_count = *input_p;
    input_p += 1;

    for (i = 0; i < output->positions_count; ++i) {
        copy_chars((char*)input_p, 4, output->positions[i].compressed_lat);
        input_p += 4;
        copy_chars((char*)input_p, 5, output->positions[i].compressed_lng);
        input_p += 5;
        decode_uint32(input_p, &output->positions[i].timestamp);
        input_p += sizeof(uint32_t);
    }

    return input_p - input;
}

int encode_state_update(const struct state_update* input,
                        uint8_t* output,
                        int output_len) {
    uint8_t* output_p = output;

    if (output_len < STATE_UPDATE_SIZE) {
        return ERR_FULL;
    }

    encode_uint16(STATE_UPDATE_SIZE, output_p);
    output_p += sizeof(uint16_t);

    *output_p = input->repair_state * 10 + input->owner_state;
    output_p +=1;

    copy_chars(input->account, ACCOUNT_LEN, (char*)output_p);
    output_p += ACCOUNT_LEN;

    copy_chars(input->pin, PIN_LEN, (char*)output_p);
    output_p += PIN_LEN;

    encode_uint16(input->reservation_timeout, output_p);
    output_p += sizeof(uint16_t);

    encode_uint16(input->hold_timeout, output_p);
    output_p += sizeof(uint16_t);

    encode_uint16(input->sleep_timeout, output_p);
    output_p += sizeof(uint16_t);

    encode_uint16(input->wakeup_timeout, output_p);
    output_p += sizeof(uint16_t);

    return output_p - output;
}

int decode_state_update(const uint8_t* input,
                        int input_len,
                        struct state_update* output) {
    const uint8_t* input_p = input;
    uint16_t update_len;

    if (input_len < STATE_UPDATE_SIZE) {
        return ERR_TOO_SHORT;
    }
    decode_uint16(input_p, &update_len);
    input_p += sizeof(uint16_t);
    if (update_len != STATE_UPDATE_SIZE) {
        return ERR_PROTO;
    }
    output->repair_state = (*input_p) / 10;
    output->owner_state = (*input_p) % 10;
    input_p +=1;

    copy_chars((char *)input_p, ACCOUNT_LEN, output->account);
    input_p += ACCOUNT_LEN;
    output->account[ACCOUNT_LEN] = 0;

    copy_chars((char *)input_p, PIN_LEN, output->pin);
    input_p += PIN_LEN;
    output->pin[PIN_LEN] = 0;

    decode_uint16(input_p, &output->reservation_timeout);
    input_p += sizeof(uint16_t);

    decode_uint16(input_p, &output->hold_timeout);
    input_p += sizeof(uint16_t);

    decode_uint16(input_p, &output->sleep_timeout);
    input_p += sizeof(uint16_t);

    decode_uint16(input_p, &output->wakeup_timeout);
    input_p += sizeof(uint16_t);

    return input_p - input;
}

int compress_position(const char* lat, const char* lng,
                      struct position* p) {
    unsigned char i;
    for (i = 0; i < 2; ++i) {  // First 4
	p->compressed_lat[i] = (lat[(i * 2)] - '0') * 10 +  (lat[(i * 2) + 1]- '0');
    }
    // Ignore period  lat[4] = period
    p->compressed_lat[2] = (lat[5] - '0') * 10 + (lat[6] - '0');
    p->compressed_lat[3] = (lat[7] - '0') * 10 + (lat[8] - '0');

    for (i = 0; i < 2; ++i) {  // First 4
        p->compressed_lng[i] =
                (lng[(i * 2)] - '0') * 10 + (lng[(i * 2) + 1] - '0');
    }
    // Ignore period  lng[5] = period
    p->compressed_lng[2] = (lng[4] - '0') * 10 + (lng[6] - '0');
    p->compressed_lng[3] = (lng[7] - '0') * 10 + (lng[8] - '0');
    p->compressed_lng[4] = (lng[9] - '0') * 10;

    // Adding 100, means its "negative"
    if (lat[9] == 'S') {
        p->compressed_lat[0] = p->compressed_lat[0] + 100;
    }
    if (lng[10] == 'W') {
        p->compressed_lng[0] = p->compressed_lng[0] + 100;
    }
    return 0;
}

#ifdef NEED_POSITION_DECOMPRESSION


// See https://github.com/sobi/micro-bike/blob/master/protocol.txt for
// the description of the format.
void decompress_position(const struct position* p,
                         double* latitude,
                         double* longitude) {
    int lat[8];
    int i;
    for (i = 0; i < 8; i += 2) {
        lat[i]= p->compressed_lat[i / 2] / 10;
        lat[i + 1] = p->compressed_lat[i / 2] % 10;
    }
    int lng[9];
    for (i = 0;  i < 8; i += 2) {
        lng[i] = p->compressed_lng[i / 2] / 10;
        lng[i + 1] = p->compressed_lng[i / 2] % 10;
    }
    lng[8] = p->compressed_lng[5] / 10;

    int lat_head = lat[0] * 10 + lat[1];
    if (lat[0] >= 10) {
        lat_head = -(lat_head - 100);
    }
    int lng_head = lng[0] * 100 + lng[1] * 10 + lng[2];
    if (lng[0] >= 10) {
        lng_head = -(lng_head - 1000);
    }

    double lat_end = 0;
    for (i = 2; i < 8; ++i) {
        lat_end *= 10;
        lat_end += lat[i];
    }
    lat_end /= 600000;

    double lng_end = 0;
    for (i = 3; i < 9; ++i) {
        lng_end *= 10;
        lng_end += lng[i];
    }
    lng_end /= 600000;

    if (lat_head < 0) {
        *latitude = lat_head - lat_end;
    } else {
        *latitude = lat_head + lat_end;
    }

    if (lng_end < 0) {
        *longitude = lng_head - lng_end;
    } else {
        *longitude = lng_head + lng_end;
    }
}

#endif

