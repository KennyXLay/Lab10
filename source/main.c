/*	Author: lab
 *  Partner(s) Name: Kenny
 *	Lab Section:
 *	Assignment: Lab #  Exercise #
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#ifdef _SIMULATiE_
#include "simAVRHeader.h"
#endif

volatile unsigned char TimerFlag = 0;

unsigned long _avr_timer_M = 1;
unsigned long _avr_timer_cntcurr = 0;

void TimerOn(){
	TCCR1B = 0x0B;
	OCR1A = 125;
	TIMSK1 = 0x02;
	TCNT1 = 0;
	_avr_timer_cntcurr = _avr_timer_M;
	SREG |= 0x80;
}

void TimerOff(){
	TCCR1B = 0x00;
}

void TimerISR(){
	TimerFlag = 1;
}

ISR(TIMER1_COMPA_vect){
	_avr_timer_cntcurr--;
	if(_avr_timer_cntcurr == 0){
		TimerISR();
		_avr_timer_cntcurr = _avr_timer_M;
	}
}

void TimerSet(unsigned long M){
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;	
}

unsigned char SetBit(unsigned char x, unsigned char k, unsigned char b){
	return (b ? (x | (0x01 << k)) : (x & -(0x01 << k)));
}

unsigned char GetBit(unsigned char x, unsigned char k){
	return ((x & (0x01 << k)));
}

typedef struct _task{
	signed char state;
	unsigned long int period;
	unsigned long int elapsedTime;
	int (*TickFct)(int);
}task;

unsigned long int findGCD(unsigned long int a, unsigned long int b){
	unsigned long int c;
	while(1){
		c = a%b;
		if(c==0){return b;}
		a = b;
		b = c;
	}
	return 0;
}

unsigned char GetKeypadKey(){
        PORTC = 0xEF;
        asm("nop");
        if(GetBit(PINC, 0) == 0) { return('1');}
        if(GetBit(PINC, 1) == 0) { return('4');}
        if(GetBit(PINC, 2) == 0) { return('7');}
        if(GetBit(PINC, 3) == 0) { return('*');}

        PORTC = 0xDF;
        asm("nop");
        if(GetBit(PINC, 0) == 0) { return('2');}
        if(GetBit(PINC, 1) == 0) { return('5');}
        if(GetBit(PINC, 2) == 0) { return('8');}
        if(GetBit(PINC, 3) == 0) { return('0');}

        PORTC = 0xBF;
        asm("nop");
        if(GetBit(PINC, 0) == 0) { return('3');}
        if(GetBit(PINC, 1) == 0) { return('6');}
        if(GetBit(PINC, 2) == 0) { return('9');}
        if(GetBit(PINC, 3) == 0) { return('#');}

        PORTC = 0x7F;
        asm("nop");
        if(GetBit(PINC, 0) == 0) { return('A');}
        if(GetBit(PINC, 1) == 0) { return('B');}
        if(GetBit(PINC, 2) == 0) { return('C');}
        if(GetBit(PINC, 3) == 0) { return('D');}

        return ('\0');
}


unsigned char keypress = 0x00;
unsigned char keypad = 0x00;
unsigned char button = 0x00;
unsigned char x = 0x00;
unsigned char y = 0x00;

enum States{keyout} state;
int keypadTick(int key_state){

	switch(state){
		case keyout:
			keypress = GetKeypadKey();
			switch(keypress){
				case'\0': keypad = 0x1F; break;
				case '1': keypad = 0x01; break;
				case '2': keypad = 0x02; break;
				case '3': keypad = 0x03; break;
				case '4': keypad = 0x04; break;
                                case '5': keypad = 0x05; break;
                                case '6': keypad = 0x06; break;
                                case '7': keypad = 0x07; break;
                                case '8': keypad = 0x08; break;
                                case '9': keypad = 0x09; break;
                                case 'A': keypad = 0x0A; break;
                                case 'B': keypad = 0x0B; break;
                                case 'C': keypad = 0x0C; break;
                                case 'D': keypad = 0x0D; break;
                                case '*': keypad = 0x0E; break;
                                case '0': keypad = 0x00; break;
                                case '#': keypad = 0x0F; break;
				default: PORTB = 0x1B; break;
			} state = keyout; break; 
		default: state = keyout; break;
	}
	return state;
}

enum Button_States{buttonpress}Button_State;
int ButtonPressTick(int Button_State){
	switch(Button_State){
		case buttonpress: 
			y = GetKeypadKey();
			if(x == '\0'){
				button = 0;
			}
			else{ button = 1;}
			break;
		default: Button_State = buttonpress; break;
	}
	return Button_State;
}


enum Combine_States{combine}Combine_State;
int CombineTick(int Combine_State){
	unsigned char output;

	switch(Combine_State){
		case combine: output = keypad | (button << 7); break;
		default: Combine_State = combine; break;
	}
	PORTB = output;
	return Combine_State;
}
	
int main(void) {
    	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xF0; PORTC = 0x0F;
    
	static task task1, task2, task3;
	task *tasks[] = {&task1, &task2, &task3};
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

	const char start = 0;
	task1.state = start;
	task1.period = 50;
	task1.elapsedTime = task1.period;
	task1.TickFct = &keypadTick;

	task2.state = start;
	task2.period = 50;
	task2.elapsedTime = task2.period;
	task2.TickFct = &ButtonPressTick;

	task3.state = start;
	task3.period = 50;
	task3.elapsedTime = task3.period;
	task3.TickFct = &CombineTick;

	TimerSet(50);
	TimerOn();

	unsigned long GCD = tasks[0]->period;
	for(unsigned short i = 1; i < numTasks; i++){
		GCD = findGCD(GCD, tasks[i]->period);
	}
	
	unsigned short i;
	while (1) {
		for (i = 0; i < numTasks;i++) {
			if (tasks[i]->elapsedTime == tasks[i]->period) {
				tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
				tasks[i]->elapsedTime = 0;
			}
			tasks[i]->elapsedTime += GCD;
		}
		while (!TimerFlag);
		TimerFlag = 0;
	}
    return 1;
}

