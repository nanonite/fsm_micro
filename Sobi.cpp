
//#includes for C headers
extern "C" {
#include "SoR_Utils.h"

};
//#include for C++ headers
#include "serial.h"
#include "mc_message.h"

#include <string.h>

#include "gsmMaster.h"

int main(void)
{
		
	
	configure_ports(); 
	
	
	delay_ms(200); //for ON/OFF DEBUG

	a2dInit();	
	
	TWIInit();	
		
	LockInit();	
	
	
	
	//Serial 0 -> Telit (RX) and Telit (TX)
	//Janis baud rate = 115.2 kbps
	//no parity , 8 bits , 1 stop bit
	//handshaking??
	SerialAVR.USART_Init(2); //name space issue with Serial, replaced SerialAVR
	
	//Serial 1 -> GPS (RX) and DEBUG (TX)
	//u16 bauddiv = ((F_CPU+(baudrate*8L))/(baudrate*16L)-1);
	//set baud rate to 9600 baud for RS232 Terminal
	//no parity , 8bits , 1 stop bit
	Serial1.USART_Init(1);
	
	milicount = 0; //reset counter number of miliseconds
	
	EnableTimer(); 
	
	KeypadInit();	
	//reset variables
	Clear_KeyArray();
	Clear_LongKeyArray();
	
	ledInit(); 
	
	uint32_t (*millis)();
	millis = &getMillis;
	

	SerialPort* DebugPort = &Serial1;
	
	sei(); 	//enable global interrupts
	
	
	gsmMASTER TelitObject(SerialAVR,millis,DebugPort);
	

	
		/*		
		char temp_pin[] = "123";
		strncpy( input.pin, temp_pin, sizeof(input.pin)-1 );
		*/
		
	    struct position p;
		   
		struct state input;
		
		for(int i  = 0 ; i < 3 ; i++){
					p.compressed_lat[i] = 3;
				}
				for(int i = 0 ; i < 4 ; i++){
					p.compressed_lng[i] = 1;
				}
				p.timestamp = 12;
				
			
		//mac adr: 310260762276130
		//SIM : 89011704258001701824
		
		
		
		//bike_id is just a convention, one first response , populates the MAC address field
		
		//char temp_bike_id[] = "8901170425800170"; //tried
		//char temp_bike_id[] = "1704258001701824"; //try
		//char temp_bike_id[] = "bike01";
		
		
		
		
		char temp_bike_id[] = "310260762276130"; //tried  <<really looks right
		strncpy( input.bike_id, temp_bike_id, sizeof(input.bike_id)-1 );
		
		//account appends 2??
		//char temp_account[]= "987654321";
		char temp_account[]=   "000000011"; //ryans account
		strncpy( input.account, temp_account, sizeof(input.account)-1 );
	
		char temp_pin[] = "1234";
		strncpy( input.pin, temp_pin, PIN_LEN-1); //<<look into <<<Serial1.write(temp_pin);
		
		/*
		input.owner_state = BIKE_AVAILABLE;
		input.lock_state = BIKE_LOCKED;
		input.repair_state = BIKE_WORKING;
		
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

		*/
		
		// this works
		input.owner_state = 1;
		input.lock_state = 1;
		input.repair_state = 1;
		
		input.timestamp= 0xABCDEF01;	//use different string timestamp
		input.battery_level = 245;
		input.positions_count = 1;
		
		input.positions[0] = p;
		
	
		
		//gps debug <--make a function to get lat and long and directioin
		/*
		const char* latitude = NULL;	
		const char* longitude = NULL;		
		TelitObject.bringUpTelit();
		TelitObject.powerGPS_GPSP(gsmTelitHW::GPSON); //sometimes it turns on/ sometimes not
		delay_ms(1000);
		
		while(longitude == NULL){
			longitude =  TelitObject.sendRecATCommandSplit("AT$GPSACP" , "," ,  2); 
		}
		
		while(latitude == NULL){
			latitude = TelitObject.sendRecATCommandSplit("AT$GPSACP" , "," ,  1);
		}
		
		const char* gpsString = parseSplit( getGPS_GPSACP();,",",1);
		
					
		Serial1.write("Latitude :");
		Serial1.write(latitude);
		Serial1.write("\nLongitude :");
		Serial1.write(longitude);
		//AT$GPSACP RETURNS: $GPSACP: <UTC>,<latitude>,<longitude>,<hdop>,<altitude>,
		//<fix>,<cog>,<spkm>,<spkn>,<date>,<nsat>
		*/
		
		/*
		while(1){
			talk();	
		}
			*/		
		
			
		const char* rxBuffr;
		uint8_t* buffr;
		int length;
		const struct state* inputPtr = &input;	
		char bytenumber[16];
		buffr = (uint8_t*) malloc(MAX_STATE_SIZE);
		length = encode_state(inputPtr, buffr, MAX_STATE_SIZE); 
		
		// to read output of encode state
		sprintf(bytenumber,"%d",length);
		Serial1.write("The encoded state is : ");
		Serial1.write(bytenumber);
		Serial1.write(" number of bytes\n");
		
		
			
		SerialAVR.flush();
		
		TelitObject.bringUpTelit();
		TelitObject.deletMessagesCMGD("4"); //reset condition, delete all messages
		TelitObject.networkInit(TelitObject.getAPN());
		TelitObject.networkReg();
		
		//sets socket into listing state,at the point context is set
		//TelitObject.SocketHandler(TelitObject.getRSocket(), TelitObject.getRPort());	
		
		//get GPS data, format and populate struct
		//setup SMS receive
		//is this the poller???
		
		//depends on how proxy is configured
		TelitObject.sendTCPIP(TelitObject.getSSocket(),buffr,length, TelitObject.getSIP(), TelitObject.getSPort());
			
			
		
		//listen for text
		const char* memTemp1 = NULL;
		//char* memTemp2 = NULL;
		//char* memTemp3 = NULL;
		char deadStr = NULL;
		int tempStr;
		unsigned char flagSMS = 0;
		
			
			//if any return > 0 , check it
			/*
			memTemp2 = TelitObject.getNumMesInMemCPMS(3);
			memTemp3 = TelitObject.getNumMesInMemCPMS(5);
			sprintf((char*)memTemp1 , "%d", tempStr);
		
			sprintf(memTemp2 , "%d", tempStr);
			if(tempStr > 0) break;		
			sprintf(memTemp3 , "%d", tempStr);
			if(tempStr > 0) break;
			*/
			//waiting for an SMS to be sent by the proxy
		while(!TelitObject.telitSMSHandler()){
			
		}		
		
	
		DebugPort->write(TelitObject.getPinSMS());
		DebugPort->write(TelitObject.getAccountSMS());
		
		while(1){
			
			talk();
		}		

		

		//listen for TCP/IP
		/*
		while(1){
			rxBuffr = TelitObject.telitEventHandler();
			if(rxBuffr != NULL){
				DebugPort->write("Received Package: ");
				DebugPort->write(rxBuffr);
				break;
			}
		}
		*/
	
	//TelitObject.sendNoSaveCMGS("3602599543","Test"); //send SMS
	
	//Engage_Lock();
	
	Serial1.write("Reset...\n"); 
	
    while(1)
    {
	
		//control(0);
		talk();
	}		
	
}

