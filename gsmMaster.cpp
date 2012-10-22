
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




#include "gsmMaster.h"

#define DEBUGT 



//gsmMASTER::CNTXID  = "2"; //context ID
/*
static const char* const SSOCKET = "1";	//Socket to send info on
static const char* const RSOCKET = "2";	//Socket to receive on	
static const char* const RIP	 = "127.0.0.1"; //IP to listen to
static const char* const RMASK   = "255.255.255.0";
static const char* const SIP	 = "127.0.0.1"; //IP to send to
static const char* const RPORT   = "80";	//listen port or Telit's port
static const char* const SPORT   = "80";	//send port or server's port
static const char* const USER   = NULL;	//USER NAME
static const char* const PASS   = NULL;	//PASSWORD
static const char* const APN	= "wap.cingular";//APN
*/


/*
#define		KORE_APN		"c1\.korem2m\.com"
#define		RACO_APN		"m2m\.t-mobile\.com"
#define		JASPER_APN		"m2m\.com\.attz"

#define     RACO_MSG_CENTER   "+12063130004"
#define     ATT_MSG_CENTER "+13123149810"

#define 	CSTT_APN		  JASPER_APN
#define 	TXT_MSG_CENTER	  ATT_MSG_CENTER
*/


//Proxy IP: 166.84.136.78
//port 443 is for t-mobile
//port 5674 is old encoding (eric's)
//port 5675 is new encoding (jan's)

//this should work for AT&T
//APN : m2m.com.attz
//user name : null / password : null

//space and case sensitive!!!

//AT&T Connection

//APN will be modified to point to an encrypted APN which uses internally encrypted VPN

gsmMASTER::gsmMASTER(Serial& _telit, uint32_t (*_millis)(), Serial* _debug)
: GSMbase(_telit, _millis, _debug), gsmSMS(_telit ,_millis, _debug),
gsmGPRS(_telit, _millis, _debug), gsmTelitHW(_telit, _millis, _debug),
CNTXID("2"),SSOCKET("1"),RSOCKET("2"),RIP("166.84.136.78"),RMASK("255.255.255.0"),
SIP("166.84.136.78"),RPORT("443"),SPORT("443"),USER(NULL),
PASS(NULL),APN("m2m\.com\.attz"),NUM(NULL),pinSMS(NULL),accountSMS(NULL)
{ }



//wap.cingular works with at&t

//T-Mobile Connection, //why not receive on the same socket??'

/*
gsmMASTER::gsmMASTER(Serial& _telit, uint32_t (*_millis)(), Serial* _debug)
: GSMbase(_telit, _millis, _debug), gsmSMS(_telit ,_millis, _debug), 
gsmGPRS(_telit, _millis, _debug), gsmTelitHW(_telit, _millis, _debug), 
CNTXID("2"),SSOCKET("1"),RSOCKET("2"),RIP("166.84.136.78"),RMASK("255.255.255.0"),
SIP("166.84.136.78"),RPORT("443"),SPORT("443"),USER("WAP@CINGULARGPRS.COM"),
PASS("CINGULAR1"),APN("wap.cingular"),NUM("3602599543")
{ }
*/





/*	//Eric's server via AT&T

gsmMASTER::gsmMASTER(Serial& _telit, uint32_t (*_millis)(), Serial* _debug)
: GSMbase(_telit, _millis, _debug), gsmSMS(_telit ,_millis, _debug),
gsmGPRS(_telit, _millis, _debug), gsmTelitHW(_telit, _millis, _debug),
CNTXID("2"),SSOCKET("1"),RSOCKET("2"),RIP("107.20.222.174"),RMASK("255.255.255.0"),
SIP("107.20.222.174"),RPORT("1035"),SPORT("1035"),USER(NULL),
PASS(NULL),APN("m2m.com.attz"),NUM("3473017780")
{ }
	
*/


bool gsmMASTER::init(uint16_t band){
	if(GSMbase::init( band) && gsmSMS::smsInit()) return 1;
return 0;	
}

////////////////////////////////////////////////////////////////////////////////DELAY
void gsmMASTER::sDelay(uint32_t _time){
	uint32_t startTime=millis();
	while((millis()-startTime) < _time);					// hang out a minute
}

