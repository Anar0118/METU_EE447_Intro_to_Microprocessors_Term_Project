/*Pulse_init.h file
Function for creating a pulse train using interrupts
Uses Channel 0, and a 1Mhz Timer clock (_TAPR = 15)
Uses Timer0A to create pulse train on PF2
*****************************************************
Notes on using different on-board LEDs
-----------------------------------------------------
*****************BLUE LED****************************
GPIO PIN: PF2 
*****************GREEN LED***************************
GPIO PIN: PF3
*****************RED LED*****************************
GPIO PIN: PF1
*****************************************************
*/
#include "TM4C123GH6PM.h"
void pulse_init(void);
void TIMER0A_Handler (void);

extern volatile int HIGH;
extern volatile int LOW;


extern volatile double amplitude;
extern volatile double threshold_amp;
extern uint32_t frequency;
extern volatile uint32_t LOW_FREQ;
extern volatile uint32_t HIGH_FREQ;


int blue = 2;
int green = 3;
int red = 1;

void pulse_init(void){
	volatile int *NVIC_EN0 = (volatile int*) 0xE000E100;
	volatile int *NVIC_PRI4 = (volatile int*) 0xE000E410;
	SYSCTL->RCGCGPIO 	|= 0x20; 								// Turn on bus clock for GPIOF
	__ASM("NOP");
	__ASM("NOP");
	__ASM("NOP");
	
	
	GPIOF->DIR			|= ((1<<red)|(1<<green)|(1<<blue)); 						// Set PF1 PF2 PF3 as output
	GPIOF->AFSEL		&= ~((1<<red)|(1<<green)|(1<<blue));  					// Regular port function
	GPIOF->PCTL			&= ~((0x0F<<(4*red))|(0x0F<<(4*green))|(0x0F<<(4*blue)));  		                    // No alternate function
	GPIOF->AMSEL		= 0; 									  		                    // Disable analog
	GPIOF->DEN			|= ((1<<red)|(1<<green)|(1<<blue)); 						// Enable PF1 PF2 PF3 pin as digital
	
	SYSCTL->RCGCTIMER	|= 0x01; 							// Start timer0
	__ASM("NOP");
	__ASM("NOP");
	__ASM("NOP");
	TIMER0->CTL			&= 0xFFFFFFFE; 					// Disable timer during setup
	TIMER0->CFG			= 0x04;  								// Set 16 bit mode
	TIMER0->TAMR		= 0x02; 								// set to periodic, count down
	TIMER0->TAILR		= LOW; 									// Set interval load as LOW
	TIMER0->TAPR		= 16; 									// Divide the clock by 16 to get 1us
	TIMER0->IMR			= 0x01; 								// Enable timeout interrupt	
	
	// Timer0A is interrupt 19
	// Interrupt 16-19 are handled by NVIC register PRI4
	// Interrupt 19 is controlled by bits 31:29 of PRI4
	*NVIC_PRI4 &= 0x00FFFFFF; 									// Clear interrupt 19 priority
	*NVIC_PRI4 |= 0x60000000; 									// Set interrupt 19 priority to 3
	
	// NVIC has to be enabled
	// Interrupts 0-31 are handled by NVIC register EN0
	// Interrupt 19 is controlled by bit 19
	*NVIC_EN0 |= 0x00080000;
	
	// Enable timer
	TIMER0->CTL	|= 0x03; 						// bit 0 to enable and bit 1 to stall on debug
	return;
}

void TIMER0A_Handler(void){
	// Implement
	TIMER0->ICR |= 0x01; 	// Clear the interrupt
	// LEDS OFF STATE
	if(amplitude < threshold_amp)
	{
		GPIOF->DATA &= ~((1<<red)|(1<<green)|(1<<blue)); // Turn OFF all leds
		// Clear the timer0A timeout interrupt
		//TIMER0->ICR |= 0x01; 	// Clear the interrupt
		return;
	}
	
	// RED LED ON STATE
	if(frequency < LOW_FREQ)
	{
		GPIOF->DATA &= ~((1<<green)|(1<<blue)); // Turn OFF BLUE and GREEN
		
		// TOGGLE RED 
		GPIOF->DATA ^= (1<<red);
	
		// Choose next interval depending on the new state of PF1
		if (GPIOF->DATA & (1<<red)) 
		{
			// PF1 is now HIGH -> set timer for HIGH duration
			TIMER0->TAILR = HIGH;
		} else 
		{
			// PF1 is now LOW -> set timer for LOW duration
			TIMER0->TAILR = LOW;
		}

		// Clear the timer0A timeout interrupt
		//TIMER0->ICR |= 0x01; 	// Clear the interrupt
		return;
	}
	
	// GREEN LED ON STATE
	if(frequency >= LOW_FREQ && frequency < HIGH_FREQ)
	{
		GPIOF->DATA &= ~((1<<red)|(1<<blue)); // Turn OFF RED and BLUE
		
		// TOGGLE GREEN 
		GPIOF->DATA ^= (1<<green);
	
		// Choose next interval depending on the new state of PF3
		if (GPIOF->DATA & (1<<green)) 
		{
			// PF3 is now HIGH -> set timer for HIGH duration
			TIMER0->TAILR = HIGH;
		} else 
		{
			// PF3 is now LOW -> set timer for LOW duration
			TIMER0->TAILR = LOW;
		}

		// Clear the timer0A timeout interrupt
		//TIMER0->ICR |= 0x01; 	// Clear the interrupt
		return;
	}
	
	// BLUE LED ON STATE
	if(frequency >= HIGH_FREQ)
	{
		GPIOF->DATA &= ~((1<<red)|(1<<green)); // Turn OFF RED and GREEN
		
		// TOGGLE BLUE 
		GPIOF->DATA ^= (1<<blue);
	
		// Choose next interval depending on the new state of PF2
		if (GPIOF->DATA & (1<<blue)) 
		{
			// PF2 is now HIGH -> set timer for HIGH duration
			TIMER0->TAILR = HIGH;
		} else 
		{
			// PF2 is now LOW -> set timer for LOW duration
			TIMER0->TAILR = LOW;
		}

		// Clear the timer0A timeout interrupt
		//TIMER0->ICR |= 0x01; 	// Clear the interrupt
		return;
	}
}