// SpaceInvaders.c
// Runs on LM4F120/TM4C123
// Jonathan Valvano and Daniel Valvano
// This is a starter project for the EE319K Lab 10

// Last Modified: 11/20/2018 
// http://www.spaceinvaders.de/
// sounds at http://www.classicgaming.cc/classics/spaceinvaders/sounds.php
// http://www.classicgaming.cc/classics/spaceinvaders/playguide.php
/* This example accompanies the books
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2018

   "Embedded Systems: Introduction to Arm Cortex M Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2018

 Copyright 2018 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */
// ******* Possible Hardware I/O connections*******************
// Slide pot pin 1 connected to ground
// Slide pot pin 2 connected to PD2/AIN5
// Slide pot pin 3 connected to +3.3V 
// fire button connected to PE0
// special weapon fire button connected to PE1
// 8*R resistor DAC bit 0 on PB0 (least significant bit)
// 4*R resistor DAC bit 1 on PB1
// 2*R resistor DAC bit 2 on PB2
// 1*R resistor DAC bit 3 on PB3 (most significant bit)
// LED on PB4
// LED on PB5

// Backlight (pin 10) connected to +3.3 V
// MISO (pin 9) unconnected
// SCK (pin 8) connected to PA2 (SSI0Clk)
// MOSI (pin 7) connected to PA5 (SSI0Tx)
// TFT_CS (pin 6) connected to PA3 (SSI0Fss)
// CARD_CS (pin 5) unconnected
// Data/Command (pin 4) connected to PA6 (GPIO), high for data, low for command
// RESET (pin 3) connected to PA7 (GPIO)
// VCC (pin 2) connected to +3.3 V
// Gnd (pin 1) connected to ground

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "ST7735.h"
#include "Random.h"
#include "PLL.h"
#include "ADC.h"
#include "Images.h"
#include "Sound.h"
#include "Timer0.h"
#include "Timer1.h"
#include "Print.h"


void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void Delay100ms(uint32_t count); // time delay in 0.1 seconds

uint32_t array[2];
	
void SysTick_Init(void){
	NVIC_ST_CTRL_R = 0;                   // disable SysTick during setup
  NVIC_ST_RELOAD_R = 0x0013D620;  // maximum reload value for 60MHZ
  NVIC_ST_CURRENT_R = 0;                // any write to current clears it
  NVIC_ST_CTRL_R = 0x07; // enable SysTick with core clock and interrupts
}

void PortF_Init(void){
	volatile int delay;
	SYSCTL_RCGCGPIO_R |= 0x20;  //initialize port F clock
	delay++;
	GPIO_PORTF_DEN_R |= 0x1E;   //enables PF1, PF2, PF3, PF4
	GPIO_PORTF_DIR_R |= 0x0E;   //establish PF1, PF2, PF3 as outputs, PF4 as input
	
	volatile int delay2;
	SYSCTL_RCGCGPIO_R |= 0x01;  //initialize port A clock
	delay2++;
	GPIO_PORTA_DEN_R |= 0x10;   //enable PA4
	GPIO_PORTA_DIR_R &= 0xEF;   //enable PA4 as input
}

void setGrass(void){
	ST7735_DrawBitmap(0, 18, GRASS, 24,18);
	ST7735_DrawBitmap(24, 18, GRASS, 24,18);
	ST7735_DrawBitmap(48, 18, GRASS, 24,18);
	ST7735_DrawBitmap(72, 18, GRASS, 24,18);
	ST7735_DrawBitmap(96, 18, GRASS, 24,18);
	ST7735_DrawBitmap(120, 18, GRASS, 24,18);
	
	ST7735_DrawBitmap(0, 100, GRASS, 24,18);
	ST7735_DrawBitmap(24, 100, GRASS, 24,18);
	ST7735_DrawBitmap(48, 100, GRASS, 24,18);
	ST7735_DrawBitmap(72, 100, GRASS, 24,18);
	ST7735_DrawBitmap(96, 100, GRASS, 24,18);
	ST7735_DrawBitmap(120, 100, GRASS, 24,18);
	
	ST7735_DrawBitmap(0, 160, GRASS, 24,18);
	ST7735_DrawBitmap(24, 160, GRASS, 24,18);
	ST7735_DrawBitmap(48, 160, GRASS, 24,18);
	ST7735_DrawBitmap(72, 160, GRASS, 24,18);
	ST7735_DrawBitmap(96, 160, GRASS, 24,18);
	ST7735_DrawBitmap(113, 160, GRASS, 24,18);
	ST7735_DrawBitmap(117, 160, GRASS, 24,18);
}

