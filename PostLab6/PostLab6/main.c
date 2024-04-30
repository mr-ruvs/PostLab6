//***************************************************************************
// Universidad del Valle de Guatemala
// IE2023: Programación de Microcontroladores
// Autor: Ruben Granados
// Hardware: ATMEGA328P
// Created: 22/04/2024
//***************************************************************************
// PreLab 6
//***************************************************************************

#define F_CPU 16000000
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>

void initUART(void);
void writeUART(char caracter);
void writeTextUART(char* texto);
void asciiVal(void);
void MenuL0(void);
void ADC_init(void);
uint16_t adcRead(uint8_t);

int adcValue1 = 0;
uint8_t StateAscii = 0;
uint8_t StatePot = 0;
char buffLast = '0';
char buffer[10]; 

volatile char bufferRX;


int main(void)
{
	initUART();
	ADC_init();
	sei();
	DDRD = 0xFF;
	DDRB = 0xFF;
	MenuL0();
	PORTD = 0;
	PORTB = 0;
	while (1){

		asciiVal();
		if (StatePot == 1){
			adcValue1 = adcRead(0);
			
			// Convertir el valor int en una cadena de caracteres
			sprintf(buffer, "%d", adcValue1);
			
			// Enviar la cadena de caracteres por el puerto serie
			writeTextUART(buffer);
			
			writeTextUART("\n");
			
			_delay_ms(1000);
		}
	}
}

void initUART(void){
	// Rx y Tx
	DDRD &= ~(1<<DDD0);
	DDRD |= (1<<DDD1);
	// fast mode
	// CONFIGURAR A
	UCSR0A = 0;
	UCSR0A |= (1<<U2X0);
	// CONFIGURAR B /  ISR RX/ habilitar Rx y Tx
	UCSR0B = 0;
	UCSR0B |= (1<<RXCIE0)|(1<<RXEN0)|(1<<TXEN0);
	// frame 8 bits, no paridad, 1 bit stop
	UCSR0C = 0;
	UCSR0C |= (1<<UCSZ01)|(1<<UCSZ00);
	// baudrate 9600
	UBRR0 = 207;
}
void writeUART(char caracter){
	while(!(UCSR0A&(1<<UDRE0)));
	UDR0 = caracter;
}
void writeTextUART(char* texto){
	uint8_t i;
	for (i=0; texto[i] != '\0'; i++){
		while(!(UCSR0A&(1<<UDRE0)));
		UDR0 = texto[i];
	}
}
ISR(USART_RX_vect){
	bufferRX = UDR0;
	if(StateAscii >=1){
		PORTB |= (1<<PB5);
		buffLast = bufferRX;
	}
	while(!(UCSR0A&(1<<UDRE0)));
	UDR0 = bufferRX;
	if (StateAscii >= 1){
		StateAscii++;
		if (StateAscii >= 2){
			StateAscii = 0;
			MenuL0();
		}
	}
	if (StatePot == 1){
		if (bufferRX == '0'){
			StatePot = 0;
			MenuL0();
		}
	}
	if (bufferRX == '1'){
		if (StateAscii == 0){
			writeTextUART("\nValor Potenciometro\n");
			writeTextUART("Escribir 0 para volver\n");
			StatePot = 1;
		}
	} else if (bufferRX == '2'){
		if (StateAscii == 0){
			writeTextUART("\nAscii\n");
			writeTextUART("\nescribe tu codigo Ascii:\n");
			StateAscii = 1;
			PORTD = 0;
			PORTB = 0;
		}
	} 
	
}
void asciiVal(void){
	PORTD = buffLast<<2;
	PORTB = buffLast>>6;
}
void MenuL0(void){
	writeTextUART("\nElige una opcion:\n");
	writeTextUART("1. Leer potenciometro\n");
	writeTextUART("2. enviar Ascii\n");
}
void ADC_init(void){
	ADMUX |= (1<<REFS0);	// VCC REF
	ADMUX &= ~(1<<REFS1);
	ADMUX &= ~(1<<ADLAR);	// 10 bits
	// PRESCALER 128 > 16M/128 = 125KHz
	ADCSRA |= (1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
	ADCSRA |= (1<<ADEN);	// ADC ON
}
uint16_t adcRead(uint8_t canal){
	ADMUX = (ADMUX & 0xF0)|canal;	// selección de canal
	ADCSRA |= (1<<ADSC);	// inicia conversión
	while((ADCSRA)&(1<<ADSC));	// hasta finalizar conversión
	return(ADC);
}




