
/*AVR SOURCE FILES FOR GSM,SERIAL FUNCTIONALITY                         *
*                     Copyright (C) 2012  Justin Downs of IEF R&D	*
*                     www.IEFRD.com     1%                          	*
*                                                                 	*
* This program is free software: you can redistribute it and/or modify 	*
* it under the terms of the GNU General Public License as published by 	*
* the Free Software Foundation, either version 3 of the License, or    	*
* at your option) any later version.                                   	*
*                                                                      	*
* This program is distributed in the hope that it will be useful,      	*
* but WITHOUT ANY WARRANTY; without even the implied warranty of       	*
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        	*
* GNU General Public License for more details.                         	*
*                                                                      	*
* You should have received a copy of the GNU Lesser General Public      *
* License with this program.If not, see <http://www.gnu.org/licenses/>. * 
************************************************************************/


//EXAMPLE OBJECT instance
//#include gsmTelitHardware.h
//uint32_t (*millis)() = &yourMillis;   // millis func pointer **NEED A FUNCTION that returns MILLIS**
//gsmTelitHW (telitPort,millis,DebugPort):
//END CHANGES

#ifndef GSMMASTER
#define GSMMASTER
#include "gsmSMS.h"
#include "gsmGPRS.h"
#include "gsmTelitHardware.h"

/*
#define KORE_APN "c1\.korem2m\.com"
#define RACO_APN "m2m\.t-mobile\.com"
#define JASPER_APN "m2m\.com\.attz"

#define RACO_MSG_CENTER "+12063130004"
#define ATT_MSG_CENTER "+13123149810"

#define CSTT_APN JASPER_APN
#define TXT_MSG_CENTER ATT_MSG_CENTER
#define SERVER_DOMAIN "www\.buswave\.com" 
*/


class gsmMASTER : public gsmSMS, public gsmGPRS, public gsmTelitHW 
{
	private:
	char* const CNTXID;//  = "2"; //context ID
	char* const SSOCKET;// = "1";	//Socket to send info on
	char* const RSOCKET;// = "2";	//Socket to receive on	
	char* const RIP;	// = "127.0.0.1"; //IP to listen to
	char* const RMASK; // = "255.255.255.0";
	char* const SIP;	// = "127.0.0.1"; //IP to send to
	char* const RPORT; // = "80";	//listen port or Telit's port
	char* const SPORT; // = "80";	//send port or server's port
	char* const USER; //   = "user";	//USER NAME
	char* const PASS; //   = "password";	//PASSWORD
	char* const APN; //	= "wap.cingular";//APN
	char* const NUM;	//cell phone number to send to
	//char* const used for variables that you only want modified during class instantiation
	
	char telitIP[16];		// telit IP adress

	public:
	
	char* pinSMS; //* PIN_LEN
	char* accountSMS; //*ACCOUNT_LEN
	
	gsmMASTER(Serial& _telit, uint32_t (*_millis)(), Serial* _debug);	
	bool init(uint16_t = 100);						//Telit init sequence
	bool bringUpTelit();							//Turns telit on and inits it
	void sDelay(uint32_t _time);					
	bool telitAlive();							//Telit in command mode.
	bool telitAlive_ATNT();						//used to debug AT&T
	bool waitForNetwork();							//waits to register
	bool networkInit(const char* const _APN);				//sets up network (no connect)
	bool networkReg(const char* const _USER=NULL,
	const char* const _PASS=NULL);						//get IP
	bool networkUnReg();							//release IP
	bool signalOk();							//Checks how many bars we have
	
	//sends info out TCP connection
	/*
	bool sendTCPIP(const char* socket, const char* const data, 	uint8_t byte_length,
	const char* const _URL,const char* const PORT);			
	*/
	
	bool sendTCPIP(const char* socket, const uint8_t* const data, 	uint8_t byte_length,
	const char* const _URL,const char* const PORT);	


	const char* const SocketHandler(const char*, const char*);//socket listen (call after each network op)
	const char* const telitEventHandler();				//keeps tabs on server conntacting telit
	const int telitSMSHandler();				//check out if any SMS has been sent
	void checkSMS ();			//checks for SMS messages
	const char* const getIP();
	const char* const getNUM();
	const char* const getSSocket();
	const char* const getSIP();
	const char* const getSPort();
	const char* const getRSocket();
	const char* const getRIP();
	const char* const getRPort();
	const char* const getAPN();
	const char* const getPinSMS();
	const char* const getAccountSMS();
	void setPinSMS(char*);
	void setAccountSMS(const char*);
	
	///HELPER FUNCS CAN TAKE OUT
	void msgHandler(const char* const theMessage);
	void tester();


};

inline const char* const gsmMASTER::getIP(){
return telitIP;
}

inline const char* const gsmMASTER::getNUM(){
return NUM;
}

inline const char* const gsmMASTER::getAPN(){
return APN;
}

inline const char* const gsmMASTER::getSSocket(){
return SSOCKET;
}
inline const char* const gsmMASTER::getSIP(){
	return SIP;
}
inline const char* const gsmMASTER::getSPort(){
	return SPORT;
}

inline const char* const gsmMASTER::getRSocket(){
	return RSOCKET;
}
inline const char* const gsmMASTER::getRIP(){
	return RIP;
}
inline const char* const gsmMASTER::getRPort(){
	return RPORT;
}
inline const char* const gsmMASTER::getPinSMS(){
	return pinSMS;
}
inline const char* const gsmMASTER::getAccountSMS(){
	return accountSMS;
}

inline void gsmMASTER::setPinSMS(char* c){
	pinSMS = c;
}

inline void gsmMASTER::setAccountSMS(const char* c){
		for(int i = 0 ; i <6 ; i++){
			accountSMS[i] = c[i];
		}
}

#endif
