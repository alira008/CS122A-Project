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

//========================= Shared Variables ==============================

unsigned char receivedValue = 0x00;
unsigned char forwardCheck = 0x00;
unsigned char backwardCheck = 0x00;
unsigned char leftCheck = 0x00;
unsigned char rightCheck = 0x00;
unsigned char count = 0x00;
unsigned char DCMotorOutput = 0x00;
unsigned char stepperMotorOutput = 0x00;

//========================= Automatic Night Time Lights ===================

enum NightTimeLightsState{NightLightsOn, NightLightsOff};

int NightTimeLightTick(int state){
	unsigned short input = ADC_read(0);
	// State Transitions
	switch(state){
		case NightLightsOff:
			if(input < 70)
				state = NightLightsOn;
			else
				state = NightLightsOff;
			break;
		case NightLightsOn:
			if(input < 70)
				state = NightLightsOn;
			else
				state = NightLightsOff;
			break;
	}
	// State Actions
	switch(state){
		case NightLightsOff:
			PORTC = 0x00;
			break;
		case NightLightsOn:
			PORTC = 0x03;
			break;
	}
	return state;
}

//========================= USART Receive State Machine ===================

enum ReceiveStates {ReceiveWait, ReceiveData};
	
int ReceiveTick(int state) {
	// State Transitions
	switch(state) {
		case ReceiveWait:
			if(USART_HasReceived(1))
				state = ReceiveData;
			else
				state = ReceiveWait;
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
			receivedValue = USART_Receive(1);
			USART_Flush(1);
			//PORTA = receivedValue;
			break;
		default: 
			break;
	}
	return state;
}

//========================= Command State Machine =========================

enum CommandStates{CommandParse};

int CommandTick(int state) {
	//	State Transitions
	switch(state) {
		case CommandParse:
			state = CommandParse;
			break;
	}
	//	State Actions
	switch(state) {
		case CommandParse:
			forwardCheck = (receivedValue >> 1) & 0x01;
			backwardCheck = (receivedValue >> 0) & 0x01;
			leftCheck = (receivedValue >> 3) & 0x01;
			rightCheck = (receivedValue >> 2) & 0x01;
			break;
	}
	return state;
}

//================== Up/Down Joystick State Machine =======================
enum UDJoystickState{UDJoystick_wait, Joystick_forward, Joystick_backward};

int UDJoystickTick(int state){
    unsigned short input = ADC_read(2);
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
    }
    //	State Actions
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

//================== Left/Right Joystick State Machine ====================
enum LRJoystickState{LRJoystick_wait, Joystick_Left, Joystick_right};

int LRJoystickTick(int state){
    unsigned short input = ADC_read(1);
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
    }
    //	State Actions
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

//========================= DC Motor State Machine ========================

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
	}
	//	State Actions
	switch(state){
		case DCMotorWait:
			DCMotorOutput = 0x00;
			break;
		case DCMotorBackward:
			DCMotorOutput = 0x05;
			break;
		case DCMotorForward:
			DCMotorOutput = 0x0A;
			break;
	}

	return state;
}

//========================= Stepper Motor State Machines ==================

enum StepperMotor_States { StepperWait, StepperRotateCW, StepperRotateCCW };

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
	}
	//	State Actions
	switch(state){
		case StepperWait:
			stepperMotorOutput = 0x00;
			break;
		case StepperRotateCW:
			if(count == 0){
				stepperMotorOutput = 0x90;
				count++;
			}
			else if(count == 1){
				stepperMotorOutput = 0x80;
				count++;
			}
			else if(count == 2){
				stepperMotorOutput = 0xC0;
				count++;
			}
			else if(count == 3){
				stepperMotorOutput = 0x40;
				count++;
			}
			else if(count == 4){
				stepperMotorOutput = 0x60;
				count++;
			}
			else if(count == 5){
				stepperMotorOutput = 0x20;
				count++;
			}
			else if(count == 6){
				stepperMotorOutput = 0x30;
				count++;
			}
			else{
				stepperMotorOutput = 0x10;
				count = 0;
			}
			break;
		case StepperRotateCCW:
			if(count == 0){
				stepperMotorOutput = 0x10;
				count++;
			}
			else if(count == 1){
				stepperMotorOutput = 0x30;
				count++;
			}
			else if(count == 2){
				stepperMotorOutput = 0x20;
				count++;
			}
			else if(count == 3){
				stepperMotorOutput = 0x60;
				count++;
			}
			else if(count == 4){
				stepperMotorOutput = 0x40;
				count++;
			}
			else if(count == 5){
				stepperMotorOutput = 0xC0;
				count++;
			}
			else if(count == 6){
				stepperMotorOutput = 0x80;
				count++;
			}
			else{
				stepperMotorOutput = 0x90;
				count = 0;
			}
			break;
	}
	return state;
}

