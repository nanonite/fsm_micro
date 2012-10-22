#ifndef MC_MESSAGE_H_
#define MC_MESSAGE_H_

#include "arch.h"
#include "keypad.h"
#include "global.h"

#ifdef __cplusplus

extern "C" {
	
#endif
//Minimum state size?
#define ERR_FULL -1
#define ERR_TOO_SHORT -2
#define ERR_PROTO -3

#define BIKE_ID_LEN 15
#define PIN_LEN 4
#define ACCOUNT_LEN 9

#define BLINK_TIME 5
// It does not make sense to sent messages bigger than 1492 bytes.
// Such messages exceed MTU for 3G network and will be sent in fragments
// anyway.

//#define MAX_POSITIONS 100
#define MAX_POSITIONS 2

// Size of a state update message (it is not equal to the size of the
// C structure).
#define STATE_UPDATE_SIZE ( \
    sizeof(uint16_t) + /* message size */                               \
    sizeof(uint8_t) +  /* repair_state * 10 + owner state */            \
    ACCOUNT_LEN +                                                       \
    PIN_LEN +                                                           \
    sizeof(uint16_t) + /* reservation timeout */                        \
    sizeof(uint16_t) + /* hold timeout */                               \
    sizeof(uint16_t) + /* sleep timeout */                              \
    sizeof(uint16_t)   /* wakeup timeout */)

// Maximum size of the state message.
#define MAX_STATE_SIZE (                                                \
    sizeof(uint16_t) + /* message size */                               \
    sizeof(uint8_t) +  /* repair_state*100 + owner_state*10 + lock_state) */ \
    sizeof(uint32_t) + /* timestamp */                                  \
    BIKE_ID_LEN +                                                       \
    ACCOUNT_LEN +                                                       \
    PIN_LEN +                                                           \
    sizeof(uint8_t) +  /* battery level */                              \
    sizeof(uint8_t) +  /* positions count */                            \
    MAX_POSITIONS * (9 * sizeof(char) + sizeof(uint32_t)) /* positions */)
	
//max  = 233 , owner , lock , repair

enum owner_state {
    BIKE_AVAILABLE = 1,
    BIKE_HOLD      = 2,
    BIKE_OWNED     = 3,
};

enum lock_state {
    BIKE_LOCKED   = 1,
    BIKE_RELEASED = 2,
    BIKE_UNLOCKED = 3,
};

enum repair_state {
    // TODO: update .proto to match these values.
    BIKE_WORKING = 1,
    BIKE_BROKEN = 2,
};


enum fsm_event_t {
	EV_STATE_UPDATE = 10,
	EV_STATE_DEFAULT = 11,
	EV_TIMER = 12,
	EV_INVALID_ACNT = 13,
	EV_VALID_ACNT = 14,
	EV_EMPTY_SMS = 15,
	EV_KEY_PRESS = 16,
	EV_HOLD_PRESS = 17,
	EV_REPAIR_PRESS = 18,
	EV_UNLOCK = 19,
	EV_LOCK = 20,
};

enum fsm_state_t {
	ST_SLEEP = -1,
	ST_AVAILABLE = 0,
	ST_ACNT_CONFIRM = 1,
	ST_WAIT_PIN = 2,
	ST_BROKEN = 3,
	ST_WAIT_UNLOCK = 4,
	ST_RIDE = 5,
	ST_HOLD = 6,
	ST_WAIT_LOCK_HOLD = 7,
	ST_WAIT_LOCK_REPAIR = 8,
};

struct position {
    // TODO: position encoding could probably be simplified.
    char compressed_lat[4]; // Not 0 terminated.
    char compressed_lng[5]; // Not 0 terminated.
    uint32_t timestamp;
};


