/*
 * Lab10_ex1.c
 *
 * Created: 11/19/2019 10:04:39 AM
 * Author : Matthew L
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

volatile unsigned char TimerFlag = 0; // TimerISR() sets this to 1. C programmer should clear to 0.

// Internal variables for mapping AVR's ISR to our cleaner TimerISR model.
unsigned long _avr_timer_M = 1; // Start count from here, down to 0. Default 1ms
unsigned long _avr_timer_cntcurr = 0; // Current internal count of 1ms ticks

// Set TimerISR() to tick every M ms
void TimerSet(unsigned long M) {
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}

void TimerOn() {
	// AVR timer/counter controller register TCCR1
	TCCR1B 	= 0x0B;	// bit3 = 1: CTC mode (clear timer on compare)
	// bit2bit1bit0=011: prescaler /64
	// 00001011: 0x0B
	// SO, 8 MHz clock or 8,000,000 /64 = 125,000 ticks/s
	// Thus, TCNT1 register will count at 125,000 ticks/s

	// AVR output compare register OCR1A.
	OCR1A 	= 125;	// Timer interrupt will be generated when TCNT1==OCR1A
	// We want a 1 ms tick. 0.001 s * 125,000 ticks/s = 125
	// So when TCNT1 register equals 125,
	// 1 ms has passed. Thus, we compare to 125.
	// AVR timer interrupt mask register

	TIMSK1 	= 0x02; // bit1: OCIE1A -- enables compare match interrupt

	//Initialize avr counter
	TCNT1 = 0;

	// TimerISR will be called every _avr_timer_cntcurr milliseconds
	_avr_timer_cntcurr = _avr_timer_M;

	//Enable global interrupts
	SREG |= 0x80;	// 0x80: 1000000
}

void TimerOff() {
	TCCR1B 	= 0x00; // bit3bit2bit1bit0=0000: timer off
}

void TimerISR() {
	TimerFlag = 1;
}

// In our approach, the C programmer does not touch this ISR, but rather TimerISR()
ISR(TIMER1_COMPA_vect)
{
	// CPU automatically calls when TCNT0 == OCR0 (every 1 ms per TimerOn settings)
	_avr_timer_cntcurr--; 			// Count down to 0 rather than up to TOP
	if (_avr_timer_cntcurr == 0) { 	// results in a more efficient compare
		TimerISR(); 				// Call the ISR that the user uses
		_avr_timer_cntcurr = _avr_timer_M;
	}
}
unsigned char output_Three_LEDS;
unsigned char output_Blinking_LEDS;

enum ThreeLEDsSM{threeLEDS, S1, S2, S3} statethreeLEDS;
enum BlinkingLEDSM{blinkingLED, ON, OFF} stateblinkingLED;
enum CombineLEDsSM{combineLEDS} statecombineLEDS;

void ThreeLEDsSM() // B0-B2
{
	switch(statethreeLEDS)
	{               // Transitions
		case threeLEDS:
			statethreeLEDS = S1;
			break;
		case S1:
			statethreeLEDS = S2;
			break;
		case S2:
			statethreeLEDS = S3;
			break;
		case S3:
			statethreeLEDS = S1;
			break;
		default:
			break;
	}
	switch(statethreeLEDS)
	{               // Actions
		case threeLEDS:
			output_Three_LEDS = 0x00;
			break;
		case S1:
			output_Three_LEDS = 0x01;
			break;
		case S2:
			output_Three_LEDS = 0x02;
			break;
		case S3:
			output_Three_LEDS = 0x04;
			break;
		default:
			break;
	}
}
void BlinkingLEDSM() // B3
{
	switch(stateblinkingLED)
	{               // Transitions
		case blinkingLED:
			stateblinkingLED = ON;
			break;
		case ON:
			stateblinkingLED = OFF;
			break;
		case OFF:
			stateblinkingLED = ON;
			break;
		default:
			break;
	}
	switch(stateblinkingLED)
	{               // Actions
		case blinkingLED:
			output_Blinking_LEDS = 0x00;
			break;
		case ON:
			output_Blinking_LEDS = 0x08;
			break;
		case OFF:
			output_Blinking_LEDS = 0x00;
			break;
		default:
			break;
	}
}
void CombineLEDsSM()
{
	switch(statecombineLEDS)
	{               // Transitions
		case combineLEDS:
			break;
		default:
			break;
	}
	switch(statecombineLEDS)
	{               // Actions
		case combineLEDS:
			PORTB = output_Three_LEDS | output_Blinking_LEDS;
			break;
		default:
			break;
	}
}

int main(void)
{
		DDRB = 0xFF; PORTB = 0x00;
		
		TimerSet(1000);
		TimerOn();
		
		statethreeLEDS = threeLEDS;
		stateblinkingLED = blinkingLED;
		statecombineLEDS = combineLEDS;
		
    /* Replace with your application code */
    while (1) 
    {
			ThreeLEDsSM();
			BlinkingLEDSM();
			CombineLEDsSM();
			
			while(!TimerFlag){}
			TimerFlag = 0;
    }
}

