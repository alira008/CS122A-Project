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


//=================== ADC Functions ====================================

void ADC_init(){
    ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE);
}

unsigned short ADC_read(){
    return ADC;
}

//================== DC Motor State Machine ==============================

unsigned char forwardCheck = 0x00;
unsigned char backwardCheck = 0x00;

//================== State Machines ======================================

enum DCMotorState{DCMotorWait, DCMotorForward, DCMotorBackward};

int DCMotorTick(int state){
	//	State Transitions
	switch(state){
		case DCMotorWait:
			if(forwardCheck == 0x01)
				state = DCMotorForward;
			else if(backwardCheck == 0x01)
				state = DCMotorBackward;
			else 
				state = DCMotorWait;
			break;
		case DCMotorBackward:
			if(backwardCheck == 0x01)
				state = DCMotorBackward;
			else
				state = DCMotorWait;
			break;
		case DCMotorForward:
			if(forwardCheck == 0x01)
				state = DCMotorForward;
			else
				state = DCMotorWait;
			break;
		default:
			state = DCMotorWait;
			break;
	}
	//	State Actions
	switch(state){
		case DCMotorWait:
			PORTB = 0x00;
			break;
		case DCMotorBackward:
			PORTB = 0x05;
			break;
		case DCMotorForward:
			PORTB = 0x0A;
			break;
	}

	return state;
}

//================== Joystick State Machine ==============================
enum JoystickState{Joystick_wait, Joystick_forward, Joystick_backward};

int JoystickTick(int state){
    unsigned short input = ADC_read();
    //	State Transitions
    switch(state){
		case Joystick_wait:
	    	if(input >= 700)
				state = Joystick_forward;
	    	else if(input <= 300)
				state = Joystick_backward;
	    	else 
				state = Joystick_wait;
	    	break;
		case Joystick_forward:
			if(input >= 700)
				state = Joystick_forward;
	    	else if(input <= 300)
				state = Joystick_backward;
	    	else 
				state = Joystick_wait;
	    	break;
		case Joystick_backward:
			if(input >= 700)
				state = Joystick_forward;
	    	else if(input <= 300)
				state = Joystick_backward;
	    	else 
				state = Joystick_wait;
	    	break;
		default:
	    	state = Joystick_wait;
	    	break;
    }
    switch(state){
		case Joystick_wait:
	    	forwardCheck = 0x00;
	    	backwardCheck = 0x00;
	    	break;
		case Joystick_forward:
	    	forwardCheck = 0x01;
	    	break;
		case Joystick_backward:
	    	backwardCheck = 0x01;
	    	break;
    }
    return state;
}


int main(void) {
 	DDRA = 0x00; PORTA = 0xFF;		//	Setting first four pins to input and last four pins are output
 	DDRB = 0xFF; PORTB = 0x00;

 	ADC_init();

 	unsigned long int DCMotorTick_calc = 1;
	unsigned long int JoystickTick_calc = 1;

	// Calculate GCD
	unsigned long int tmpGCD = findGCD(DCMotorTick_calc, JoystickTick_calc);

    unsigned long int GCD = tmpGCD;

    //Greatest common divisor for all tasks or smallest time unit for tasks.
    unsigned long int JoystickTick_period = JoystickTick_calc/GCD;
    unsigned long int DCMotorTick_period = DCMotorTick_calc/GCD;

    static task task1, task2;
    task *tasks[] = { &task1, &task2 };
    const unsigned short numTasks = sizeof(tasks)/sizeof(task*);
    
    // Task 1
    task1.state = -1;
    task1.period = JoystickTick_period;
    task1.elapsedTime = JoystickTick_period;
    task1.TickFct = &JoystickTick;

    // Task 2
    task2.state = -1;
    task2.period = DCMotorTick_period;
    task2.elapsedTime = DCMotorTick_period;
    task2.TickFct = &DCMotorTick;

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