/*
//sending correct packet screwed due encoding of enum types
struct state {
    enum repair_state repair_state; // Must be set.
    enum owner_state owner_state; // Must be set.
    enum lock_state lock_state; // Must be set.
    uint32_t timestamp; // Must be set.
    char bike_id[BIKE_ID_LEN + 1]; // Must be set.
    char account[ACCOUNT_LEN + 1]; // All 0s - not set.
    char pin[PIN_LEN + 1]; // All 0s - not set.
    uint8_t battery_level; // Must be set.
    uint8_t positions_count; // Can be 0.
    struct position positions[MAX_POSITIONS];
};
*/

struct state {
	uint8_t repair_state; // Must be set.
	uint8_t owner_state; // Must be set.
	uint8_t lock_state; // Must be set.
	uint32_t timestamp; // Must be set.
	char bike_id[BIKE_ID_LEN + 1]; // Must be set.
	char account[ACCOUNT_LEN + 1]; // All 0s - not set.
	char pin[PIN_LEN + 1]; // All 0s - not set.
	uint8_t battery_level; // Must be set.
	uint8_t positions_count; // Can be 0.
	struct position positions[MAX_POSITIONS];
};

struct state_update {
    // TODO: timestamp.
    enum repair_state repair_state; // 0 means keep the current repair_state.
    enum owner_state owner_state; // Must be set.
    char account[ACCOUNT_LEN + 1]; // All 0s - not set.
    char pin[PIN_LEN + 1]; // All 0s - not set.

    uint16_t reservation_timeout;
    uint16_t hold_timeout;
    uint16_t sleep_timeout;
    uint16_t wakeup_timeout;
	uint16_t pin_timeout;
	uint32_t timestamp;
};


// Timeout Variables and Counters // init function?

unsigned int reservation_timeout_counter,hold_flash_timeout_counter;
unsigned int lite_flash_val,lite_flash_counter;
unsigned int block_state_timer, sense_state_timer;
//unsigned int block_state_counter , sense_state_counter;
unsigned int unlocked_butnotremoved_counter;
unsigned int  unlocked_butnotremoved_timer;

unsigned char hold_key_count;
unsigned char repair_key_count;

//LED variables
unsigned char led_blink;


//Timer counters
unsigned int sleep_timeout_counter,
			  confirm_timeout_counter,
			  pin_timeout_counter,
			  unlock_timeout_counter,
			  hold_timeout_counter,
			  broken_timeout_counter;
			  
//Timer flags to start counting
unsigned char sleep_timeout_flag,
			  confirm_timeout_flag,
			  pin_timeout_flag,
			  unlock_timeout_flag,
			  hold_timeout_flag,
			  broken_timeout_flag;
			  
//Timer flags to post timeout for getEvent()
unsigned char sleep_timeout_ready,
			  confirm_timeout_ready,
			  pin_timeout_ready,
			  unlock_timeout_ready,
			  hold_timeout_ready,
			  broken_timeout_ready;

//Number of Seconds until Timeout occurs
#define SLEEP_TIMEOUT 45
#define CONFIRM_TIMEOUT 60
#define PIN_TIMEOUT 60 //This value should be dynamically set depending on server response, -> getPinTIMER()
#define UNLOCK_TIMEOUT 60
#define HOLD_TIMER 60 // This value should be dynamically set depending on server response
#define BROKEN_TIMER 45


/*
 * Encodes a state structure into an output buffer.
 *
 * The output buffer should have at least MAX_STATE_SIZE bytes.
 * Returns the length of the encoded message, or ERR_FULL if the
 * output buffer is too short.
 */

int encode_state(const struct state* input, uint8_t* output, int output_len);
//int encode_state(struct state* input, uint8_t* output, int output_len);

/*
 * Decodes a state structure from the input buffer.
 *
 * Returns:
 *   The number of processed bytes: if the message was sucesfully
 *       read from the input buffer.
 *   ERR_TOO_SHORT: if the buffer does not contain a complete state
 *       message. In such case decode_state should be called again after
 *       more data is appended to the buffer.
 *   ERR_PROTO: if the buffer does not contain a correct message. In
 *       such case a connection over which the buffer was read should be
 *       terminated, because it likely become corrupted.
 */
