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
//#include "Sound.h"
//#include "Timer0.h"
#include "Print.h"
//#include "DAC.h"


void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void Delay100ms(uint32_t count); // time delay in 0.1 seconds



const unsigned short wave[31] = {8,9,11,12,13,14,14,15,15,14,14,13,12,11,9,8,7,5,4,3,2,2,1,1,1,2,2,3,4,5,7}; //4-bit sine wave table
uint32_t waveCount = 0;
int soundC = 0;
void (*PeriodicTask)(void);
	
// **************DAC_Init*********************
// Initialize 4-bit DAC, called once 
// Input: none
// Output: none
void Timer1_Init(/*uint32_t period*/){
	volatile int delay;
  SYSCTL_RCGCTIMER_R |= 0x02;   // 0) activate TIMER1
	delay++;
	delay++;
	delay++;
	delay++;	
  //PeriodicTask = task;          // user function
  TIMER1_CTL_R = 0x00000000;    // 1) disable TIMER1A during setup
  TIMER1_CFG_R = 0x00000000;    // 2) configure for 32-bit mode
  TIMER1_TAMR_R = 0x00000002;   // 3) configure for periodic mode, default down-count settings
  //TIMER1_TAILR_R = 4000;         //period-1;    // 4) reload value
  TIMER1_TAPR_R = 0;            // 5) bus clock resolution
  TIMER1_ICR_R = 0x00000001;    // 6) clear TIMER1A timeout flag
  TIMER1_IMR_R = 0x00000001;    // 7) arm timeout interrupt
  NVIC_PRI5_R = (NVIC_PRI5_R&0xFFFF00FF)|0x00008000; // 8) priority 4
// interrupts enabled in the main program after all devices initialized
// vector number 37, interrupt number 21
  NVIC_EN0_R = 1<<21;           // 9) enable IRQ 21 in NVIC
  TIMER1_CTL_R = 0x00000001;    // 10) enable TIMER1A
}

/*void DAC_Init(void){
	SYSCTL_RCGCGPIO_R |= 0x02;
	volatile uint32_t count;
	count++;
	GPIO_PORTB_DEN_R |= 0x0F;
	GPIO_PORTB_DIR_R |= 0x0F;
	
} */

// **************DAC_Out*********************
// output to DAC
// Input: 4-bit data, 0 to 15 
// Input=n is converted to n*3.3V/15
// Output: none

/*void DAC_Out(uint32_t data){
	GPIO_PORTB_DATA_R &= 0x0;
	GPIO_PORTB_DATA_R |= (0x01 & data);
	GPIO_PORTB_DATA_R |= (0x02 & data);
	GPIO_PORTB_DATA_R |= (0x04 & data);
	GPIO_PORTB_DATA_R |= (0x08 & data);
} */
void DAC_Out(uint32_t data)
{
	GPIO_PORTB_DATA_R &= 0xF0;
	GPIO_PORTB_DATA_R|= (data&0x0F);
 }	

int ptr=0;
int count=50;
void Sound_Handler(void)
{
	DAC_Out(wave[ptr]);
	if(ptr==31)
	{
		count--;
		if(count==0){
			//Timer1_Init(1000);
					Timer1_Init();
			count=50;
		}
	}
	ptr=(ptr+1)%32;
}

void DAC_Init(void){
	SYSCTL_RCGCGPIO_R |= 0x02;
	volatile int nop;
	nop++;
	nop++;
	GPIO_PORTB_DEN_R|=0x0F;
	GPIO_PORTB_DIR_R|=(0x0F);											//initialize PB0-3 as outputs for the DAC
	
	//Timer1_Init(4257);
	//	Timer1_Init();
}


/*
void Sound_Play(uint32_t period){ //const uint8_t *pt, uint32_t count)
	if(period == 0) {
		GPIO_PORTB_DATA_R = 0;
		TIMER0_TAILR_R = 0;
	}
	else
		TIMER0_TAILR_R = period;  //update systick interrupt period
};
*/

void Timer1A_Handler(void){
  TIMER1_ICR_R = TIMER_ICR_TATOCINT;// acknowledge timer0A timeout
	DAC_Out(wave[ptr]);
	if(soundC > 1000){
		if(ptr==31)
		{
			count--;
			if(count==0){
				//Timer1_Init(1000);
				count=50;
			}
		}
		ptr=(ptr+1)%32;
		soundC--;
	}
}

/*
void Note(void){
	//Sound_Play(A);
	DAC_Init();
	GPIO_PORTF_DATA_R ^= 0x04;
	DAC_Out(wave[waveCount]);
	waveCount = (waveCount + 1) % 30;
}
*/

