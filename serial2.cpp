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

//serial for mega1280
#include "serial.h"

#define RX_BUFFER_SIZE 128 //***watch buffer overflow!!!

//initializes ring_buffer struct
struct ring_buffer {
  unsigned char buffer[RX_BUFFER_SIZE];
  int head;
  int tail;
};

#ifdef SERIAL0
ring_buffer rx_buffer = { { 0 }, 0, 0 };
#endif
#ifdef SERIAL1
ring_buffer rx_buffer1 = { { 0 }, 0, 0 };
#endif
#ifdef SERIAL2
ring_buffer rx_buffer2 = { { 0 }, 0, 0 };
#endif
#ifdef SERIAL3
ring_buffer rx_buffer3 = { { 0 }, 0, 0 };
#endif

//why do you use inline!??!
/*
inline void store_char(unsigned char c, ring_buffer *rx_buffer)
{
	int i = (rx_buffer->head + 1) % RX_BUFFER_SIZE;
 	if (i != rx_buffer->tail) {			//check if they equal if so buffer overflow
 		rx_buffer->buffer[rx_buffer->head] = c;
 		rx_buffer->head = i;
 	}
 }
 */

void store_char(unsigned char c, ring_buffer *rx_buffer)
{
	int i = (rx_buffer->head + 1) % RX_BUFFER_SIZE;
	if (i != rx_buffer->tail) {			//check if they equal if so buffer overflow
	rx_buffer->buffer[rx_buffer->head] = c;
	rx_buffer->head = i;
	}
}

#ifdef SERIAL0
//serial 0 interupts
ISR(USART0_RX_vect) //triggers when recieve buffer is full
{
 unsigned char c = UDR0;
 store_char(c, &rx_buffer);
}

ISR(USART0_TX_vect) //triggers when send buffer is empty
{}
#endif


#ifdef SERIAL1
//Serial 1 interupts
ISR(USART1_RX_vect) //triggers when recieve buffer is full
{
 unsigned char c = UDR1;
 store_char(c, &rx_buffer1);
}

ISR(USART1_TX_vect) //triggers when send buffer is empty
{}
#endif


#ifdef SERIAL2
//Serial 2 interupts
ISR(USART2_RX_vect) //triggers when recieve buffer is full
{
 unsigned char c = UDR2;
 store_char(c, &rx_buffer2);
}

ISR(USART2_TX_vect) //triggers when send buffer is empty
{}
#endif


#ifdef SERIAL3
//Serial 3 interupts
ISR(USART3_RX_vect) //triggers when recieve buffer is full
{
 unsigned char c = UDR3;
 store_char(c, &rx_buffer3);
}

ISR(USART3_TX_vect) //triggers when send buffer is empty
{}
#endif

//initialize a SerialPort class , refers to constructor
SerialPort::SerialPort (
  ring_buffer *rx_buffer,
  //Registers
 volatile uint8_t* ubrrh, volatile uint8_t* ubrrl,
 volatile uint8_t* ucsra, volatile uint8_t* ucsrb,
 volatile uint8_t* ucsrc, volatile uint8_t* udr, 
 //last are offset numbers for bits
 uint8_t rxen, uint8_t txen, uint8_t rxcie, uint8_t txcie,
 uint8_t udre, uint8_t usbs, uint8_t ucsz, uint8_t u2x):
 rx_buffer(rx_buffer),
 ubrrh(ubrrh), ubrrl(ubrrl), ucsra(ucsra), ucsrb(ucsrb), ucsrc(ucsrc), udr(udr),
 rxen(rxen), txen(txen), rxcie(rxcie),txcie(txcie), udre(udre), usbs(usbs),
 ucsz(ucsz),u2x(u2x){ }

// ^^^defines functions of class from above class instantiation ^^^
void SerialPort:: USART_Transmit( char data )
{
  	/* Wait for empty transmit buffer */
	while ( !( *ucsra & (1<< udre)) )
       		;
  	/* Put data into buffer, sends the data */
  	*udr = data;
}

/*
void SerialPort:: USART_Transmit(uint8_t data)
{

	while ( !( *ucsra & (1<< udre)) )
	;
	
	*udr = data;
	
}
*/