void talk(){

	while (SerialAVR.available()>0){
		Serial1.write(SerialAVR.read());
	}
	while (Serial1.available()>0){
		SerialAVR.write(Serial1.read());
	}
}

/*
DDR PORT PUD I/O Pull-UP State
0	0	 X	Input No	Tri-state (Hi-Z)
0	1	 0	Input Yes	Pxn will source current if ext. pulled low.
0	1	 1	Input No	Tri-state (Hi-Z)
1	0	 X	OutputNo	Output Low (Sink)
1	1	 X	OutputNo	Output High (Source)

*/

void configure_ports(void) {
	//DIGITAL PORTS
	DDRC = 0b11111111;
	//       ||||||||
	//       |||||||\___0: scl
	//       ||||||\____1: sda
	//       |||||\_____2: L2
	//       ||||\______3: L1
	//       |||\_______4: L3
	//       ||\________5: L4
	//       |\_________6: L5
	//       \__________7: L6

	PORTC = 0b00000000; // all should be low
	
	DDRA = 0b01001100;
	//       ||||||||
	//       |||||||\___0: OUT_LOCK
	//       ||||||\____1: MIDLOCK
	//       |||||\_____2: IN_2 (out)
	//       ||||\______3: IN_1 (out)
	//       |||\_______4:
	//       ||\________5: SOLAR_AN
	//       |\_________6:
	//       \__________7: KEY_INT
	PORT_ON(PORTA,7); // enable pullup keyint
	PORT_ON(PORTA,0); // enable pullup Outlock
	PORT_ON(PORTA,1); // enable pullup midlock

	//PORTB reserved for programmer (use programmer pins if you know what you are doing)
	
	DDRB|=_BV(4);//set PORTB pin4 to zero as output   -- SS
	DDRB|=_BV(3);//set PORTB pin3 to one as output-- BEEP
	
	DDRB&=~_BV(2);//set PORTB pin2 to zero as input	  -- BAT_INT
	DDRB|=_BV(1);//set PORTB pin1 to one as output	  -- GPS RESET
	
	//DDRB |= 0x01; //ON/OFF PORT //output
	DDRB &= 0xFE;
	PORTB &= 0xFE; //ON/OFF PORT HI Z input ,for GSM module
	
	PORT_OFF(PORTB,3); // Speaker pin should be low
	
	
	DDRD = 0b00111010;
	//       ||||||||
	//       |||||||\___0: Cell rx	(Input)
	//       ||||||\____1: cell tx  (Output)
	//       |||||\_____2: DEBUG RX (input)
	//       ||||\______3: DEBUG TX (Output)
	//       |||\_______4:
	//       ||\________5:
	//       |\_________6: DRDY
	//       \__________7: INT1
	
	
//	DDRD &= 0x3F;
	
	
	
	/*
	PORT_OFF(PORTD,0); // needed?
	PORT_OFF(PORTD,1);
	*/
	
	PORT_ON(PORTD,7); // maybe low? for interrupt on keypad controller?
	
	//MCUCR default to PUR active when PORT set
	
}


