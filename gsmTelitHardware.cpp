
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





#include "gsmTelitHardware.h"


gsmTelitHW::gsmTelitHW (Serial& telit, uint32_t(*millis)(), Serial* debug):
GSMbase(telit, millis, debug)
{
}

int gsmTelitHW::powerCycle(){
	
	telitPort.write("AT\r");
	if(!catchTelitData(1000,true,20,2000)) return 1;		// is the module turned off?
	//HW TURN OFF
	setOutput(OnOffPortD,OnOffPin); 				// set direction register pin
	uint64_t startTime;
	while(1){
		outputLow(OnOffPort,OnOffPin);			// bring pin low [AOG]
		startTime = millis();
		while ((millis() - startTime) < 5000); 		// block 2 seconds
	
		outputHigh(OnOffPort,OnOffPin); 			// bring pin high [AOG]
		//setInput(OnOffPortD,OnOffPin);	
		startTime = millis();
		while ((millis() - startTime) < 10000);		// block 1 seconds
		//setInput(OnOffPortD,OnOffPin);
		telitPort.write("AT\r");
		if(!catchTelitData(1000,true,20,2000)) return 1;
		DebugPort->write("Stuck turning OFF");
	}		
	
}

//////////////////////////////////////////////////////////////////////HARDWARE FUNCS
bool gsmTelitHW::turnOn(){
	telitPort.flush();
	telitPort.write("AT\r");
	if(catchTelitData(1000,true,20,2000)) return 1;		// the power is already on
	
	setOutput(OnOffPortD,OnOffPin); 				// set direction register pin
	uint64_t startTime; 
	while(1){ 

		outputLow(OnOffPort,OnOffPin);			// bring pin low [AOG]
	 	startTime = millis();		
		while ((millis() - startTime) < 5000); 		// block 5 seconds
				
		outputHigh(OnOffPort,OnOffPin); 			// bring pin high [AOG]
		//no need to reset to input direction
		startTime = millis();		
		while ((millis() - startTime) < 10000);		// block 10 seconds
		
		telitPort.flush();
		telitPort.write("AT\r");
		/* at&t sim case */
		startTime = millis();
		while ((millis() - startTime) < 500);		// block 1 seconds
		
		if(catchTelitData(1000,true,20,2000)) return 1;		// the power is already on --- this works with T-Mobile SIM
		
		//if(catchTelitData(5000,true,20,5000)) return 1;		//at&T 

		DebugPort->write("stuck in ON\n");
		}
	return 1;							//should never get here
}


//Turns off Telit
bool gsmTelitHW::turnOff(){
	uint64_t startTime;
	while(1){
		if(sendRecQuickATCommand("AT#SHDN")){			//send off command
			startTime = millis();
			while (((millis() - startTime) < 10000)); 	//block 10 second
			if(!sendRecQuickATCommand("AT"))return 1;
//DebugPort->write("stuck in OFF");
		}else if(!sendRecQuickATCommand("AT"))return 1;		//double check
	}		
//DebugPort->write("stuck in OFF outside");
return 1;
}

//reboots telit
bool gsmTelitHW::reBoot(){
	return sendRecQuickATCommand("AT#REBOOT");
}

//AT+TEMPMON=1 returns temp of mod in C
const char* const gsmTelitHW::getTemperatureTEMPMON(){
//RETURNS: #TEMPMEAS: 0,25 
return  sendRecATCommandSplit("AT+TEMPMON=1",",",1);
}


//GPS functionality--------------------------------
bool gsmTelitHW::powerGPS_GPSP(gps_t state){
//Turns on/off GPS
	char sendMe = (state + 48); //cheap Itoa
	telitPort.write("AT$GPSP=");
	telitPort.write(sendMe);
	telitPort.write("\r");
return catchTelitData(2000,1);		//1= quick check for OK one 2 sec timeout

}	

bool gsmTelitHW::resetGPS_GPSR(){
//resets the GPS
return sendRecQuickATCommand("AT$GPSR=1");	//Reset cold start (option 0-3)
}

bool gsmTelitHW::retoreDefualtGPS_GPSRST(){
//resets GPS settings
return sendRecQuickATCommand("AT$GPSR");
}

const char* const gsmTelitHW::getGPS_GPSACP(){
//AT$GPSACP RETURNS: $GPSACP: <UTC>,<latitude>,<longitude>,<hdop>,<altitude>,
//<fix>,<cog>,<spkm>,<spkn>,<date>,<nsat>
return	sendRecATCommand("AT$GPSACP");
}	


//CFUN SLEEP MODE-----------------------------------
bool gsmTelitHW::setSleepCFUN(sleep_t state){
//Sets what sleep mode to be in
	char sendMe = (state+ 48); //cheap Itoa
	telitPort.write("AT+CFUN=");
	telitPort.write(sendMe);
	telitPort.write("\r");
return catchTelitData(2000,1);		//1= quick check for OK one 2 sec timeout
}

uint16_t gsmTelitHW::checkSleep(){
//returns what sleep mode we are in
//RETURNS +CFUN: 1 OK 
return atoi(sendRecATCommandSplit("AT+CFUN?",":\n",1));
}
//////////////////////////////////////////////////////////////////////HARDWARE FUNCS*

