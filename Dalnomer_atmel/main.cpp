/*
 * Dalnomer_atmel.cpp
 *
 * Created: 19.05.2022 11:10:22
 * Author : Кирилл
 */ 

#define LED PB7
#define LED_PORT PORTB
#define LED_DDR DDRB

#include <avr/io.h>
#define F_CPU 16000000UL
#include<util/delay.h>
#include <avr/interrupt.h>
#include <stddef.h>



using namespace std;

uint8_t    Lazer_opros[4] = {0x80,0x6,0x2,0x78},
           Lazer_start[4] = {0x80,0x6,0x3,0x77},
           Lazer_mm[5] = {0xFA,0x04,0x0C,0x02,0xF4},
           Lazer_0c[5] = {0xFA,0x04,0x05,0x00,0xFD},  //Временной интервал (0 с)
           Lazer_1g[5] = {0xFA,0x04,0x0A,0x00,0xF8},  //Установить частоту 1Гц
           Lazer_5g[5] = {0xFA,0x04,0x0A,0x05,0xF3},  //Установить частоту 5Гц
           Lazer_10g[5] = {0xFA,0x04,0x0A,0x0A,0xEE}, //Установить частоту 10Гц
           Lazer_20g[5] = {0xFA,0x04,0x0A,0x14,0xE4}; //Установить частоту 20Гц

uint8_t Otvet_Lazer[12], Otvet_Lazer_TXT[9], i, incomingByte, idx, j, Error_j;

uint8_t q = 0;
uint8_t receive = 0;
uint8_t rx_data = 0;
volatile uint8_t rx_flag = 0;
bool readyToExchange;
unsigned char numOfDataToSend_USB;
unsigned char numOfDataToReceive_USB;
unsigned char *sendDataPtr_USB;
unsigned char *receivedDataPtr_USB;
unsigned char numOfDataSended_USB;
unsigned char numOfDataReceived_USB;
unsigned char numOfDataToSend_Dal;
unsigned char numOfDataToReceive_Dal;
unsigned char *sendDataPtr_Dal;
unsigned char *receivedDataPtr_Dal;
unsigned char numOfDataSended_Dal;
unsigned char numOfDataReceived_Dal;


void UARTInit() 
{
	UBRR0H = 0;
	UBRR0L = 103; //скорость 9600
	UCSR0B = (1<<RXEN0)|(1<<TXEN0)|(1<<RXCIE0)|(1<<UDRIE0); // разрешаем прием, передачу и прерывания по окончанию приема и опустошению буфера
	UCSR0B = (1<<RXEN0)|(1<<TXEN0); // разрешаем прием, передачу
	UCSR0C = (1<<UCSZ01)|(1<<UCSZ00); //8 бит, 1 стоп бит
	UBRR1H = 0;
	UBRR1L = 103; //скорость 9600
	UCSR1B = (1<<RXEN1)|(1<<TXEN1)|(1<<RXCIE1)|(1<<UDRIE1); // разрешаем прием, передачу и прерывания по окончанию приема и опустошению буфера
	UCSR1B = (1<<RXEN1)|(1<<TXEN1); // разрешаем прием, передачу
	UCSR1C = (1<<UCSZ11)|(1<<UCSZ10); //8 бит, 1 стоп бит
}

void UARTSend_USB(uint8_t *pSendData, uint8_t nNumOfDataToSend)               //отправить бит
{
	sendDataPtr_USB = pSendData;
	numOfDataToSend_USB = nNumOfDataToSend;
	numOfDataSended_USB = 0;
}

unsigned char UARTGet_USB()                  //получить бит
{
	/*if ((Otvet_Lazer[0]==0x80) && (Otvet_Lazer[1]==0x06) && (Otvet_Lazer[2]==0x83) && (Otvet_Lazer[3]==48) && (Otvet_Lazer[6]==0x2E)) {
		int CS = Otvet_Lazer[0]+Otvet_Lazer[1]+Otvet_Lazer[2]+Otvet_Lazer[3];
		Error_j = 0;
		for (int i=0; i <= 6; i++) {
			j = Otvet_Lazer[i+4];
			CS=CS+j;
			if (((j > 47) && (j < 59)) || (j==0x2E)) Otvet_Lazer_TXT[i] = j;
			else Error_j=1;
		}
		if ((Error_j == 0) && (256 - CS % 256 == Otvet_Lazer[11])) {
			Otvet_Lazer_TXT[7] = 0x0D;              //Ставим символ переноса в конце строки
			Otvet_Lazer_TXT[2] = 0x2C;              //Ставим запятую вместо точки в число на выдачу
			Serial.write(Otvet_Lazer_TXT,8);
		}*/
}
void UARTSend_Dal(uint8_t *pSendData, uint8_t nNumOfDataToSend)               //отправить бит
{
	sendDataPtr_Dal = pSendData;
	numOfDataToSend_Dal = nNumOfDataToSend;
	numOfDataSended_Dal = 0;
	UDR1 = sendDataPtr_Dal[numOfDataSended_Dal];
		//sendDataPtr_Dal++;
		numOfDataSended_Dal++;
    //UCSR0B = (1<<UDRIE0);
	//while (numOfDataToSend_Dal != numOfDataSended_Dal);
}