///////////////////////////////////////////////////////////////////////////////Power on and init Telit
bool gsmMASTER::bringUpTelit(){

	#ifdef DEBUGT//DEBUG
	DebugPort->write("POWER ON\n");
	#endif	//DEBUG

	turnOn();		//Turn on GSM	
	//Init Telit settings , POLL FOR INIT TRUE?
	init();
	if (!waitForNetwork()) return 0;	//look for network
	sDelay(30000);		//Wait for CREG to fully register
return 1;
}



//////////////////////////////////////////////////////////////////////////Check for Telit
bool gsmMASTER::telitAlive(){
	// Send out AT to see if your in command mode
	// repete 4 times, true if good reply;
	//add delay
	
	for (int i=0; i<4; ++i){
		if (sendRecQuickATCommand("AT")) return 1; 	
	}	
return 0;
}
//TODO:REMOVE
//////////////////////////////////////////////////////////////////////////Check for Telit
bool gsmMASTER::telitAlive_ATNT(){
	
	for(int i=0 ; i<4 ;++i)
	{
		if(sendRecQuickATCommand_ATNT("AT")) return 1;
	}
	return 0; //else?
}

//////////////////////////////////////////////////////////////////////////Network Check
bool gsmMASTER::waitForNetwork(){
//CHECK CREG (NETWORK REGISTRATION)
	uint16_t timeOut=0;
	while (!checkCREG()){
		if(timeOut++>12) return 0;		//no network return 0
	
		#ifdef DEBUGT//DEBUG
		DebugPort->write("LOOKING FOR NETWORK");					
		#endif//DEBUG

		uint32_t startTime=millis();
		while((millis()-startTime) < 5000);	// hang out a minute
	}

	#ifdef DEBUGT//DEBUG
	DebugPort->write("REGISTERED");								
	#endif//DEBUG
return true;	
}

bool gsmMASTER::signalOk(){
	//Use later to determine SMS or GPRS mode
	//dBm = (-113 +(n*2)), //where n is the CSQ.
	int16_t csq = (int16_t)(-113 + (checkCSQ()*2));
	bool signalGood = (csq > -95 && csq < -85 ? 1 : 0) ;	//1 bar or more 
	#ifdef DEBUGT//DEBUG
	char dbm[6];
	DebugPort->write ("Signal strength, dbm: ");						
	DebugPort->write((itoa(csq,dbm,10)));
	#endif//DEBUG
return signalGood;
}




//////////////////////////////////////////////////////////////////////////network inits
//Get IP allocated
//APN -> "wap.cingular", USER-> "WAP@CINGULARGPRS.COM",PASS -> "CINGULAR1"
bool gsmMASTER::networkInit(const char* const _APN){
	//SETS the context number associated with a PDP protocal "IP"/"PPP" and APN number.
	//context "2" now has these settings (NOTE don't use "0" it is reserved for SMS)

	if (!setApnCGDCONT(CNTXID,"IP", _APN)) return 0;
	#ifdef DEBUGT//DEBUG
	DebugPort->write("CGDCONT(APN)\n");
	#endif	//DEBUG

	//SETS the TCP/IP stack
	//socket connection ID is now linked to context ID data, with default timeouts TCP/IP
	if (!setTcpIpStackSCFG(SSOCKET,CNTXID)) return 0;	//Set socket to send on
	DebugPort->write("SCFG to send\n");
	if (!setTcpIpStackSCFG(RSOCKET,CNTXID)) return 0;	//Listen socket
	DebugPort->write("SCFG to receive\n");
	//set firewall , neccessary?
	/*
	firewallFRWL("1",RIP,RMASK);	//set the firewall settings for incoming connection
	DebugPort->write("FRWL\n");
	*/
	socketConSCFGEXT(RSOCKET,"0");	//set SRING notification
	
	//socketConSCFGEXT(RSOCKET , "0" ,"0" ,"0"  ,"1"); //todo: test auto listen
	#ifdef DEBUGT//DEBUG
	DebugPort->write("SCFGEXT\n");
	#endif	//DEBUG


return 1;	//return true if good init
}

