// BranchingFunctionsDelays.c Lab 6
// Runs on LM4F120/TM4C123
// Use simple programming structures in C to 
// toggle an LED while a button is pressed and 
// turn the LED on when the button is released.  
// This lab will use the hardware already built into the LaunchPad.
// Daniel Valvano, Jonathan Valvano
// January 8, 2014

// built-in connection: PF0 connected to negative logic momentary switch, SW2
// built-in connection: PF1 connected to red LED
// built-in connection: PF2 connected to blue LED
// built-in connection: PF3 connected to green LED
// built-in connection: PF4 connected to negative logic momentary switch, SW1

#include "TExaS.h"

#define GPIO_PORTF_DATA_R       (*((volatile unsigned long *)0x400253FC))
#define GPIO_PORTF_DIR_R        (*((volatile unsigned long *)0x40025400))
#define GPIO_PORTF_AFSEL_R      (*((volatile unsigned long *)0x40025420))
#define GPIO_PORTF_PUR_R        (*((volatile unsigned long *)0x40025510))
#define GPIO_PORTF_DEN_R        (*((volatile unsigned long *)0x4002551C))
#define GPIO_PORTF_AMSEL_R      (*((volatile unsigned long *)0x40025528))
#define GPIO_PORTF_PCTL_R       (*((volatile unsigned long *)0x4002552C))
#define SYSCTL_RCGC2_R          (*((volatile unsigned long *)0x400FE108))
#define SYSCTL_RCGC2_GPIOF      0x00000020  // port F Clock Gating Control
#define GPIO_PORTF_LOCK_R       (*((volatile unsigned long *)0x40025520))
#define GPIO_PORTF_CR_R         (*((volatile unsigned long *)0x40025524))
	
// basic functions defined at end of startup.s
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void initPortF(unsigned long volatile delay)
{
	/*
	//found in discussion forum - where is it in the documentation?
	SYSCTL_RCGC2_R |= 0x00000020; // activate clock for Port F
	//allow time for clock to start (found in discussion forum):
	delay = SYSCTL_RCGC2_R;    // force flush of ram write buffer
	*/
	//Note - causes compiler WARNING - delay set but not used
	//However - on discussion forum they said:
	/* Discovered that I cannot change the variable "delay" 
	   to something else like "ffrwb" because TExaS won't
	   go without finding and monitoring the variable 
	   "delay". */
	//Someone else said in response:
	/*You're turning on the clock AFTER you try to 
	  initialize the PORTF registers.  You need to turn 
	  on the clock FIRST, before you can even write to 
	  the control registers.
		FWIW, I did "delay = 0; delay = 0; 
	  // pause a few cycles", because I didn't like the 
	     mystery of reading the register I had just 
	     written.  It seems to work fine.
	*/
	/*Someone else wrote they simple did as I did above
	  so that is what I will try.
   */
/* my original init port F 
	GPIO_PORTF_AMSEL_R &= ~(0x10); //Clear PF4 bit in AMSEL to disable analog
	GPIO_PORTF_AMSEL_R &= ~(0x04); //Clear PF2 bit in AMSEL to disable analog
	GPIO_PORTF_PCTL_R &= ~(0x10); //Clear PF4 bit in GPIO to configure as GPIO
	GPIO_PORTF_PCTL_R &= ~(0x04); //Clear PF2 bit in GPIO to configure as GPIO
	GPIO_PORTF_DIR_R |= 0x04; // PortF,Bit2 = output (LED) in direction register
	GPIO_PORTF_DIR_R &= ~(0x10); //PortF,Bit4 = input (switch1) in direction reg
	GPIO_PORTF_AFSEL_R &= ~(0x10); //Clear PF4 bit in AFSEL to disable alternate functions
	GPIO_PORTF_AFSEL_R &= ~(0x04); //Clear PF2 bit in AFSEL to disable alternate functions
  	GPIO_PORTF_DEN_R |= 0x14; //PortF,Bits 4 and 2 enabled digital
  	GPIO_PORTF_PUR_R |= 0x14; //PortF,Bit 4 PUR=Pull up Resistor activated
	GPIO_PORTF_DATA_R |= 0x04; //PortF,Bit2 LED starts ON
*/
  SYSCTL_RCGC2_R |= 0x00000020;     // 1) F clock
  delay = SYSCTL_RCGC2_R;           // delay   
  GPIO_PORTF_LOCK_R = 0x4C4F434B;   // 2) unlock PortF PF0  
  GPIO_PORTF_CR_R = 0x1F;           // allow changes to PF4-0       
  GPIO_PORTF_AMSEL_R = 0x00;        // 3) disable analog function
  GPIO_PORTF_PCTL_R = 0x00000000;   // 4) GPIO clear bit PCTL  
  GPIO_PORTF_DIR_R = 0x0E;          // 5) PF4,PF0 input, PF3,PF2,PF1 output   
  GPIO_PORTF_AFSEL_R = 0x00;        // 6) no alternate function
  GPIO_PORTF_PUR_R = 0x11;          // enable pullup resistors on PF4,PF0       
  GPIO_PORTF_DEN_R = 0x1F;          // 7) enable digital pins PF4-PF0

}

void Delay100ms(unsigned long numOf100msDelays)
{
	unsigned long i;
	while (numOf100msDelays > 0)
	{
		i = 1333333; //this number means 100ms
		while (i > 0)
		{
			i = i - 1;
		}
		//decrements every 100ms
		numOf100msDelays = numOf100msDelays - 1; 
	}
}

int main(void){ 
	unsigned long volatile delay;
	unsigned long numOf100msDelays = 1;
	unsigned long dataRegIn = 0;
	
  TExaS_Init(SW_PIN_PF4, LED_PIN_PF2);  // activate grader and set system clock to 80 MHz
  // initialization goes here
  initPortF(delay);
	
  EnableInterrupts();           // enable interrupts for the grader
  while(1){
    // body goes here
		//Delay 100 ms
		Delay100ms(numOf100msDelays);
		
		//If switch pressed (PF4=0), toggle LED once, else turn LED ON
		if ((GPIO_PORTF_DATA_R & 0x10) == 0) 
			// PF4=0, (Negative Logic, Switch is Pressed)
			//toggle LED once (PF2 XORed ^ with opposite state) (Positive Logic LED so On=1 for PF2)
		{
			dataRegIn = GPIO_PORTF_DATA_R & 0x04;
			if (dataRegIn == 0) //PF2 = 0 = LED off
			{ //LED off, toggle LED once (turn it on)
				GPIO_PORTF_DATA_R |= 0x04; //turn LED on
			}
			else //LED on already
			{ //LED on, toggle LED once (turn off)
				GPIO_PORTF_DATA_R &= ~(0x04); //turn LED off
			}
		}
		else //PF4=1, (Negative Logic, Switch NOT Pressed)
				 //so turn LED on (PF2 = 1 = LED ON, Positive Logic)
		{
				GPIO_PORTF_DATA_R |= 0x04; //turn LED on
		}
  }
}
