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
    //ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE);
    ADMUX = (1 << REFS0);
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

unsigned short ADC_read(unsigned char channel){
	// select the corresponding channel 0~7
  	// ANDing with ’7′ will always keep the value
  	// of ‘ch’ between 0 and 7
  	channel = channel & 0x07;
	ADMUX = (ADMUX & 0xF8) | channel;

	// start single convertion
  	// write ’1′ to ADSC
  	ADCSRA |= (1<<ADSC);
 
  	// wait for conversion to complete
  	// ADSC becomes ’0′ again
  	// till then, run loop continuously
  	while(ADCSRA & (1<<ADSC));

    return ADC;
}

//================== Shared Variables ==============================

unsigned char forwardCheck = 0x00;
unsigned char backwardCheck = 0x00;
unsigned char leftCheck = 0x00;
unsigned char rightCheck = 0x00;
unsigned char count = 0x00;

//================== DC Motor State Machines =============================

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

//================== Stepper Motor State Machines =============================

enum StepperMotor_States { StepperWait, StepperRotateCW, StepperRotateCCW};

int StepperMotorTick(int state){
	//	State Transitions
	switch(state){
		case StepperWait:
			if(leftCheck == 0x01)
				state = StepperRotateCW;
			else if(rightCheck == 0x01)
				state = StepperRotateCCW;
			else 
				state = StepperWait;
			break;
		case StepperRotateCW:
			if(leftCheck == 0x01)
				state = StepperRotateCW;
			else 
				state = StepperWait;
			break;
		case StepperRotateCCW:
			if(rightCheck == 0x01)
				state = StepperRotateCCW;
			else 
				state = StepperWait;
			break;
		default:
			state = StepperWait;
			break;
	}
	//	State Actions
	switch(state){
		case StepperWait:
			PORTD = 0x00;
			break;
		case StepperRotateCW:
			if(count == 0){
				PORTD = 0x90;
				count++;
			}
			else if(count == 1){
				PORTD = 0x80;
				count++;
			}
			else if(count == 2){
				PORTD = 0xC0;
				count++;
			}
			else if(count == 3){
				PORTD = 0x40;
				count++;
			}
			else if(count == 4){
				PORTD = 0x60;
				count++;
			}
			else if(count == 5){
				PORTD = 0x20;
				count++;
			}
			else if(count == 6){
				PORTD = 0x30;
				count++;
			}
			else{
				PORTD = 0x10;
				count = 0;
			}
			break;
		case StepperRotateCCW:
			if(count == 0){
				PORTD = 0x10;
				count++;
			}
			else if(count == 1){
				PORTD = 0x30;
				count++;
			}
			else if(count == 2){
				PORTD = 0x20;
				count++;
			}
			else if(count == 3){
				PORTD = 0x60;
				count++;
			}
			else if(count == 4){
				PORTD = 0x40;
				count++;
			}
			else if(count == 5){
				PORTD = 0xC0;
				count++;
			}
			else if(count == 6){
				PORTD = 0x80;
				count++;
			}
			else{
				PORTD = 0x90;
				count = 0;
			}
			break;
	}
	return state;
}

//================== Up/Down Joystick State Machine ==============================
enum UDJoystickState{UDJoystick_wait, Joystick_forward, Joystick_backward};

int UDJoystickTick(int state){
    unsigned short input = ADC_read(1);
    //	State Transitions
    switch(state){
		case UDJoystick_wait:
	    	if(input >= 700)
				state = Joystick_forward;
	    	else if(input <= 300)
				state = Joystick_backward;
	    	else 
				state = UDJoystick_wait;
	    	break;
		case Joystick_forward:
			if(input >= 700)
				state = Joystick_forward;
	    	else if(input <= 300)
				state = Joystick_backward;
	    	else 
				state = UDJoystick_wait;
	    	break;
		case Joystick_backward:
			if(input >= 700)
				state = Joystick_forward;
	    	else if(input <= 300)
				state = Joystick_backward;
	    	else 
				state = UDJoystick_wait;
	    	break;
		default:
	    	state = UDJoystick_wait;
	    	break;
    }
    switch(state){
		case UDJoystick_wait:
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

//================== Left/Right Joystick State Machine ==============================
enum LRJoystickState{LRJoystick_wait, Joystick_Left, Joystick_right};

int LRJoystickTick(int state){
    unsigned short input = ADC_read(0);
    //	State Transitions
    switch(state){
		case LRJoystick_wait:
	    	if(input >= 700)
				state = Joystick_Left;
	    	else if(input <= 300)
				state = Joystick_right;
	    	else 
				state = LRJoystick_wait;
	    	break;
		case Joystick_Left:
			if(input >= 700)
				state = Joystick_Left;
	    	else if(input <= 300)
				state = Joystick_right;
	    	else 
				state = LRJoystick_wait;
	    	break;
		case Joystick_right:
			if(input >= 700)
				state = Joystick_Left;
	    	else if(input <= 300)
				state = Joystick_right;
	    	else 
				state = LRJoystick_wait;
	    	break;
		default:
	    	state = LRJoystick_wait;
	    	break;
    }
    switch(state){
		case LRJoystick_wait:
	    	leftCheck = 0x00;
	    	rightCheck = 0x00;
	    	break;
		case Joystick_Left:
	    	leftCheck = 0x01;
	    	break;
		case Joystick_right:
	    	rightCheck = 0x01;
	    	break;
    }
    return state;
}

//===============================================================

int main(void) {
 	DDRA = 0x00; PORTA = 0xFF;		//	Setting first four pins to input and last four pins are output
 	DDRB = 0xFF; PORTB = 0x00;
 	DDRD = 0xFF; PORTD = 0x00;

 	ADC_init();

 	unsigned long int DCMotorTick_calc = 1;
	unsigned long int UDJoystickTick_calc = 1;
	unsigned long int StepperMotorTick_calc = 1;
	unsigned long int LRJoystickTick_calc= 1;

	// Calculate GCD
	unsigned long int tmpGCD = findGCD(DCMotorTick_calc, UDJoystickTick_calc);
	tmpGCD = findGCD(tmpGCD, StepperMotorTick_calc);
	tmpGCD = findGCD(tmpGCD, LRJoystickTick_calc);

    unsigned long int GCD = tmpGCD;

    //Greatest common divisor for all tasks or smallest time unit for tasks.
    unsigned long int UDJoystickTick_period = UDJoystickTick_calc/GCD;
    unsigned long int DCMotorTick_period = DCMotorTick_calc/GCD;
    unsigned long int StepperMotorTick_period = StepperMotorTick_calc/GCD;
    unsigned long int LRJoystickTick_period = LRJoystickTick_calc/GCD;

    static task task1, task2, task3, task4;
    task *tasks[] = { &task1, &task2, &task3, &task4 };
    const unsigned short numTasks = sizeof(tasks)/sizeof(task*);
    
    // 	Task 1
    task1.state = -1;
    task1.period = UDJoystickTick_period;
    task1.elapsedTime = UDJoystickTick_period;
    task1.TickFct = &UDJoystickTick;

    // 	Task 2
    task2.state = -1;
    task2.period = DCMotorTick_period;
    task2.elapsedTime = DCMotorTick_period;
    task2.TickFct = &DCMotorTick;

    //	Task 3
    task3.state = -1;
    task3.period = StepperMotorTick_period;
    task3.elapsedTime = StepperMotorTick_period;
    task3.TickFct = &StepperMotorTick;

    //	Task 4
    task4.state = -1;
    task4.period = LRJoystickTick_period;
    task4.elapsedTime = LRJoystickTick_period;
    task4.TickFct = &LRJoystickTick;


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
