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
#include "lcd.h"
#include "spi.c"
#include "mfrc522.c"


//----------------------------------------------

unsigned char receivedData = 0x00;
uint8_t str[900];
uint8_t byte;

//------------Master SPI Code-------------------

// Code is in spi.c

//----------------------------------------------

enum LCD_States {RefreshLCD, Found};

int DisplayLCDTick(int state){
	byte = mfrc522_request(PICC_REQALL, str);
    switch(state){
		case(RefreshLCD):
			if(byte == CARD_FOUND)
				state = Found;
			else
	   			state = RefreshLCD;
	    	break;
	    case Found:
	    	if(byte == CARD_FOUND)
	    		state = Found;
	    	else
	    		state = RefreshLCD;
	    	break;
    }

    switch(state){
		case(RefreshLCD):
	    	LCD_ClearScreen();
	    	LCD_DisplayString(1, "Waiting for RFID");
	    	LCD_Cursor(17);
	    	break;
	    case Found:
	    	LCD_ClearScreen();
			LCD_DisplayString(1, "Found card");
			//byte = mfrc522_get_card_serial(str);
			//if(byte == CARD_FOUND){
			//	for(byte=0; byte < 8; byte++)
			//		LCD_WriteData(str[byte]);
			//}
			// else{
			// 	LCD_DisplayString(17,"Error");
			// }
	    	break;
    }
    return state;
}


int main(void) {
 	DDRD = 0xFF; PORTD = 0x00;
 	DDRC = 0xFF; PORTC = 0x00;
 	DDRA = 0xFF; PORTA = 0x00;
 	
 	spi_init();
 	mfrc522_init();
 	LCD_init();

 	byte = mfrc522_read(VersionReg);
	if(byte == 0x92)
	{
		PORTA = 0x01;
	}else if(byte == 0x91 || byte==0x90)
	{
		PORTA = 0x02;
	}else
	{
		LCD_DisplayString(1 ,"No reader found");
	}

 	byte = mfrc522_read(ComIEnReg);
	mfrc522_write(ComIEnReg,byte|0x20);
	byte = mfrc522_read(DivIEnReg);
	mfrc522_write(DivIEnReg,byte|0x80);

	unsigned long int DisplayLCDTick_calc = 500;
	//unsigned long int TransmitDataTick_calc = 500;

	// Calculate GCD
	unsigned long int tmpGCD = DisplayLCDTick_calc;
	//unsigned long int tmpGCD = findGCD(BlinkLEDsTick_calc, TransmitDataTick_calc);

    unsigned long int GCD = tmpGCD;

    //Greatest common divisor for all tasks or smallest time unit for tasks.
    unsigned long int DisplayLCDTick_period = DisplayLCDTick_calc/GCD;
    //unsigned long int TransmitDataTick_period = TransmitDataTick_calc/GCD;

    static task task1;
    task *tasks[] = { &task1 };
    const unsigned short numTasks = sizeof(tasks)/sizeof(task*);
    
    // Task 1
    task1.state = RefreshLCD;
    task1.period = DisplayLCDTick_period;
    task1.elapsedTime = DisplayLCDTick_period;
    task1.TickFct = &DisplayLCDTick;

    // Task 2
    // task2.state = -1;
    // task2.period = TransmitDataTick_period;
    // task2.elapsedTime = TransmitDataTick_period;
    // task2.TickFct = &TransmitDataTick;

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