uint32_t array[2];
long x = 64;
long y = 160;
int level = 1;
int endGame = 0;
int winGame = 0;
int lives = 3;
int levelWait = 0;

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
	else if (space < 0 && init->x_pos <= 5){
		init->x_pos = 128;
		ST7735_FillRect(0,107,37,15,0x0000);
		ST7735_FillRect(0,53,37,15,0x0000);
	}
	init->x_pos = (init->x_pos + space);
	ST7735_DrawBitmap(init->x_pos, init->y_pos, init->image, init->x_size, init->y_size);
}


//CheckCollision
//returns 0 if no collision detected
//returns 1 if a collision is detected
int CheckCollision(uint32_t px, uint32_t py, enemy_t* check){
	if((px+12) > check->x_pos && (px+12) < (check->x_pos+check->x_size) && (py-9) < check->y_pos && (py-9) > (check->y_pos-check->y_size))
		return 1;
	else 
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
enemy_t car2;
enemy_t truck2;
enemy_t car3;
enemy_t car4;
enemy_t car5;
enemy_t car6;

void SysTick_Init(void){
	NVIC_ST_CTRL_R = 0;                   // disable SysTick during setup
  NVIC_ST_RELOAD_R = 0x0013D620;  // maximum reload value for 60MHZ
  NVIC_ST_CURRENT_R = 0;                // any write to current clears it
  NVIC_ST_CTRL_R = 0x07; // enable SysTick with core clock and interrupts
}


void PortF_Init(void){
	
	volatile int delay;
	SYSCTL_RCGCGPIO_R |= 0x20;     // 1) activate clock for Port F
  delay = SYSCTL_RCGC2_R;           // allow time for clock to start
	delay++;
  GPIO_PORTF_LOCK_R = 0x4C4F434B;   // 2) unlock GPIO Port F
  GPIO_PORTF_CR_R = 0x1F;           // allow changes to PF4-0
  // only PF0 needs to be unlocked, other bits can't be locked
  GPIO_PORTF_AMSEL_R = 0x00;        // 3) disable analog on PF
  GPIO_PORTF_PCTL_R = 0x00000000;   // 4) PCTL GPIO on PF4-0
  GPIO_PORTF_DIR_R = 0x0E;          // 5) PF4,PF0 in, PF3-1 out
  GPIO_PORTF_AFSEL_R = 0x00;        // 6) disable alt funct on PF7-0
  GPIO_PORTF_PUR_R = 0x11;          // enable pull-up on PF0 and PF4
  GPIO_PORTF_DEN_R = 0x1F;          // 7) enable digital I/O on PF4-0
	
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

void finishLevelorDeadRestart(void){
	    DisableInterrupts();
			ST7735_SetCursor(0,6);
			ST7735_OutString("Ready?");
			ST7735_SetCursor(0,7);
			ST7735_OutString("Yes or No?");
			while (!(((GPIO_PORTA_DATA_R & 0x10) == 0x10) || (GPIO_PORTF_DATA_R & 0x10) == 0x00))
			{}; 
			/*while((GPIO_PORTA_DATA_R & 0x10) == 0 || (GPIO_PORTD_DATA_R & 0x01) == 0x0){
				int titleDelay = 0;
				int titleDelay2 = 0;
				while(titleDelay != 10){
					titleDelay++;
				}
		
				while(titleDelay2 != 10){
					titleDelay2++;
				}
			}
			*/
			if((GPIO_PORTA_DATA_R & 0x10) == 0x10){
				x = 64; 
				y = 160;
				levelWait = 0;
				ST7735_FillRect(0,36,60,50,0x0000);
				SysTick_Init();
				EnableInterrupts();
			}
			
			if((GPIO_PORTF_DATA_R & 0x10) == 0x00){
				endGame = 1;
				ST7735_FillScreen(0x00);
				ST7735_DrawBitmap(0, 160, gameoverscreen, 128,160);
				DisableInterrupts();
			}
}

int main(void){
  PLL_Init(Bus80MHz);       // Bus clock is 80 MHz 
	DisableInterrupts();
	//Timer1_Init(4000);
  Timer1_Init();
	DAC_Init();
  Random_Init(1);
  Output_Init();
  //ST7735_InitR(INITR_REDTAB);
  ADC_Init89();
	PortF_Init();
	EnableInterrupts();
	TIMER1_TAILR_R = 4000;
	ST7735_FillScreen(0x0000);            // set screen to black
	ST7735_DrawBitmap(0, 160, splash, 128,160);
	//DAC_Init();
 // Timer1_Init(4000);
	//TIMER0_TAILR_R = 0;
	
	while((GPIO_PORTA_DATA_R & 0x10) == 0){
		int titleDelay = 0;
		int titleDelay2 = 0;
		while(titleDelay != 10){
			ST7735_DrawBitmap(0, 160, splash, 128,160);
			titleDelay++;
		}
		
		while(titleDelay2 != 10){
			ST7735_DrawBitmap(0, 160, splashflash1, 128,160);
			titleDelay2++;
		}
	} 
	
	levelWait = 1;
	DisableInterrupts();
	ST7735_FillScreen(0x0000);            // set screen to black
	setGrass();
	ST7735_DrawBitmap(95, 10, frogheart, 11,9);
	ST7735_DrawBitmap(106, 10, frogheart, 11,9);
  ST7735_DrawBitmap(117, 10, frogheart, 11,9);
	int count = 0;
	InitEnemy(&car1,     0, 141, 22, 18, car);
	InitEnemy(&truck1, 126,123, 35,15,truck);
	InitEnemy(&car2,     0, 86, 22, 20, car);
	InitEnemy(&truck2, 126,66, 35,15,truck);
	InitEnemy(&car3,     0, 51, 22, 20, car);
	//new cars
	InitEnemy(&car4, 50, 141, 22, 18, car);
	InitEnemy(&car5, 65, 51, 22, 20, car);
	InitEnemy(&car6, 90, 86, 22, 20, car);
	ST7735_DrawBitmap(64, 160, frograss, 24,18);
	//EnableInterrupts();
	//SysTick_Init();
	ST7735_SetCursor(0,0);
	ST7735_OutString("Level 1!");
  while(1){
		if(endGame == 1){
			ST7735_FillScreen(0x0000);            // set screen to black
			DisableInterrupts();
			ST7735_DrawBitmap(0, 160, gameoverscreen, 128,160);
			if((GPIO_PORTA_DATA_R & 0x10) == 0x10){
				ST7735_FillScreen(0x0000);            // set screen to black
				ST7735_FillScreen(0x0000);            // set screen to black
				ST7735_FillScreen(0x0000);            // set screen to black
				setGrass();
				ST7735_DrawBitmap(64, 160, frograss, 24,18);
				ST7735_SetCursor(0,0);
				ST7735_OutString("Level 1!");
				ST7735_DrawBitmap(95, 10, frogheart, 11,9);
				ST7735_DrawBitmap(106, 10, frogheart, 11,9);
				ST7735_DrawBitmap(117, 10, frogheart, 11,9);
				x = 64;
				y = 160;
				level = 1;
				endGame = 0;
				lives = 3;
				EnableInterrupts();
			}
		}
		else if (winGame == 1){
			ST7735_FillScreen(0x0000);            // set screen to black
			DisableInterrupts();
			ST7735_DrawBitmap(0, 160, youwin, 128,160);
			if((GPIO_PORTA_DATA_R & 0x10) == 0x10){
				ST7735_FillScreen(0x0000);            // set screen to black
				ST7735_FillScreen(0x0000);            // set screen to black
				ST7735_FillScreen(0x0000);            // set screen to black
				setGrass();
				ST7735_DrawBitmap(64, 160, frograss, 24,18);
				ST7735_SetCursor(0,0);
				ST7735_OutString("Level 1!");
				ST7735_DrawBitmap(95, 10, frogheart, 11,9);
				ST7735_DrawBitmap(106, 10, frogheart, 11,9);
				ST7735_DrawBitmap(117, 10, frogheart, 11,9);
				x = 64;
				y = 160;
				level = 1;
				winGame = 0;
				lives = 3;
				EnableInterrupts();
			}
		}
		
		else if (levelWait == 1){
			finishLevelorDeadRestart();
		}
		
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
	  
		ADC_In89(array);
	
	  slow2++;
		if(slow2 == 10){
			
			if(array[0] < 1700){   															//up
			soundC = 2000;
				if(CheckStreetToGrass() == 1){
					y-= 18;
					ST7735_DrawBitmap(x, y, frograss, 24,18);
					ST7735_FillRect(x,y,24,18,0x0000);
					if(y < 35){
						if(level == 1){
							level = 2;
							ST7735_SetCursor(0,0);
							ST7735_OutString("Level 2!");
							levelWait = 1;
						}
						else if (level == 2){
							level = 3;
							ST7735_SetCursor(0,0);
							ST7735_OutString("Level 3!");
							levelWait = 1;
						}
						else if (level == 3){
							winGame = 1;
						}
						x = 64;
						y = 160;
						ST7735_DrawBitmap(x, y, frograss, 24,18);
					}
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
			
			else if(array[0] > 2700){   
				soundC = 2000;													//down
				if(CheckStreetToGrassDown() == 1){
					y+= 18;
					ST7735_DrawBitmap(x, y, frograss, 24,18);
					ST7735_FillRect(x,y-36,24,18,0x0000);
				}
				
				else if(CheckGrassToStreetDown()==1){
					y+=18;
					ST7735_DrawBitmap(x, y, frog1, 24,18);
					ST7735_DrawBitmap(x, y-18, GRASS, 24,18);
				//	DAC_Init();
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
		
			else if(array[1] > 2700){
				soundC = 2000;															//right
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
			else if(array[1] < 1400){
				soundC = 2000;																//left
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
		
		
		if(level == 1){
				MoveEnemy(&car1, 1);
				MoveEnemy(&truck1, -1);
		}
		
		 if(level == 2){
				MoveEnemy(&car1, 1);
				MoveEnemy(&truck1, -2);
			  MoveEnemy(&car2, 1);
			  MoveEnemy(&truck2, -2);
			  MoveEnemy(&car3, 2);
		}
		
		 if(level == 3){
				MoveEnemy(&car1, 2);
				MoveEnemy(&truck1, -2);
		   	MoveEnemy(&car2, 2);
		   	MoveEnemy(&truck2, -2);
		  	MoveEnemy(&car3, 2);
			  MoveEnemy(&car4, 2);
			  MoveEnemy(&car5, 2);
			  MoveEnemy(&car6, 2);
		}
		
		if((CheckCollision(x,y,&car1)==1 ||CheckCollision(x,y,&truck1)==1) && levelWait == 0){
			lives--;
			if(lives == 2){
				ST7735_FillRect(95,1,11,9,0x0000);
				x = 64;
				y = 160;
			  ST7735_DrawBitmap(x, y, frograss, 24,18);
				finishLevelorDeadRestart();
			}
			else if (lives == 1){
				ST7735_FillRect(106,1,11,9,0x0000);
				x = 64;
				y = 160;
			  ST7735_DrawBitmap(x, y, frograss, 24,18);
				finishLevelorDeadRestart();
			}
			else if (lives == 0)
				endGame = 1;
		}
		
		
		else if(((CheckCollision(x,y,&car2)==1 || CheckCollision(x,y,&truck2)==1)&&level==2) && levelWait == 0){
			lives--;
			if(lives == 2){
				ST7735_FillRect(95,1,11,9,0x0000);
				x = 64;
				y = 160;
			  ST7735_DrawBitmap(x, y, frograss, 24,18);
				finishLevelorDeadRestart();
			}
			else if (lives == 1){
				ST7735_FillRect(106,1,11,9,0x0000);
				x = 64;
				y = 160;
			  ST7735_DrawBitmap(x, y, frograss, 24,18);
				finishLevelorDeadRestart();
			}
			else if (lives == 0)
				endGame = 1;
		}
		
		else if(((CheckCollision(x,y,&car2)==1 || CheckCollision(x,y,&truck2)==1 || CheckCollision(x,y,&car3))&&(level==3 || level ==2)) && levelWait == 0){
			lives--;
			if(lives == 2){
				ST7735_FillRect(95,1,11,9,0x0000);        //clear a life
				x = 64;
				y = 160;
			  ST7735_DrawBitmap(x, y, frograss, 24,18);
				finishLevelorDeadRestart();
			}
			else if (lives == 1){
				ST7735_FillRect(106,1,11,9,0x0000);       //clear a life
				x = 64;
				y = 160;
			  ST7735_DrawBitmap(x, y, frograss, 24,18);
				finishLevelorDeadRestart();
			}
			else if (lives == 0)
				endGame = 1;
		}
		
		else if(((CheckCollision(x,y,&car4)==1 || CheckCollision(x,y,&car5)==1 || CheckCollision(x,y,&car6)==1)&&level==3) && levelWait == 0){
			lives--;
			if(lives == 2){
				ST7735_FillRect(95,1,11,9,0x0000);        //clear a life
				x = 64;
				y = 160;
			  ST7735_DrawBitmap(x, y, frograss, 24,18);
				finishLevelorDeadRestart();
			}
			else if (lives == 1){
				ST7735_FillRect(106,1,11,9,0x0000);       //clear a life
				x = 64;
				y = 160;
			  ST7735_DrawBitmap(x, y, frograss, 24,18);
				finishLevelorDeadRestart();
			}
			else if (lives == 0)
				endGame = 1;
		}
		
}