//REGISTERS with the network, receives IP address and network resources.
//	connect the specified context ID to the network. 
//	1 gets network resources 0 disconnects from network and frees resources.
//const char* const _USER,const char* const _PASS
bool gsmMASTER::networkReg(const char* const _USER,const char* const _PASS){
	
	if(!setContextSGACT(CNTXID,"1",_USER,_PASS)) return 0;
	strcpy(telitIP,checkipCGPADDR(CNTXID)); //get IP and store it.

	#ifdef DEBUGT//DEBUG
	DebugPort->write("SGACT set\n");
	#endif//DEBUG
return 1;
}

	
	//log off network and free up IP address
bool gsmMASTER::networkUnReg(){
	if(!setContextSGACT(CNTXID,"0")) return 0;
return 1;
}

//Start listing to socket

///////////////////////////////////////////////////////////////////////////////send TCP IP


bool gsmMASTER::sendTCPIP(const char* socket, const uint8_t* const data,
 uint8_t length , const char* const _URL,const char* const PORT){
	
	if( socketDialSD(socket,"0",PORT,_URL)){
		//RETURNS: CONNECT
		//Constructs and send a get request on open socket

		//sendRecFunc();		//talk with server here, i.e send fake packet
		
		telitPort.write(data,length);		//send out data
		//AT#SH closes the socket connection, no data in or out
		uint64_t startTime; 
		startTime = millis();
		while ((millis() - startTime) < 2000); 		// block 5 seconds
		
		//how long is socket going to be open for? determined by the
		//<<<intiates close on the socket from the telit
		
		//pings server, dump data,
		
		suspendSocket(); //set on Socket ID = 0 , why? , normally the server initiates the server doing this!
		
		
		
		#ifdef DEBUGT
		if(telitAlive()) DebugPort->write("We have control\n");		//Test if we have command back
		//if(telitAlive_ATNT()) DebugPort->write("We have control\n");	//ATNT Debug
		else DebugPort->write("Telits hung");
		#endif
		
		//	closeSocketSH(socket); //close on Socket ID = 1  , ?? why
		
		
	}else{
		#ifdef DEBUGT
		//DEBUG
		DebugPort->write("SGACT couldn't get socket");
		//DEBUG
		#endif
		return 0;
	}
	
	return 1;
	
}	


//handles socket put it in a listing state
const char* const gsmMASTER:: SocketHandler(const char* const _socket, const char* const _port){
	const char* data= NULL;
	//GRABS----------\ The state
	//#SS:<connId>,<state>,<locIP>,<locPort>,<remIP>,<remPort>
	const char* theState = parseSplit(socketStatusSS(_socket),",",1);
	
	uint64_t startTime;
	startTime = millis();
	while ((millis() - startTime) < 2000); 		// block 2 seconds	
	
	#ifdef DEBUGT//DEBUG
	DebugPort->write("\nState: ");
	DebugPort->write(theState[0]);
	DebugPort->write("\n");
	#endif//DEBUG

	//State 0->Closed, 1->Acive, 2->Suspended, 3-> pending
	//4-> Listning, 5-> Incoming connection.
	switch (theState[0]){
		
		case '0': socketListenSL(_socket, "1",_port);//set it to listen SL
			
			break;
		case '1': //leave it be
			break;
		case '2': socketListenSL(_socket, "1",_port);
			break;
		case '3': resumeSocketSO(_socket);//ANSWER SO
			data = catchTelitData(3000,0,300,1000);	//give a one second wait to make sure 
			sDelay(1000);		//let it breath //transmision is done
			suspendSocket();
			socketListenSL(_socket, "1",_port);
			break;
		case '4': //leave it be
			break;
		case '5': 
			DebugPort->write("PROXY PACKET RECEIVED\n");
			socketAcceptSA(_port);//ANSWER SA
			data = catchTelitData(3000,0,300,1000);	//give a one second wait to make sure
			sDelay(1000);		//let it breath
			suspendSocket();
			socketListenSL(_socket, "1",_port);
			break;
	}			
return data;	//returns NULL if error happend use with if()
}