//========================= Combine Motor Output ==========================

enum CombineMotorStates{CombineMotorOutput};

int CombineMotorTick(int state) {
	//	State Transitions
	switch(state){
		case CombineMotorOutput:
			state = CombineMotorOutput;
			break;
	}
	//	State Actions
	switch(state){
		case CombineMotorOutput:
			PORTB = DCMotorOutput | stepperMotorOutput;
			break;
	}
	return state;
}

//=========================================================================


int main(void) {
	DDRA = 0xF0; PORTA = 0x0F;
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xFF; PORTC = 0x00;

	ADC_init();
 	initUSART(1);
 	USART_Flush(1);

	unsigned long int ReceiveTick_calc = 3;
	unsigned long int CommandTick_calc = 1;
	unsigned long int DCMotorTick_calc = 1;
	unsigned long int StepperMotorTick_calc = 1;
	unsigned long int CombineMotorTick_calc = 1;
	unsigned long int NightTimeLightTick_calc = 500;
	unsigned long int UDJoystickTick_calc = 1;
	unsigned long int LRJoystickTick_calc= 1;


	// Calculate GCD
	unsigned long int tmpGCD = findGCD(ReceiveTick_calc, CommandTick_calc);
	tmpGCD = findGCD(tmpGCD, DCMotorTick_calc);
	tmpGCD = findGCD(tmpGCD, StepperMotorTick_calc);
	tmpGCD = findGCD(tmpGCD, CombineMotorTick_calc);
	tmpGCD = findGCD(tmpGCD, NightTimeLightTick_calc);
	tmpGCD = findGCD(tmpGCD, UDJoystickTick_calc);
	tmpGCD = findGCD(tmpGCD, LRJoystickTick_calc);

    unsigned long int GCD = tmpGCD;

    //Greatest common divisor for all tasks or smallest time unit for tasks.
    unsigned long int ReceiveTick_period = ReceiveTick_calc/GCD;
    unsigned long int CommandTick_period = CommandTick_calc/GCD;
    unsigned long int DCMotorTick_period = DCMotorTick_calc/GCD;
    unsigned long int StepperMotorTick_period = StepperMotorTick_calc/GCD;
    unsigned long int CombineMotorTick_period = CombineMotorTick_calc/GCD;
    unsigned long int NightTimeLightTick_period = NightTimeLightTick_calc/GCD;
    unsigned long int UDJoystickTick_period = UDJoystickTick_calc/GCD;
    unsigned long int LRJoystickTick_period = LRJoystickTick_calc/GCD;

    static task task1, task3, task4, task5, task6, task7, task8;
    task *tasks[] = { &task1, &task3, &task4, &task5, &task6, &task7, &task8 };
    const unsigned short numTasks = sizeof(tasks)/sizeof(task*);
    
    // Task 1
    task1.state = ReceiveWait;
    task1.period = ReceiveTick_period;
    task1.elapsedTime = ReceiveTick_period;
    task1.TickFct = &ReceiveTick;

    // // Task 2
    // task2.state = CommandParse;
    // task2.period = CommandTick_period;
    // task2.elapsedTime = CommandTick_period;
    // task2.TickFct = &CommandTick;

    // Task 3
    task3.state = DCMotorWait;
    task3.period = DCMotorTick_period;
    task3.elapsedTime = DCMotorTick_period;
    task3.TickFct = &DCMotorTick;

    // Task 4
    task4.state = StepperWait;
    task4.period = StepperMotorTick_period;
    task4.elapsedTime = StepperMotorTick_period;
    task4.TickFct = &StepperMotorTick;

    // Task 5
    task5.state = CombineMotorOutput;
    task5.period = CombineMotorTick_period;
    task5.elapsedTime = CombineMotorTick_period;
    task5.TickFct = &CombineMotorTick;

    // Task 6
    task6.state = NightLightsOff;
    task6.period = NightTimeLightTick_period;
    task6.elapsedTime = NightTimeLightTick_period;
    task6.TickFct = &NightTimeLightTick;

    // 	Task 7
    task7.state = UDJoystick_wait;
    task7.period = UDJoystickTick_period;
    task7.elapsedTime = UDJoystickTick_period;
    task7.TickFct = &UDJoystickTick;

    //	Task 8
    task8.state = LRJoystick_wait;
    task8.period = LRJoystickTick_period;
    task8.elapsedTime = LRJoystickTick_period;
    task8.TickFct = &LRJoystickTick;

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