//for debug
void SerialPort::write(){
unsigned char myMessage[]="hello  world\r\n";
unsigned char* message;
		message = myMessage;
		unsigned char c;
		while( (c = *(message++)) != '\0' ){
	        USART_Transmit(c );
		}
		message = myMessage;	
}


 
char SerialPort::read()
{
// if the head isn't ahead of the tail, we don't have any characters
	if (rx_buffer->head == rx_buffer->tail) {
		return -1;
	} else {
	char c = rx_buffer->buffer[rx_buffer->tail];
	rx_buffer->tail = (rx_buffer->tail + 1) % RX_BUFFER_SIZE;
	return c;
}
}

uint8_t SerialPort::available()
{
  return (RX_BUFFER_SIZE + rx_buffer->head - rx_buffer->tail) % RX_BUFFER_SIZE;
}

void SerialPort::flush()
{
 rx_buffer->head = rx_buffer->tail;
 }
 

//calc
void SerialPort:: USART_Init( uint32_t  baud )
{
uint16_t baud_rate;
	baud_rate = 0;
	//refer to USART table in atmel datasheet
	switch(baud)
	{
		case 1 : //Serial for Debug/GPS
		baud_rate = 103; //9600
		break;
		
		case 2:	//Serial for Telit
		baud_rate = 8; //115200
		//baud_rate = 103;
		//baud_rate = 25; // 38400
		break;
	}
	
//overflows were occurring in calculation, bad UBRR values assigned

//uint16_t  baud_rate = (F_CPU / 8 / baud - 1) / 2;
//uint16_t baud_rate = (F_CPU / (16 * baud) ) - 1; // = UBRRn value

  //disable power saving
  // PRR0= 0;
  // PRR1= 0;
  /* Set baud rate */
   *ubrrh = (uint8_t)(baud_rate >> 8);
   *ubrrl = (uint8_t)baud_rate;

  // set no U2x double speed
   *ucsra =  (0<<u2x);
   //might have to be set due to large USART error
   
 /* Enable receiver and transmitter
 * and receive transmit interupts
 * (RXCIE0)(TXIE0)*/
   *ucsrb = (1<<rxen)|(1<<txen)|(1<<rxcie)|(1<<txcie);
   //uscrc =  asyncrohnous USART
   
  // Set frame format: 8data, 2stop bit 
  // *ucsrc = (0<<usbs)|(3<<ucsz);
  
}


//Serial and Serial1 are both objects instantiated from Serial Class
//ERROR U2Xn is missing from first serial
//extern def?? so that it can be referenced here?


/*
#ifdef SERIAL0
SerialPort Serial(&rx_buffer, &UBRR0H, &UBRR0L, &UCSR0A, &UCSR0B, &UCSR0C, &UDR0,
 (uint8_t)(RXEN0), (uint8_t)(TXEN0),(uint8_t) (RXCIE0), (uint8_t)(TXCIE0),(uint8_t) (UDRE0),
  (uint8_t)(USBS0),(uint8_t) (UCSZ00), (uint8_t)(U2X0));
#endif
*/
#ifdef SERIAL0
SerialPort SerialAVR(&rx_buffer, &UBRR0H, &UBRR0L, &UCSR0A, &UCSR0B, &UCSR0C, &UDR0,RXEN0,TXEN0,
RXCIE0,TXCIE0,UDRE0,USBS0,UCSZ00,U2X0);
#endif

#ifdef SERIAL1
SerialPort Serial1(&rx_buffer1, &UBRR1H, &UBRR1L, &UCSR1A, &UCSR1B, &UCSR1C, &UDR1,
 RXEN1, TXEN1, RXCIE1, TXCIE1, UDRE1, USBS1, UCSZ10, U2X1);
#endif
#ifdef SERIAL2
SerialPort Serial2(&rx_buffer2, &UBRR2H, &UBRR2L, &UCSR2A, &UCSR2B, &UCSR2C, &UDR2,
 RXEN2, TXEN2, RXCIE2, TXCIE2, UDRE2, USBS2, UCSZ20, U2X2);
#endif
#ifdef SERIAL3
SerialPort Serial3(&rx_buffer3, &UBRR3H, &UBRR3L, &UCSR3A, &UCSR3B, &UCSR3C, &UDR3,
 RXEN3, TXEN3, RXCIE3, TXCIE3, UDRE3, USBS3, UCSZ30,U2X3);
#endif


