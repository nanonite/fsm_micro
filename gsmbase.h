/*AVR SOURCE FILES FOR GSM,SERIAL FUNCTIONALITY                         *
*                     Copyright (C) 2010  Justin Downs of GRounND LAB	*
*                     www.GroundLab.cc     1%                          	*
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


#ifndef GSM
#define GSM
//Includes from http://www.nongnu.org/avr-libc/
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
//Local
#include "serial.h"
#include "ioHelper.h"

#define DEBUGB			//Undefine to stop debug on this file

class GSMbase{

	protected:
//!!CHANGE HERE FOR SERIAL AND TIMER COMPATIBILITY
typedef SerialPort Serial;      // change here to support your serial friend
				// Serial needs a read, write, flush, available functions  
	Serial& telitPort; 	// serial object
	uint32_t (*millis)();	// millis func pointer **NEED A FUNCTION that returns MILLIS**
	Serial* DebugPort;	// pointer so it can default to null


	char*  fullData;	// full telit responce data, filled by catchTelitData
	char*  parsedData; 	// parsed responce
	public:
	inline const char* const getFullData(){return fullData;}	
	inline const char* const getParsedData(){return parsedData;}

	GSMbase(Serial& ,uint32_t(*FP)(), Serial* = NULL);	// Call Once for best results
	
	//Handel Telit Data
	virtual const char* const catchTelitData
	(uint32_t = 180000, bool = false, uint16_t= 300, uint32_t = 60);//Default time out for +COPS is 180 seconds
	//time out	   //quickCheck	 //datasize	//baudDelay
	virtual const char* const parseData(const char* const,const char*,const char*);	//Parses string see below
	virtual const char* const parseSplit(const char* const,const char*,uint16_t);	//Splits string see below
	virtual bool parseFind(const char* const, const char*);		//returns true if it finds a string 

	//Talking to Telit	
	virtual bool init(uint16_t = 100);	//Used to init Telit to right settings, code doesn't work if not used.
	virtual void sendATCommand(const char*);			//Sends command in the clear
	virtual const char* const sendRecQuickATCommand(const char*);	//Used to send/get reply for a OK reply
	const char* const sendRecQuickATCommand_ATNT(const char* theMessage); //using to debug AT&T
	virtual const char* const sendRecATCommand(const char*);	//Main function that gets Telit reply
	virtual const char* const sendRecATCommandParse(const char*, 
	const char* _start, const char* _end);		//Sends AT command parses reply, between _start _end
	virtual const char* const sendRecATCommandSplit(const char*,
	const char* _delimiters, uint16_t _field);//Sends AT command splits data according to delimeter

	//Network calls 2G (*not HAYNES COMPATIBLE)
	bool checkCREG();			//Gets registration status 
	const char* const checkCOPS();		//Gets network availability and current registration
	const char* const checkGSN();				//Gets serial number
	uint16_t checkCSQ();					//Gets signal quality
	const char* const checkMONI();	//*gets info for surronding availble cell towers
	const char* const getMyNumCNUM(); //gets tel number of current mod, if stored on SIM 
	const char* const getIMSI();	//gets IMSI number
};

//////////////////////////////////////////////////////////////////////NETWORK STATUS FUNCS*


//Gets registration status (true REGISTERED, false NOT)
inline bool GSMbase::checkCREG(){
//RETURNS: +CREG: 0,1  OK
	if ( (sendRecATCommandSplit("AT+CREG?",":,",2)[0]) == '1') return true;
return false;
}
//Gets network availability and current registration
inline const char* const GSMbase::checkCOPS(){
//RETURNS: +COPS: (2,"Cingular",,"310410"),(1,"T-Mobile",,"310260"),,(0-4),(0,2) OK
return sendRecATCommandParse("AT+COPS=?","1,\"","\"");
}
//Gets serial number
inline const char* const GSMbase::checkGSN(){
//RETURNS: 0000646714 OK
return sendRecATCommandParse("AT+GSN","\r\n","\r\n");
}

/*CONVERT CSQ INTO DBM WITH:
 *dBm = (-113 +n)*2, where n is the CSQ.
 * test signal strength:
 * average phone handset
 * GSM monitor 
 *
 * RX (from - to)                  amount of bars
 * From -105 to -100 dBm               0   
 * from -100 to -95 dBm                1   
 * from -95 to -90 dBm                 2   
 * from -90 to -85 dBm                 3   
 * more than -85 dBm                   4   
 * */
//Gets signal quality
inline uint16_t GSMbase::checkCSQ(){
//RETURNS: +CSQ: 7,0 OK
return atoi(sendRecATCommandSplit("AT+CSQ",":,",1));
}
	
//MONI *TELIT ONLY* gets info for all surronding
//availble cell towers,AT#MONI=0 then AT#MONI
//gets info for the one you're connected to. 
//Use AT#MONI?? for list of all towers.
//AT#MONI=7 then AT#MONI to get all towers info.
inline const char* const GSMbase::checkMONI(){
//RETURNS: #MONI: Cingular BSIC:51 RxQual:0 LAC:8AD3 
//Id:6014 ARFCN:148 PWR:-99dbm TA:0 OK
return sendRecATCommandParse("AT#MONI","\r\n","\r\n");
}

//AT+CNUM returns the phone # number of the device, if stored on SIM 
inline const char* const GSMbase::getMyNumCNUM(){
//RETURNS: +CNUM: "name","3449709999",129
return  sendRecATCommandSplit("AT+CNUM",",",1);
}

inline const char* const GSMbase::getIMSI(){
//Gets your IMSI Number 
return sendRecATCommand("AT+CIMI");
}

//////////////////////////////////////////////////////////////////////NETWORK STATUS FUNCS*
#endif
