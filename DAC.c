// dac.c
// This software configures DAC output
// Lab 6 requires a minimum of 4 bits for the DAC, but you could have 5 or 6 bits
// Runs on LM4F120 or TM4C123
// Program written by: put your names here
// Date Created: 3/6/17 
// Last Modified: 3/5/18 
// Lab number: 6
// Hardware connections
// TO STUDENTS "REMOVE THIS LINE AND SPECIFY YOUR HARDWARE********
/*
#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "Timer0.h"
// Code files contain the actual implemenation for public functions
// this file also contains an private functions and private data


const unsigned short wave[31] = {8,9,11,12,13,14,14,15,15,14,14,13,12,11,9,8,7,5,4,3,2,2,1,1,1,2,2,3,4,5,7}; //4-bit sine wave table
uint32_t waveCount = 0;
	
	
// **************DAC_Init*********************
// Initialize 4-bit DAC, called once 
// Input: none
// Output: none

void DAC_Init(void){
	volatile uint32_t count;
	SYSCTL_RCGCGPIO_R |= 0x02;
	count = 0;
	GPIO_PORTB_DEN_R |= 0x0F;
	GPIO_PORTB_DIR_R |= 0x0F;
	
	Timer0_Init(3579);
} 

// **************DAC_Out*********************
// output to DAC
// Input: 4-bit data, 0 to 15 
// Input=n is converted to n*3.3V/15
// Output: none

void DAC_Out(uint32_t data){
	GPIO_PORTB_DATA_R &= 0x0;
	GPIO_PORTB_DATA_R |= (0x01 & data);
	GPIO_PORTB_DATA_R |= (0x02 & data);
	GPIO_PORTB_DATA_R |= (0x04 & data);
	GPIO_PORTB_DATA_R |= (0x08 & data);
} 

void Sound_Play(uint32_t period){ //const uint8_t *pt, uint32_t count)
	if(period == 0) {
		GPIO_PORTB_DATA_R = 0;
		TIMER1_TAILR_R = 0;
	}
	else
		TIMER1_TAILR_R = period;  //update systick interrupt period
};

void Timer0_Handler(void){
  TIMER0_ICR_R = TIMER_ICR_TATOCINT;// acknowledge timer0A timeout
	DAC_Out(wave[waveCount]);
	waveCount = (waveCount + 1) % 30;
	
}
*/
