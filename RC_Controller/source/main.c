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
#include "SpiIncludes/spi.c"
#include "RFIDIncludes/mfrc522.c"
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

//================== RFID Shared Variables ================================

unsigned char tag1[] = {0x19, 0x6F, 0x0C, 0x95, 0xEF};
unsigned char tag2[] = {0x99, 0xC7, 0xC2, 0x86, 0x1A};
unsigned char tag3[] = {0x29, 0xA0, 0xFE, 0x4E, 0x39};
unsigned char inputCard[] = {0x00, 0x00, 0x00, 0x00, 0x00};
unsigned char isTag1 = 0x01;
unsigned char isTag2 = 0x01;
unsigned char isTag3 = 0x01;
uint8_t str[10];
uint8_t byte;

//================== Motor Shared Variables ===============================

unsigned char forwardCheck = 0x00;
unsigned char backwardCheck = 0x00;
unsigned char leftCheck = 0x00;
unsigned char rightCheck = 0x00;
unsigned char transmitValue = 0x00;
unsigned char correctCard = 0x00;


//================== RFID Card Read =======================================

enum RFIDStates{RFIDWait, RFIDFoundCard, RFIDLock};

int RFIDTick(int state){
	byte = mfrc522_request(PICC_REQALL, str);
	//	State Transitions
	switch(state){
		case RFIDWait:
			if(byte == CARD_FOUND)
				state = RFIDFoundCard;
			else
				state = RFIDWait;
			break;
		case RFIDFoundCard:
			if(correctCard == 0x01)
				state = RFIDLock;
			else
				state = RFIDWait;
			break;
		case RFIDLock:
			state = RFIDLock;
			break;
	}
	//	State Actions
	switch(state){
		case RFIDWait:
			PORTC = 0x00;
			isTag1 = 0x01;
			isTag2 = 0x01;
			isTag3 = 0x01;
			break;
		case RFIDFoundCard:
			PORTC = 0x01;
			byte = mfrc522_get_card_serial(str);
			if(byte == CARD_FOUND){
				for(byte=0; byte < 5; byte++){
					inputCard[byte] = str[byte];
				}
				for(byte = 0; byte < 5; byte++){
					if(inputCard[byte] != tag1[byte])
						isTag1 = 0x00;
				}
				for(byte = 0; byte < 5; byte++){
					if(inputCard[byte] != tag2[byte])
						isTag2 = 0x00;
				}
				for(byte = 0; byte < 5; byte++){
					if(inputCard[byte] != tag3[byte])
						isTag3 = 0x00;
				}
				if(isTag1 == 0x01 || isTag2 == 0x01 || isTag3 == 0x01){
					correctCard = 0x01;
					PORTC = 0x02;
				}
			}
	    	break;
	    case RFIDLock:
	    	break;
	}
	return state;
}


//================== Up/Down Joystick State Machine =======================
enum UDJoystickState{UDJoystick_wait, Joystick_forward, Joystick_backward};

int UDJoystickTick(int state){
    unsigned short input = ADC_read(1);
    //	State Transitions
    switch(state){
		case UDJoystick_wait:
	    	if(input >= 700 && correctCard == 0x01)
				state = Joystick_forward;
	    	else if(input <= 300 && correctCard == 0x01)
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
			forwardCheck = 0x02;
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
    unsigned short input = ADC_read(0);
    //	State Transitions
    switch(state){
		case LRJoystick_wait:
	    	if(input >= 700 && correctCard == 0x01)
				state = Joystick_Left;
	    	else if(input <= 300 && correctCard == 0x01)
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
			leftCheck = 0x04;
	    	break;
		case Joystick_right:
			rightCheck = 0x08;
	    	break;
    }
    return state;
}

//========================= USART State Machine ===========================

enum TransmitData_States {TransmitData_Wait, TransmitData_Write};

int TransmitDataTick(int state){
	transmitValue = forwardCheck | backwardCheck | leftCheck | rightCheck;
	PORTA = transmitValue << 4;
    //	State Transitions
    switch(state){
		case TransmitData_Wait:
	    	if(USART_IsSendReady(0) && correctCard == 0x01)
				state = TransmitData_Write;
	    	else
				state = TransmitData_Wait;
	    	break;
		case TransmitData_Write:
	    	if(USART_HasTransmitted(0))
				state = TransmitData_Wait;
	    	else
				state = TransmitData_Write;
	    	break;
    }
    //	State Actions
    switch(state){
		case TransmitData_Wait:
	    	break;
		case TransmitData_Write:
	    	USART_Send(transmitValue, 0);
	    	break;
    }
    return state;
}

//========================================================================


int main(void) {
    DDRA = 0xF0; PORTA = 0x0F;
    DDRC = 0xFF; PORTC = 0x00;

    ADC_init();
    spi_init();
    initUSART(0);
 	mfrc522_init();

 	byte = mfrc522_read(VersionReg);
	if(byte == 0x92){
		//PORTC = 0x01;
	}else if(byte == 0x91 || byte==0x90){
		PORTC = 0x02;
	}

    byte = mfrc522_read(ComIEnReg);
	mfrc522_write(ComIEnReg,byte|0x20);
	byte = mfrc522_read(DivIEnReg);
	mfrc522_write(DivIEnReg,byte|0x80);

	unsigned long int RFIDTick_calc = 500;
    unsigned long int UDJoystickTick_calc = 1;
	unsigned long int LRJoystickTick_calc= 1;
	unsigned long int TransmitDataTick_calc = 5;

	unsigned long int tmpGCD = findGCD(UDJoystickTick_calc, LRJoystickTick_calc);
	tmpGCD = findGCD(tmpGCD, TransmitDataTick_calc);
	tmpGCD = findGCD(tmpGCD, RFIDTick_calc);

    unsigned long int GCD = tmpGCD;

    //Greatest common divisor for all tasks or smallest time unit for tasks.
    unsigned long int UDJoystickTick_period = UDJoystickTick_calc/GCD;
    unsigned long int LRJoystickTick_period = LRJoystickTick_calc/GCD;
    unsigned long int TransmitDataTick_period = TransmitDataTick_calc/GCD;
    unsigned long int RFIDTick_period = RFIDTick_calc/GCD;

    static task task1, task2, task3, task4;
    task *tasks[] = { &task1, &task2, &task3, &task4 };
    const unsigned short numTasks = sizeof(tasks)/sizeof(task*);
    
    // 	Task 1
    task1.state = RFIDWait;
    task1.period = RFIDTick_period;
    task1.elapsedTime = RFIDTick_period;
    task1.TickFct = &RFIDTick;

    // 	Task 2
    task2.state = UDJoystick_wait;
    task2.period = UDJoystickTick_period;
    task2.elapsedTime = UDJoystickTick_period;
    task2.TickFct = &UDJoystickTick;

    //	Task 3
    task3.state = LRJoystick_wait;
    task3.period = LRJoystickTick_period;
    task3.elapsedTime = LRJoystickTick_period;
    task3.TickFct = &LRJoystickTick;

    //	Task 4
    task4.state = TransmitData_Wait;
    task4.period = TransmitDataTick_period;
    task4.elapsedTime = TransmitDataTick_period;
    task4.TickFct = &TransmitDataTick;


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