int decode_state(const uint8_t* input, int input_len, struct state* output);

/*
 * Encodes a state structure into an output buffer.
 *
 * The output buffer should have at least STATE_UPDATE_SIZE bytes.
 * Returns the length of the encoded message, or ERR_FULL if the
 * output buffer is too short.
 */
int encode_state_update(
    const struct state_update* input, uint8_t* output, int output_len);

/*
 * Decodes a state update structure from the input buffer. Should be
 * called when the input buffer has at least STATE_UPDATE_SIZE
 * bytes. In such case never returns ERR_TOO_SHORT (but can still
 * return ERR_PROTO).
 *
 * Returns:
 *   The number of processed bytes: if the message was sucesfully
 *       read from the input buffer.
 *   ERR_TOO_SHORT: if the buffer does not contain a complete state update
 *       message. In such case decode_state_update should be called again
 *       after more data is appended to the buffer.
 *   ERR_PROTO: if the buffer does not contain a correct message. In
 *       such case a connection over which the buffer was read should be
 *       terminated, because it likely become corrupted.
 */
int decode_state_update(
    const uint8_t* input, int input_len, struct state_update* output);

/*
 * lat and lng are zero terminated strings.
 * Lattitude has   XXXX.YYYYS or XXXX.YYYYYN format.
 * Longituted has XXXXX.YYYYW or XXXX.YYYYYE format.
 * Returns -1 if input strings are invalid, 0 otherwise.
 */
int compress_position(const char* lat, const char* lng,
                      struct position* position);
					  
					  
					  
					  
					  
					  
//Prototypes for control.c
void Set_SleepMode(void);
void PrintStatus(void);
void control(int);

enum fsm_event_t getNextEvent(void);

//state functions must return a enum fsm type
enum fsm_state_t available_Timeout(void);
enum fsm_state_t available_Server(void);
enum fsm_state_t available_Key_Reserve(void);
enum fsm_state_t available_Default(void);

enum fsm_state_t acntConfirm_Timeout(void);
enum fsm_state_t acntConfirm_Server(void);
enum fsm_state_t acntConfirm_Default(void);

enum fsm_state_t pinWait_Timeout(void);
enum fsm_state_t pinWait_Server(void);
enum fsm_state_t pinWait_Key_Pin(void);
enum fsm_state_t pinWait_Default(void);

enum fsm_state_t unlockWait_Timeout(void);
enum fsm_state_t unlockWait_Unlock(void);
enum fsm_state_t unlockWait_Key_Repair(void);
enum fsm_state_t unlockWait_Default(void);

enum fsm_state_t ride_Lock(void);
enum fsm_state_t ride_Key_Hold(void);
enum fsm_state_t ride_Key_Repair(void);
enum fsm_state_t ride_Default(void);

enum fsm_state_t holdWait_Hold(void);
enum fsm_state_t holdWait_Lock(void);
enum fsm_state_t holdWait_Default(void);

enum fsm_state_t repairWait_Repair(void);
enum fsm_state_t repairWait_Lock(void);
enum fsm_state_t repairWait_Default(void);

enum fsm_state_t hold_Timeout(void);
enum fsm_state_t hold_Key_Pin(void);
enum fsm_state_t hold_Server(void);
enum fsm_state_t hold_Default(void);

enum fsm_state_t broken_Timeout(void);
enum fsm_state_t broken_Server(void);
enum fsm_state_t broken_Default(void);



#ifdef NEED_POSITION_DECOMPRESSION

// This is needed only on the server and the client probably doesn't
// support doubles well.
void decompress_position(const struct position* p, double* latitude, double* longitude);

#endif

#ifdef __cplusplus

	}	
	
#endif

#endif /* MC_MESSAGE_H_ */