//************DELAY FUNCTIONS************
//wait for X amount of cycles (234 cycles is about 1.003 milliseconds)//incorrect
//to calculate: 234/1.003*(time in milliseconds) = number of cycles
void delay_cycles(volatile unsigned long int cycles){
	//cycles=cycles;//doubled frequency but too lazy to change times
	//WHYNOT: while (cycles--);
	while(cycles > 0)
		cycles--;
	}

void delay_us(unsigned long int microseconds) {
	delay_cycles(microseconds*CYCLES_PER_US/30);
} 

void delay_ms(unsigned long int milliseconds) {
	int i;	
	for (i=0;i<1000;i++) {
		delay_cycles(milliseconds*CYCLES_PER_US/30);
	}
} 


uint32_t getMillis(void){
	
	return milicount;
}
		/*
	 //power cycles
	Serial1.write("Here we are");
	TelitObject.sendRecQuickATCommand("AT#SHDN");
	delay_ms(10000);
	Serial1.write("NO RESPONSES SENT?\n");
	TelitObject.sendRecQuickATCommand("AT");
	TelitObject.sendRecQuickATCommand("AT");
	TelitObject.sendRecQuickATCommand("AT");
	if(TelitObject.powerCycle()) //confirm off
		Serial1.write("POWER OFF");
	*/







/* 32 bit int print via DebugPort

char temp;
uint32_t what;

unsigned int   i1 , i2;
unsigned char  buf1[16];
unsigned char  buf2[16];

what = getMillis();
i1 = (int)(what);
i2 = (int)(what>>16);

sprintf(buf1, "%d", i1);
sprintf(buf2, "%d", i2);

Serial1.write(buf1);
Serial1.write("\r\n");
Serial1.write(buf2);
Serial1.write("\r\n");

delay_ms(100);

*/