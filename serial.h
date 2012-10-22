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
* You should have received a copy of the GNU General Public License    	*
* with this program.  If not, see <http://www.gnu.org/licenses/>.      	* 
************************************************************************/


#ifndef SERIAL_H
#define SERIAL_H

#include <inttypes.h> //for u_types
#include <avr/io.h>	//for PORT and REG defs
#include <stdio.h>
#include <avr/interrupt.h> //FOR ISR

#include "global.h"


//DEFINE TO USE
#define SERIAL0
#define SERIAL1
//#define SERIAL2
//#define SERIAL3

#define ATT_MSG_CENTER "+13123149810"

//define port functions; example: PORT_ON( PORTD, 6);
#define PORT_ON( port_letter, number )			port_letter |= (1<<number)
#define PORT_OFF( port_letter, number )			port_letter &= ~(1<<number)
#define FLIP_PORT( port_letter, number )		port_letter ^= (1<<number)
#define PORT_IS_ON( port_letter, number )		( port_letter & (1<<number) )
#define PORT_IS_OFF( port_letter, number )		!( port_letter & (1<<number) )



struct ring_buffer;

class SerialPort{

private:
	ring_buffer* rx_buffer;
	//Registers
	volatile uint8_t* ubrrh;
	volatile uint8_t* ubrrl;
	volatile uint8_t* ucsra;
	volatile uint8_t* ucsrb;
	volatile uint8_t* ucsrc;
	volatile uint8_t* udr;

	//bit offsets
	uint8_t rxen;
	uint8_t txen;
	uint8_t rxcie;
	uint8_t txcie;
	uint8_t udre;
	uint8_t usbs;
	uint8_t ucsz;
	uint8_t u2x;
	
//class SerialPort constructors w/ mad params
public:
	//class SerialPort default constructor
	SerialPort();
		
	SerialPort (
	ring_buffer *rx_buffer,
	//Registers
	volatile uint8_t* ubrrh, volatile uint8_t* ubrrl,
	volatile uint8_t* ucsra, volatile uint8_t* ucsrb,
	volatile uint8_t* ucsrc, volatile uint8_t* udr, 
	//last are offset numbers for bits
	uint8_t rxen, uint8_t txen, uint8_t rxcie, uint8_t txcie,
	uint8_t udre, uint8_t usbs, uint8_t ucsz, uint8_t u2x);
	


	//methods within SerialPort Class
	void USART_Transmit(char data);
	//void USART_Transmit(uint8_t data);
	
	//generic write
	void write();	
	//template allows you to send a parameter whose type can change. Write() must handle different types
	template<typename T>
	void write(T);
	//pointer to the type identifier
	template <typename T>
	void write(T*);
	template <typename T>
	void write(T*, uint8_t);
	
	//allows a storing a char  to buffer
	void store_char(unsigned char c, ring_buffer *rx_buffer);
	//reads a byte from buffer and returns char
	char read();
	//returns whether buffer is available (empty?)
	uint8_t available();
	//empties buffer of all data
	void flush();
	//initialize USART
	void USART_Init( uint32_t baud );

};

//uint8_t
/*
template <typename UT>
void SerialPort::write(UT ubyte){
	USART_Transmit(ubyte);
}
*/	

//char
template <typename T>
void SerialPort::write(T ch){
	
USART_Transmit(ch);

}

template <typename T>
void SerialPort::write(T* str)
{
while (*str) write(*str++);
}

template <typename T>
void SerialPort::write(T* str, uint8_t numbytes)
{
	while(numbytes > 0){
		//USART_Transmit(*str++); //byte plus
		write(*str++);
		numbytes--;
	}
}


uint32_t getMillis();

 
#ifdef SERIAL0
extern SerialPort SerialAVR;
#endif

#ifdef SERIAL1
extern SerialPort Serial1;
#endif

#ifdef SERIAL2
extern SerialPort Serial2;
#endif

#ifdef SERIAL3
extern SerialPort Serial3;
#endif


#endif
