
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








#ifndef TELIT_HW
#define TELIT_HW

#include "gsmbase.h"

#define OnOffPin PB0 	//pin connected to Telit power
#define OnOffPort PORTB	//PORT B for SOBI POWER
#define OnOffPortD DDRB
#define SleepDTRPin	//pin to control Telit DTR
#define SleepMonitorPin	//pin to monitor CTS sleep pin

//GPS HELPER


class gsmTelitHW : virtual public GSMbase 
{

	public:
	//Sleep modes use with CFUN
	enum sleep_t{
		DEEPSLEEP	= 0,	//DEEP SLEEP ONLY WAKEN WITH RTS LINE NO AT
		ON 		= 1,	//Full functionality
		DISABLENETWORK 	= 4,	//sleeps off network
		KEEPNETWORK 	= 5,	//Good sleep can be woke from SMS
		CYCLIC		= 7	//timed sleep
	};
	//To turn on off GPS
	enum gps_t{
		GPSON		= 1,
		GPSOFF		= 2
	};
	gsmTelitHW(SerialPort&, uint32_t (*)(), SerialPort*);	
	//Hardware
	virtual bool turnOn();		//Used to turn on Telit 
	virtual bool turnOff();		//Turn off Telit
	bool reBoot();	//Reboots Telit
	const char* const getTemperatureTEMPMON(); // Returns Temperature of mod in C
	int powerCycle();
	//GPS Functionality
	bool powerGPS_GPSP(gps_t);	//Turns on off GPS	
	bool resetGPS_GPSR();		//resets the GPS
	bool retoreDefualtGPS_GPSRST();	//resets GPS settings
	const char* const getGPS_GPSACP(); //returns GPS !!*if supported*!!
	//$GPSACP: <UTC>,<latitude>,<longitude>,<hdop>,<altitude>,
	//<fix>,<cog>,<spkm>,<spkn>,<date>,<nsat>

	//SLEEP
	//sleep mode NOTES:
	//To exit sleep mode 5 the DTR line must be ON (0V) and then poll
	//the Telit CTS line for on state before sending AT commands. It
	//can also be woke from a incoming SMS message.
	bool setSleepCFUN(sleep_t);	//Sets what sleep mode to be in
	uint16_t checkSleep();		//returns what sleep mode we are in

};

#endif