void UARTGet_Dal()                  //получить бит
{
	if ((Otvet_Lazer[0]==0x80) && (Otvet_Lazer[1]==0x06) && (Otvet_Lazer[2]==0x83) && (Otvet_Lazer[3]==48) && (Otvet_Lazer[6]==0x2E)) 
	{
		int CS = Otvet_Lazer[0]+Otvet_Lazer[1]+Otvet_Lazer[2]+Otvet_Lazer[3];
		Error_j = 0;
		for (int i=0; i <= 6; i++) 
		{
			j = Otvet_Lazer[i+4];
			CS=CS+j;
			if (((j > 47) && (j < 59)) || (j==0x2E)) Otvet_Lazer_TXT[i] = j;
			else Error_j=1;
		}
		if ((Error_j == 0) && (256 - CS % 256 == Otvet_Lazer[11])) 
		{
			Otvet_Lazer_TXT[7] = 0x0D;              //Ставим символ переноса в конце строки
			Otvet_Lazer_TXT[2] = 0x2C;              //Ставим запятую вместо точки в число на выдачу
			UARTSend_USB(Otvet_Lazer_TXT,8);
		}
	}
}

ISR(USART0_RX_vect)    //прерывание Прием окончен(USB)
{
    /*Otvet_Lazer[0] = Otvet_Lazer[1];
    Otvet_Lazer[1] = Otvet_Lazer[2];
    Otvet_Lazer[2] = Otvet_Lazer[3];
    Otvet_Lazer[3] = Otvet_Lazer[4];
    Otvet_Lazer[4] = Otvet_Lazer[5];
    Otvet_Lazer[5] = Otvet_Lazer[6];
    Otvet_Lazer[6] = Otvet_Lazer[7];
    Otvet_Lazer[7] = Otvet_Lazer[8];
    Otvet_Lazer[8] = Otvet_Lazer[9];
    Otvet_Lazer[9] = Otvet_Lazer[10];
    Otvet_Lazer[10] = Otvet_Lazer[11];
    Otvet_Lazer[11] = UDR0;
	receivedDataPtr_USB++;
	numOfDataReceived_USB++;*/

}

ISR(USART0_UDRE_vect)   //прерывание Передача окончена(USB)
{
	//rx_data = UDR0;
	rx_flag = 1;
}
ISR(USART1_RX_vect)    //прерывание Прием окончен(Дальномер)
{
	Otvet_Lazer[0] = Otvet_Lazer[1];
	Otvet_Lazer[1] = Otvet_Lazer[2];
	Otvet_Lazer[2] = Otvet_Lazer[3];
	Otvet_Lazer[3] = Otvet_Lazer[4];
	Otvet_Lazer[4] = Otvet_Lazer[5];
	Otvet_Lazer[5] = Otvet_Lazer[6];
	Otvet_Lazer[6] = Otvet_Lazer[7];
	Otvet_Lazer[7] = Otvet_Lazer[8];
	Otvet_Lazer[8] = Otvet_Lazer[9];
	Otvet_Lazer[9] = Otvet_Lazer[10];
	Otvet_Lazer[10] = Otvet_Lazer[11];
	Otvet_Lazer[11] = UDR1;
	UARTGet_Dal();
}

ISR(USART1_UDRE_vect)   //прерывание Передача окончена(Дальномер)
{
	numOfDataSended_Dal++;
	if(numOfDataToSend_Dal == numOfDataSended_Dal)
	{
		UCSR1B &=~(1<<UDRIE1);
	}
	else
	{
		UDR1 = sendDataPtr_Dal[numOfDataSended_Dal];
		//sendDataPtr_Dal++;
		//numOfDataSended_Dal++;
	}
}


int main(void)
{
	UARTInit();
	UARTSend_Dal(Lazer_mm,5);
	UARTSend_Dal(Lazer_0c,5);
	UARTSend_Dal(Lazer_1g,5);
	UARTSend_Dal(Lazer_start,5);
	/*DDRB |= 1<<PB7;
	PORTB |= 1<<PB7;
	sei();
	UARTInit();
	while(1) 
	{
		receive = UARTGet();
		if(receive == 0) PORTB |= 1<<PB7;
		else PORTB &= ~(1<<PB7);
	}*/
	/*sei();
	UARTInit();
	while(1) 
	{
		receive = UARTGet();
		receive++;
		UARTSend(receive);
	}*/
	/*UARTInit();
	while(1) 
	{
		q++;
		UARTSend(q);
		_delay_ms(1000);
	}*/
	
  /*LED_DDR = 1<<LED;
  while (1)
  {
	  LED_PORT=0<<LED;
	  _delay_ms(5000);
	  LED_PORT=1<<LED;
	  _delay_ms(5000); 
	  
  }*/
}





