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
long x = 64;
long y = 160;

typedef struct enemy{
	uint32_t x_pos;
	uint32_t y_pos;
	uint32_t x_size;
	uint32_t y_size;
	const uint16_t *image;
} enemy_t;
	
void InitEnemy(enemy_t* init, uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, const uint16_t *i) {
	init->x_pos = x1;
	init->y_pos = y1;
	init->x_size = x2;
	init->y_size = y2;
	init->image = i;
}

void MoveEnemy(enemy_t *init, int space){
	if (init->x_pos >= 128)
		init->x_pos=0;
	else if (space < 0 && init->x_pos == 0){
		init->x_pos = 128;
		ST7735_FillRect(0,105,35,15,0x0000);
	}
	init->x_pos = (init->x_pos + space);
	ST7735_DrawBitmap(init->x_pos, init->y_pos, init->image, init->x_size, init->y_size);
}


//CheckCollision
//returns 0 if no collision detected
//returns 1 if a collision is detected
int CheckCollision(uint32_t px, uint32_t py, enemy_t* check){
	if((py-18) <= check->y_pos && (py >= (check->y_pos - check->y_size) && (px >= check->x_pos) && (px <= (check->x_pos + check->x_size))))
		return 1;
	
	return 0;
}

int CheckGrassToStreet(){
	if(y == 160 || y==106)
		return 1;
	return 0;
}
	
int CheckStreetToGrass(){
	if(y==124 || y==52)
		return 1;
	return 0;
}

int CheckGrassToStreetDown(){
	if(y == 34 || y==106)
		return 1;
	return 0;
}
	
int CheckStreetToGrassDown(){
	if(y==142|| y==88)
		return 1;
	return 0;
}

enemy_t car1;
enemy_t truck1;


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
	ST7735_DrawBitmap(0, 34, GRASS, 24,18);
	ST7735_DrawBitmap(24, 34, GRASS, 24,18);
	ST7735_DrawBitmap(48, 34, GRASS, 24,18);
	ST7735_DrawBitmap(72, 34, GRASS, 24,18);
	ST7735_DrawBitmap(96, 34, GRASS, 24,18);
	ST7735_DrawBitmap(120, 34, GRASS, 24,18);
	
	ST7735_DrawBitmap(0, 106, GRASS, 24,18);
	ST7735_DrawBitmap(24, 106, GRASS, 24,18);
	ST7735_DrawBitmap(48, 106, GRASS, 24,18);
	ST7735_DrawBitmap(72, 106, GRASS, 24,18);
	ST7735_DrawBitmap(96, 106, GRASS, 24,18);
	ST7735_DrawBitmap(120, 106, GRASS, 24,18);
	
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
	
	while((GPIO_PORTA_DATA_R & 0x10) == 0){
		int titleDelay = 0;
		int titleDelay2 = 0;
		while(titleDelay != 10){
			ST7735_DrawBitmap(0, 160, splash, 128,160);
			titleDelay++;
		}
		
		while(titleDelay2 != 10){
			ST7735_DrawBitmap(0, 160, splashflash, 128,160);
			titleDelay2++;
		}
		
	};
	ST7735_FillScreen(0x0000);            // set screen to black
	setGrass();
	int count = 0;
	InitEnemy(&car1, 0, 140, 22, 20, car);
	InitEnemy(&truck1, 126,120, 35,15,truck);
	EnableInterrupts();
	SysTick_Init();
	ST7735_SetCursor(0,0);
	ST7735_OutString("Level 1!");
  while(1){
		
  }

}





int slow = 0;
int slow2 = 0;
long xcar_1 = 0;
long xtruck_1 = 128;
long xcar_2 = 25;
long xtruck_2 = 90;
long xcar_3 = 30;
void SysTick_Handler(void){
	//	GPIO_PORTF_DATA_R ^= 0x04;
	
	//  ST7735_DrawBitmap(x, y, frograss, 24,18); 
		ADC_In89(array);
	  slow2++;
		if(slow2 == 12){
			
			if(array[0] < 2000){                       //up
				if(CheckStreetToGrass() == 1){
					y-= 18;
					ST7735_DrawBitmap(x, y, frograss, 24,18);
					ST7735_FillRect(x,y,24,18,0x0000);
				}
				
				else if(CheckGrassToStreet()==1){
					y-=18;
					ST7735_DrawBitmap(x, y, frog1, 24,18);
					ST7735_DrawBitmap(x, y+18, GRASS, 24,18);
				}
				else if (CheckGrassToStreet() == 0 && CheckStreetToGrass() == 0){
					if(y < 35)															 
							;
					else{																					
							y-= 18;																			
							ST7735_DrawBitmap(x, y, frog1, 24,18);
							ST7735_FillRect(x,y,24,18,0x0000);
					}
				}
			}
			
			if(array[0] > 2500){                        //down
				if(CheckStreetToGrassDown() == 1){
					y+= 18;
					ST7735_DrawBitmap(x, y, frograss, 24,18);
					ST7735_FillRect(x,y-36,24,18,0x0000);
				}
				
				else if(CheckGrassToStreetDown()==1){
					y+=18;
					ST7735_DrawBitmap(x, y, frog1, 24,18);
					ST7735_DrawBitmap(x, y-18, GRASS, 24,18);
				}
				else {
					if(y > 150)															 
							;
					else{																					
							y+= 18;																			
							ST7735_DrawBitmap(x, y, frog1, 24,18);
							ST7735_FillRect(x,y-36,24,18,0x0000);
					}
				}
			}
		
			if(array[1] > 2500){                          //right
				if(x > 103)
					;
				else{
					x+=24;
					if(y == 160 || y == 106){
					ST7735_DrawBitmap(x, y, frograss, 24,18);
					ST7735_DrawBitmap(x-24, y, GRASS, 24,18);
					}
					else{
					ST7735_DrawBitmap(x, y, frog1, 24,18);
					ST7735_FillRect(x-24,y-18,24,18,0x0000);
					}
				}
			}
			if(array[1] < 2000){                          //left
				if(x < 5)
					;
				else{
					x-=24;
					if(y == 160 || y == 106){
					ST7735_DrawBitmap(x, y, frograss, 24,18);
					ST7735_DrawBitmap(x+24, y, GRASS, 24,18);
					}
					else{
					ST7735_DrawBitmap(x, y, frog1, 24,18);
					ST7735_FillRect(x+24,y-18,24,18,0x0000);
					}
				}
			}
			slow2 = 0;
		}
		
		slow++;
		if (slow == 5){
			MoveEnemy(&car1, 1);
			MoveEnemy(&truck1, -1);
			slow = 0;
		}
		
		if(CheckCollision(x,y,&car1)==1 || CheckCollision(x,y,&truck1)==1){
			ST7735_FillScreen(0x0000);            // set screen to black
			DisableInterrupts();
			ST7735_DrawBitmap(0, 160, gameoverscreen, 128,160);
			//ST7735_SetCursor(0,0);
			//ST7735_OutString("Level 1!");
			//setGrass();
			//x = 64;
		  //y= 160;
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

