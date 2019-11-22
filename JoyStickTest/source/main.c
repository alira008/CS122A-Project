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


//********** ADC Functions *********************************************
void ADC_init(){
    ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE);
}

unsigned short ADC_read(){
    return ADC;
}
//********** JoyStickState Machine *************************************
enum JoystickState{Joystick_wait, Joystick_left, Joystick_right};

int JoystickTick(int state){
    
    unsigned short input = ADC_read();
    switch(state){
	case Joystick_wait:
	    if(input >= 700)
		state = Joystick_right;
	    else if(input <= 300)
		state = Joystick_left;
	    else 
		state = Joystick_wait;
	    break;
	case Joystick_left:
		state = Joystick_wait;
	    break;
	case Joystick_right:
		state = Joystick_wait;
	    break;
	default:
	    state = Joystick_wait;
	    break;
    }
    switch(state){
	case Joystick_wait:
	    moveL = 0x00;
	    moveR = 0x00;
	    break;
	case Joystick_left:
	    moveL = 0x01;
	    break;
	case Joystick_right:
	    moveR = 0x01;
	    break;
    }
    return state;
}


int main(void) {
 	DDRA = 0xF0; PORTA = 0x0F;		//	Setting first four pins to input and last four pins are output

	unsigned long int JoystickTick_calc = 1;

	// Calculate GCD
	unsigned long int tmpGCD = JoystickTick_calc;

    unsigned long int GCD = tmpGCD;

    //Greatest common divisor for all tasks or smallest time unit for tasks.
    unsigned long int JoystickTick_period = JoystickTick_calc/GCD;

    static task task1;
    task *tasks[] = { &task1 };
    const unsigned short numTasks = sizeof(tasks)/sizeof(task*);
    
    // Task 1
    task1.state = -1;
    task1.period = JoystickTick_period;
    task1.elapsedTime = JoystickTick_period;
    task1.TickFct = &JoystickTick;

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
