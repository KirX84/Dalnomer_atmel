#include "uart13.h"

/*
*	ISR(TIM0_COMPA_vect)
*	���������� ������� �� ��������� � ��������� OCR0A.	��� �������� ��������� � ��������� ������ ���, �����
*	������ ��������� ��������, ����������� � �������� OCR0A. ����� ��� ����������, �������� ������� � ��������
*	TCNT0 ������������� ����������.
*	��� ���������� ������������ ��� �������� ������ �� UART. ���������� txbyte ������������ ��� ��������� �������:
*	������ ���, ����� ���������� ����������, ������� ������ ���������� txbyte ������������ �� ����� TXD ����������,
*	����� ���� ���������� ����� ����������� ���������� ������, � � �������������� ������� ������ ������������
*	���������� �������. ���� ������ ��� �������� ���, � ���������� �������� ����� 0xFFFF �, ��� �����, �� ������
*	TXD ���������� �������������� ������� ���������� �������. ����� �� ����� �������� ������, �� ������ ��������
*	� ������� ��� ����� ��� ��� ��������: 1 ���������, 8 ��� ������ � 1 ��������, ����� 10 (0x0A), � ��������
*	� txbyte ������ ��� �������� ������ �� ��������� �����. ����� ����� ��� ���������� ������ ������������.
*	������������� ������� ���������� ������� void uart_send(uint8_t tb).
*/

ISR(TIM0_COMPA_vect)
{
	TXPORT = (TXPORT & ~(1 << TXD)) | ((txbyte & 0x01) << TXD); // ���������� � ��� TXD ������� ��� txbyte
	txbyte = (txbyte >> 0x01) + 0x8000;							// ������� txbyte ������ �� 1 � ����� 1 � ������� ������ (0x8000)
	if(txbitcount > 0)											// ���� ���� �������� (������ ��� ������ ����),
	{
		txbitcount--;											// �� ��������� ��� �� �������.
	}
}

/*
*	ISR(TIM0_COMPB_vect)
*	���������� ������� �� ��������� � ��������� OCR0B. ��� �������� ���������� ���������� ISR(TIM0_COMPA_vect),
*	��, � ������� �� ����, ��� ���������� ����� ���������� �� ���������� ��������� ������� TCNT0. ��� ����������
*	����������� ������ ����� �� ��������� ������, � ��������� ����� ��� ���������� ���������. ����� ��� ���������,
*	�� ��������� ���������� ��������� ����� UART RXD �, ���� ��� � �������, �� ����� ������� � ������� ������
*	���������� ������ rxbyte, ����� �� ��������� �� ������� ������� �������� ��� �, ���� �� ���� �����, �����������
*	�����. ����� �������� ������ ���������� rxbyte, ����� ����������� �� � ������ ���������� ����.
*/

ISR(TIM0_COMPB_vect)
{
	if(RXPORT & (1 << RXD))			// ��������� � ����� ��������� ���� RXD
		rxbyte |= 0x80;				// ���� � 1, �� ����� 1 � ������� ������ rxbyte
	
	if(--rxbitcount == 0)			// ��������� �� 1 ������� ��� � ��������� �� ���� �� �� �����
	{
		TIMSK0 &= ~(1 << OCIE0B);	// ���� ��, ��������� ���������� �� ��������� OCR0B
		TIFR0 |= (1 << OCF0B);		// ������� ���� ���������� (�����!)
		EIFR |= (1 << INTF0);		// ������� ���� ���������� �� INT0
		EIMSK |= (1 << INT0);		// ��������� ���������� INT0
	}
	else
	{
		rxbyte >>= 0x01;			// ����� �������� ������ �� 1 rxbyte
	}
}

/*
*	ISR(INT0_vect)
*	���������� INT0. ����������� �� ������� ������ �������� �� ����� INT0, ������������
*	��� ������������ ������ ������ ����������. ���������� ������� ��� ������ 9, ��������
*	���������� ���������� rxbyte. ������ �������� ��� �������� OCR0B, ��������� �������������
*	������������ ���������� ISR(TIM0_COMPB_vect), ��� ������ ����������� �� �������� ������������
*	���� (�� �������). ����� ���� ���������� ISR(TIM0_COMPB_vect) �����������, � ���������� INT0
*	�����������.
*/

