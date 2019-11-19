/*	Author: ariel
 *  Partner(s) Name: 
 *	Lab Section:
 *	Assignment: Lab #  Exercise #
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#include "scheduler.h"
#include "timer.h"
#include "usart_ATmega1284.h"

//========================= Shared Variables =============================
unsigned char receivedValue = 0x00;

//========================= USART State Machine ===========================

enum ReceiveStates {ReceiveWait, ReceiveData};
	
int ReceiveTick(int state) {
	// State Transitions
	switch(state) {
		case ReceiveWait:
			if (USART_HasReceived(0)){
				state = ReceiveData;
			}
			else{
				state = ReceiveWait;
			}
			break;
		case ReceiveData:
			state = ReceiveWait;
			break;
		default:
			state = ReceiveWait;
			break;
	}
	// State Actions
	switch(state) {
		case ReceiveWait:
			break;
		case ReceiveData:
			receivedValue = USART_Receive(0);
			USART_Flush(0);
			break;
	}
	return state;
}

//========================= LEDs State Machine ===========================

enum DisplayLEDState {DisplayLEDs};

int DisplayLEDsTick(int state){
	//	State Transitions
	switch(state){
		case DisplayLEDs:
			state = DisplayLEDs;
		default:
			state = DisplayLEDs;
			break;
	}
	//	State Actions
	switch(state){
		case DisplayLEDs:
			PORTB = receivedValue;
			break;
	}
	return state;
}

//========================================================================

int main(void) {
	DDRB = 0xFF; PORTB = 0x00;

 	initUSART(0);

	unsigned long int DisplayLEDsTick_calc = 1000;
	unsigned long int ReceiveTick_calc = 50;

	// Calculate GCD
	unsigned long int tmpGCD = findGCD(DisplayLEDsTick_calc, ReceiveTick_calc);

    unsigned long int GCD = tmpGCD;

    //Greatest common divisor for all tasks or smallest time unit for tasks.
    unsigned long int DisplayLEDsTick_period = DisplayLEDsTick_calc/GCD;
    unsigned long int ReceiveTick_period = ReceiveTick_calc/GCD;

    static task task1, task2;
    task *tasks[] = { &task1, &task2 };
    const unsigned short numTasks = sizeof(tasks)/sizeof(task*);
    
    // Task 1
    task1.state = -1;
    task1.period = DisplayLEDsTick_period;
    task1.elapsedTime = DisplayLEDsTick_period;
    task1.TickFct = &DisplayLEDsTick;

    // Task 2
    task2.state = -1;
    task2.period = ReceiveTick_period;
    task2.elapsedTime = ReceiveTick_period;
    task2.TickFct = &ReceiveTick;

    TimerSet(GCD);
    TimerOn();

    unsigned short i;
    while (1) {
		// Scheduler code
	 	for ( i = 0; i < numTasks; i++ ) {
	    	// Task is ready to tick
	    	if ( tasks[i]->elapsedTime == tasks[i]->period ) {
				// Setting next state for task
				tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
				// Reset the elapsed time for next tick.
				tasks[i]->elapsedTime = 0;
	    	}
	    	tasks[i]->elapsedTime += 1;
		}
	while(!TimerFlag);
	TimerFlag = 0;

    }
    return 1;
}
