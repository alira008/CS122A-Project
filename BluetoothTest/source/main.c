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
unsigned char LEDs = 0x00;
unsigned char transmitValue = 0x00;

//========================= LEDs State Machine ===========================

enum BlinkLEDs {LED1, LED2, LED3};

int BlinkLEDsTick(int state){
	//	State Transitions
	switch(state){
		case LED1:
			state = LED2;
			break;
		case LED2:
			state = LED3;
			break;
		case LED3:
			state = LED1;
			break;
		default:
			state = LED1;
			break;
	}
	//	State Actions
	switch(state){
		case LED1:
			transmitValue = 0x01;
			break;
		case LED2:
			transmitValue = 0x02;
			break;
		case LED3:
			transmitValue = 0x04;
			break;
	}
	return state;
}

//========================= USART State Machine ===========================

enum TransmitData_States {TransmitData_Wait, TransmitData_Write, TransmitData_Complete};

int TransmitDataTick(int state){
    //	State Transitions
    switch(state){
		case TransmitData_Wait:
	    	if(USART_IsSendReady(0)){
				state = TransmitData_Write;
	    	}
	    	else {
				state = TransmitData_Wait;
				//PORTA = 0x00;
	    	}
	    	break;
		case TransmitData_Write:
	    	if(USART_HasTransmitted(0)){
				state = TransmitData_Wait;
				PORTA = 0x00;
				USART_Flush(0);
	    	}else
				state = TransmitData_Write;
	    	break;
		default:
	    	state = TransmitData_Wait;
	    	break;
    }
    //	State Actions
    switch(state){
		case TransmitData_Wait:
	    	break;
		case TransmitData_Write:
	    	USART_Send(0x02, 0);
	    	PORTA = 0x01;
	    	break;
    }
    return state;
}

//========================================================================

int main(void) {
	DDRA = 0xFF; PORTA = 0x00;
 	initUSART(0);

	unsigned long int BlinkLEDsTick_calc = 1000;
	unsigned long int TransmitDataTick_calc = 500;

	// Calculate GCD
	unsigned long int tmpGCD = findGCD(BlinkLEDsTick_calc, TransmitDataTick_calc);

    unsigned long int GCD = tmpGCD;

    //Greatest common divisor for all tasks or smallest time unit for tasks.
    unsigned long int BlinkLEDsTick_period = BlinkLEDsTick_calc/GCD;
    unsigned long int TransmitDataTick_period = TransmitDataTick_calc/GCD;

    static task task1, task2;
    task *tasks[] = { &task1, &task2 };
    const unsigned short numTasks = sizeof(tasks)/sizeof(task*);
    
    // Task 1
    task1.state = -1;
    task1.period = BlinkLEDsTick_period;
    task1.elapsedTime = BlinkLEDsTick_period;
    task1.TickFct = &BlinkLEDsTick;

    // Task 2
    task2.state = -1;
    task2.period = TransmitDataTick_period;
    task2.elapsedTime = TransmitDataTick_period;
    task2.TickFct = &TransmitDataTick;

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
