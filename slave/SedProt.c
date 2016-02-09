#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "SedProtDefine.h"
#include "SedProt.h"
#include "SedProtRTS.h"
#include "common_makros.h"

void sedProtProcessData(char addr, char data);
void sedProtProcessRecByte(char recByte);

volatile unsigned char g_data[ALL_ADDRESS_COUNT];

volatile char g_to_send_adr_data_flags[3];

void sedProtSlaveInit(){
	// prepare values
	for (char i = 0; i < ALL_ADDRESS_COUNT; i++){
		g_data[i] = 0;
	}
	for (char i = 0; i < 3; i++){
		g_to_send_adr_data_flags[i] = 0;
	}
	
	// prepare pins and interfaces	
	
	RTS_INIT
	_delay_us(500);
	
	DDRD |= _BV( PD1 ); 
    PORTD &= ~( _BV( PD1 ) ); 

	#define USART_BAUDRATE 9600
	#define BAUD_PRESCALE (((F_CPU / (USART_BAUDRATE * 16UL))) - 1)

	UBRRH = (BAUD_PRESCALE >> 8); // Load upper 8-bits of the baud rate value into the high byte of the UBRR register
	UBRRL = BAUD_PRESCALE; // Load lower 8-bits of the baud rate value into the low byte of the UBRR register


	UCSRA = 0; // clear all flags
	UCSRB = ( 1 << TXEN ) | ( 1 << RXEN ) | (1 << RXCIE ); // rx enable, tx enable, rx_interrupt enable
	UCSRC = (1 << URSEL) | (1 << UCSZ0) | (1 << UCSZ1); // Use 8-bit character sizes

	#define USART_TRANSMIT_INTERRUPT_ENABLE  {UCSRB |= _BV(UDRIE);}; // enable send interrupt
	#define USART_TRANSMIT_INTERRUPT_DISABLE  {UCSRB &= ~_BV(UDRIE);}; // enable send interrupt
	
	// prepare interrupts
	
	sei();
	
	// start
	
}

void sedProtSlaveWrite(unsigned char addr, unsigned char data){
	cli();
	g_data[addr] = data;
	sei();
}

unsigned char sedProtSlaveRead(unsigned char addr){
	return g_data[addr];
}

unsigned char sedProtSlaveReadNoD0(unsigned char addr){
	return (g_data[addr] >> 1) & 0b00111111;
}

unsigned char sedProtIsOurId(char dev_id){
	for (char i = 0; i < DEVICE_ADDRESS_COUNT; i++){
		if (g_device_addresses[i] == dev_id)
			return 1;
	}
	return 0;
}

inline
void sedProtProcessData(char addr, char data){

	char dev_id = (addr >> 2) & 0b00011111;
	unsigned char clear_data = (data & 0b01111111);

	if ((addr & 0b00000010) == 0){ // write data from other slave
		g_data[dev_id] = clear_data;
	}else{ // master ask something

		if ((addr & 0b00000001) == 1){ // we should write some data before answer
			g_data[dev_id] = clear_data;
		}

		if (dev_id >= 30) // timestamp. no response needed
		{
			return;
		}
		
		if (sedProtIsOurId(dev_id)){ // prepare answer
			g_to_send_adr_data_flags[0] = g_data[dev_id];
			g_to_send_adr_data_flags[1] = addr & 0b11111100;
			g_to_send_adr_data_flags[2] = 2;
			
			// send data;
			RTS_HIGH
			USART_TRANSMIT_INTERRUPT_ENABLE
			_delay_ms(100);
		}
	}
	
}

inline 
void sedProtProcessRecByte(char recByte){

	static char parts = 0;
	static char addr = 0;
	static char data = 0;
	


	if (recByte & 0b10000000){
		addr = recByte;
		parts |= 1;
		
	}else{
		if (parts & 1){
			data = recByte;
			parts |= 2;
		}else{ // get data without address
			parts = 0;
		}
	}
	
	if (parts != 3) // packet not full
		return;
		
	char cs = data & 3;
	if (cs == 1 || cs == 2) // bad packet
	{
		parts = 0;
		return;	
	}
	
	sedProtProcessData(addr, data);
	parts = 0;
}

ISR(USART_RXC_vect)
{
	cli();	
	sedProtProcessRecByte(UDR);
	sei();
}

ISR(USART_UDRE_vect)
{
	cli();
	if (g_to_send_adr_data_flags[2] == 0) {
		USART_TRANSMIT_INTERRUPT_DISABLE
		_delay_us(500);
		RTS_LOW
	} else {
			UDR = g_to_send_adr_data_flags[g_to_send_adr_data_flags[2] - 1];
			g_to_send_adr_data_flags[2]--;
	}
	sei();
}