//Listens for SRING reply
//LISTNER SRING
const char* const gsmMASTER::telitEventHandler(){
	const char* data= NULL;
	//add else if for parse find for SMS replies?
	//We have a message
	if(telitPort.available()>0){ // just polls serial buffer
		if(parseFind(catchTelitData(2000,0),"SRING:")){
			socketAcceptSA(RSOCKET);//ANSWER SA
			data = catchTelitData(3000,0,300,1000);	//give a one second wait to make sure
			sDelay(1000);		//let it breathe
			suspendSocket();
			socketListenSL(RSOCKET, "1",RPORT);
		}else{//unhandled serial event
			sDelay(1000);		//wait
			telitPort.flush();	//flush
		}
	}
return data;	//either incoming data or NULL
}

//////////////////////////////////////////////////////////////////////////SMS

void gsmMASTER::checkSMS(){
	//get message list
	const char* list = checkCMGDList();	
	char * num;                                    // you have to use a local scope char array,    
	char deadStr[strlen(list)+1];             	// or you get a bad memory leak.
	strcpy(deadStr, list);                     	// don't change this unless you are careful.

	#ifdef DEBUGT//DEBUG
	DebugPort->write("\nfrom Dead string: ");
	DebugPort->write(deadStr);
	DebugPort->write("\n");
	#endif //DEBUG

	char delimeters[]={","};
	num = strtok (deadStr,delimeters);             	// split here first
	for (; num!= '\0'; num = strtok(NULL, delimeters) ){

			#ifdef DEBUGT//DEBUG
			DebugPort->write("\nlist number: ");
			DebugPort->write(num);
			#endif //DEBUG
			
			const char* _message = readMessageCMGR(num);	//get message
			msgHandler(_message);				//Do somthing with it
			//delay
			uint32_t startTime=millis();
			while((millis()-startTime) < 1000);		// hang out a minute
			deletMessagesCMGD(num);				//delete message

			#ifdef DEBUGT//DEBUG
			DebugPort->write("cell message:");
			DebugPort->write(_message);
			DebugPort->write("\n");
			#endif//DEBUG
	}	
}


//assumptions is that as soon as one of the SMS are received it will be immediately deleted
const int gsmMASTER::telitSMSHandler(){
	//TODO:Test
	const char* smsTest;
	
	smsTest = getNumMesInMemCPMS(1); // the first memory location in the SMS
	if(!strcmp(smsTest, "0")){
		checkSMS();
		return TRUE;
	}		
	else
		return FALSE;
	
	
	//TODO:Test
	//will get CMGS list return null if no messages are stored??? , I remeber a "<>" response
	/*
	const char* smsTest;
	smsTest = gsmSMS::checkCMGDList(); // <> or <1,2>
	if(smsTest == NULL){ //won't work 
			return 0;
	}
	else{
			return 1;
	}
	*/
	//maybe put this function inside the gsmSMS CLASS LIBRARY
}


//////////////////////////////////////////////////////////////////////////HELPER FUNCS
void gsmMASTER::msgHandler(const char* const theMessage){
	 //make sure data is relevant???
	 //need to convert the const char* const  to char*
	 char* send_char;
	 const char* temp;
	 temp = parseSplit(theMessage, "," , 0);
	 send_char = const_cast<char*> (temp);
	setPinSMS(send_char);
	setAccountSMS(parseSplit(theMessage, "," , 1));
	
}

void gsmMASTER::tester(){
	char data[]="test send";
		bringUpTelit();		//turn on Telit init setting
		networkInit(APN);	//configure network and sockets		
		networkReg();		//register with Gatway get IP
		SocketHandler(RSOCKET, RPORT);	//sets socket into listing state
		
		//sendTCPIP(SSOCKET, data,SIP, SPORT);
					
		checkSMS();
		
		networkUnReg();		

}

//////////////////////////////////////////////////////////////////////////TALKING
/*void gsmMASTER::talk(){

	while (Serial3.available()>0){
		DebugPort->write(Serial3.read());
	}
	while (DebugPort.available()>0){
		Serial3.write(DebugPort.read());
	}
}

//////////////////////////////////////////////////////////////////////////TALKING
void gsmMASTER::talkReply(){
	while (DebugPort.available()>0){
		Serial3.write(DebugPort.read());
	}
	if (Serial3.available()>0){
		catchTelitData(5000,false,600,60);
		DebugPort->write(GsmMaster.getFullData());
	}
}
*/