int main(void){
  PLL_Init(Bus80MHz);       // Bus clock is 80 MHz 
  Random_Init(1);
  Output_Init();
  ST7735_FillScreen(0x0000);            // set screen to black
	ADC_Init89();
	PortF_Init();
	Timer0_Init();
	Timer1_Init();
	Sound_Init();
	
	ST7735_DrawBitmap(0, 160, splash, 128,160);
	while((GPIO_PORTA_DATA_R & 0x10) == 0){};
	ST7735_FillScreen(0x0000);            // set screen to black
	EnableInterrupts();
	SysTick_Init();
	setGrass();
  while(1){
  }

}
long x = 64;
long y = 160;

int slow = 0;
long xcar_1 = 0;
long xtruck_1 = 128;
long xcar_2 = 25;
long xtruck_2 = 90;
long xcar_3 = 30;
void SysTick_Handler(void){
	//	GPIO_PORTF_DATA_R ^= 0x04;
		ADC_In89(array);
		
		if(array[0] < 2000){                       //up
			if(y == 18)															 
				  ;
			else{																					
					y--;																			
					ST7735_DrawBitmap(x, y, frog1, 24,18);
			}
		}
		if(array[0] > 2500){                        //down
			if(y == 160)
				;
			else{
				y++;
				ST7735_DrawBitmap(x, y, frog1, 24,18);
			}
		}
		
		if(array[1] > 2500){                          //right
			if(x == 104)
				;
			else{
				x++;
				ST7735_DrawBitmap(x, y, frog1, 24,18);
			}
		}
		if(array[1] < 2000){                          //left
			if(x == 0)
				;
			else{
				x--;
				ST7735_DrawBitmap(x, y, frog1, 24,18);
			}
		}
		
		slow++;
		if(slow==1){
			xcar_1 = (xcar_1 +1) % 128;
			ST7735_DrawBitmap(xcar_1, 140, car, 22,20);      //car on first lane
			
			xtruck_1--;
			if(xtruck_1 == 1){
				xtruck_1 = 128;
				ST7735_FillRect(0,105,35,15,0x0000);
			}
			ST7735_DrawBitmap(xtruck_1, 120, truck, 35,15);  //truck on 2nd lane
			
			xcar_2 = (xcar_2 +1) % 128;
			ST7735_DrawBitmap(xcar_1, 80, car, 22,20);       //car on 3rd lane
			
			xtruck_2--;
			if(xtruck_2 == 1){
				xtruck_2 = 128;
				ST7735_FillRect(0,45,35,15,0x0000);
			}
			ST7735_DrawBitmap(xtruck_2, 60, truck, 35,15);   //truck on 4th lane
			
			xcar_3 = (xcar_3 +1) % 128;
			ST7735_DrawBitmap(xcar_3, 40, car, 22,20);       //car on 5th lane
			
			slow = 0;
		}
		
		if((y-18) <= 140 && (y >= 120) && (x >= xcar_1) && (x <= (xcar_1 +22))){
			  ST7735_FillScreen(0x0000);            // set screen to black
				setGrass();
				x = 64;
			  y= 128;
		}
		
		if((y-18) <= 120 && (y >= 105) && (x >= xtruck_1) && (x <= (xtruck_1 +35)) && ((x+24)>=xtruck_1)){
			  ST7735_FillScreen(0x0000);            // set screen to black
		    setGrass();
				x = 64;
			  y= 128;
		}

		
}








/* You can't use this timer, it is here for starter code only 
// you must use interrupts to perform delays
void Delay100ms(uint32_t count){
	uint32_t volatile time;
  while(count>0){
    time = 727240;  // 0.1sec at 80 MHz
    while(time){
	  	time--;
    }
    count--;
  }
} */

