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
#include "timer.h"
#include "scheduler.h"

unsigned char count = 0;

//========================================================================


//========================================================================
enum StepperMotor_States { StepperWait, StepperRotateCW, StepperRotateCCW};

int StepperMotorTick(int state){
	
	//	Transitions
	switch(state){
		case StepperWait:
			state = StepperRotateCW;
			break;
		case StepperRotateCW:
			state = StepperRotateCW;
			break;
		case StepperRotateCCW:
			break;
		default:
			state = StepperRotateCW;
			break;

	}

	//	State Actions
	switch(state){
		case StepperRotateCW:
			if(count == 0){
				PORTA = 0x90;
				count++;
			}
			else if(count == 1){
				PORTA = 0x80;
				count++;
			}
			else if(count == 2){
				PORTA = 0xC0;
				count++;
			}
			else if(count == 3){
				PORTA = 0x40;
				count++;
			}
			else if(count == 4){
				PORTA = 0x06;
				count++;
			}
			else if(count == 5){
				PORTA = 0x20;
				count++;
			}
			else if(count == 6){
				PORTA = 0x30;
				count++;
			}
			else{
				PORTA = 0x10;
				count = 0;
			}
			break;
	}

	return state;
}

int main(void) {
  
	DDRA = 0xF0; PORTA = 0x0F;		//	Setting first four pins to input and last four pins are output

	unsigned long int StepperMotorTick_calc = 1;

	// Calculate GCD
	unsigned long int tmpGCD = StepperMotorTick_calc;

    unsigned long int GCD = tmpGCD;

    //Greatest common divisor for all tasks or smallest time unit for tasks.
    unsigned long int StepperMotorTick_period = StepperMotorTick_calc/GCD;

    static task task1;
    task *tasks[] = { &task1 };
    const unsigned short numTasks = sizeof(tasks)/sizeof(task*);
    
    // Task 1
    task1.state = -1;
    task1.period = StepperMotorTick_period;
    task1.elapsedTime = StepperMotorTick_period;
    task1.TickFct = &StepperMotorTick;

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