ISR(INT0_vect)
{
	rxbitcount = 0x09;						// 8 ��� ������ � 1 ��������� ���
	rxbyte = 0x00;							// �������� ���������� rxbyte
	if(TCNT0 < (BAUD_DIV / 2))				// ���� ������ �� �������� �� �������� �������� �������
	{
		OCR0B = TCNT0 + (BAUD_DIV / 2);	// �� ���������� ���������� � ������� ������� ������ ��� �������
	}
	else
	{
		OCR0B = TCNT0 - (BAUD_DIV / 2);	// ����� ���������� ���������� � ��� ��������� ������� �������
	}
	EIMSK &= ~(1 << INT0);					// ��������� ���������� �� INT0
	TIFR0 |= (1 << OCF0A) | (1 << OCF0B);	// ������� ���� ���������� INT0
	TIMSK0 |= (1 << OCIE0B);				// ��������� ���������� �� OCR0B
}

/*
*	void uart_send(uint8_t tb)
*	������� �������� ����� �� UART. ��������� � �������� ��������� ���� ��� ��������, ������������� �������� ���.
*	���� � ������ ������ ���� �������� �����, �� ��� ���� ���� ��������	���������� ����� ���� ���������� �
*	������� 8 ��� ���������� txbyte ���� ��� ��������, ������� 8 ��� �������� 0xFF, ����� �������� ����������
*	�����, �������� ����� ������� ��������� ��� � ������� �������. ������ ������� ��� = 10.
*/

void uart_send(uint8_t tb)
{
	while(txbitcount);				// ���� ���� ���������� �������� ����������� �����
	txbyte = (tb + 0xFF00) << 0x01; // ����� � ������� ������� txbyte ������ ��� �������� � �������� ����� �� 1
	txbitcount = 0x0A;				// ������ ������� ���� ������ 10
}

/*
*	int16_t uart_recieve(uint8_t* rb)
*	������� ������ ����� �� UART. ��������� � �������� ��������� �� 8-������ ����������, ��� ����� �����������
*	�������� ����. ���������� �������� ����, ���� ���� ������, ���������� (-1), ���� ��������� ������.
*	���� � ������ ������ ������� ���� �����, ������� ����� ����� ��� ����������. ���� ������� ������� ������,
*	�� ������ ��� ��� ��������� �������� ����, ������ ��� (-1).
*/

int16_t uart_recieve(uint8_t* rb)
{
	if(rxbitcount < 0x09)	// ���� ������� ��� �� ����� ������ 9
	{
		while(rxbitcount);	// ���� ���� ���������� ������� �����
		*rb = rxbyte;		// ����� �� ������ ��������� �������� ����
		rxbitcount = 0x09;	// ��������������� �������� �������� ���
		return (*rb);		// ������������
	}
	else
	{
		return (-1);		// ����� ���������� -1 (��������� ������)
	}
}

/*
*	void uart_init()
*	������� ������������� UART. ���������� ���, ������������� �������� ���.
*	�������������� ���������� ���������� � �������� ����������������.
*/

void uart_init()
{
	txbyte = 0xFFFF;		// �������� ������ �� �������� - ��� �������
	rxbyte = 0x00;			// �������� ������ �� ����� - ��� ����
	txbitcount = 0x00;		// �������� �������� ������������ ��� - ���� (������ ���� �� ��������)
	rxbitcount = 0x09;		// �������� �������� ��� �� ����� - 9 (������� ���������� ������)
	
	TXDDR |= (1 << TXD);		// ������ ����������� ����� �� �������� ��� �����
	RXDDR &= ~(1 << RXD);		// ������ ����������� ����� �� ����� ��� ����
	TXPORT |= (1 << TXD);		// ����� ������� � ����� TXD
	RXPORT |= (1 << RXD);		// ����������� � ������� ���� RXD
	OCR0A = BAUD_DIV;			// ������ �������� �������� OCR0A � ������������ � ���������
	TIMSK0 |= (1 << OCIE0A);	// ��������� ���������� TIM0_COMPA
	TCCR0A |= (1 << WGM01);		// ����� ������� CTC (������� TCNT0 �� ���������� OCR0A)
	TCCR0B |= T_DIV;			// ������ �������� ����� ������� � ������������ � ���������
	MCUCR |= (1 << ISC01);		// ������ ���������� INT0 �� ������� ������ ��������
	EIMSK |= (1 << INT0);		// ��������� ���������� INT0
	sei();						// ��������� ���������� ���������
}
