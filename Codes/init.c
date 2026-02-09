#include "init.h"

#define TIMER1A_RELOAD_50HZ 319999U // 50 Hz for 16 MHz clock

// Global Variables
volatile int systick_reload_value = 999; // 4000 Hz - 250 usec (0.25 msec) : systick uses 4MHz clock


void gpio_init(void)
{
	// Implement
	// Initialize GPIOB for PB4
	SYSCTL->RCGCGPIO |= (1<<1); // Enable clock for Port B
	__ASM("NOP");
	__ASM("NOP");
	__ASM("NOP");
	GPIOB->DIR &= ~(1<<4); 			// Clear bit 4: Set PB4 as input pin
	GPIOB->AFSEL |= (1<<4);			// Enable Alternate Function on PB4
	GPIOB->DEN &= ~(1<<4);			// Disable Digital Enable on PB4
	GPIOB->AMSEL |= (1<<4);			// Enable Analog Mode on PB4
	// END of GPIO Config for GPIOB for PB4
	
	// Initialize GPIOB for PB5
	GPIOB->DIR &= ~(1<<5); 			// Clear bit 5: Set PB5 as input pin
	GPIOB->AFSEL |= (1<<5);			// Enable Alternate Function on PB5
	GPIOB->DEN &= ~(1<<5);			// Disable Digital Enable on PB5
	GPIOB->AMSEL |= (1<<5);			// Enable Analog Mode on PB5
	// END of GPIO Config for GPIOB for PB5

}

void atd_init(void)
{
	// Implement
	// Initialize ATD0 for PB4
	SYSCTL->RCGCADC |= (1<<0);    			  // Enable clock for ADC0
	while((SYSCTL->PRADC & (1<<0)) == 0); // wait for clock to stabilize

	ADC0->ACTSS &= ~(1<<3);             // Disable Sample Sequencer 3
	ADC0->EMUX &= ~(0xF<<12);						// Clear bit 15-12 of SS3: software defined sampling
	ADC0->SSMUX3 |= 0xA;								// Input MUL Select: writing 0xA to section 0 to indicate PB4 pin
	ADC0->SSCTL3 |= (1<<2);							// Setting IE0
	ADC0->SSCTL3 |= (1<<1);							// Setting END0
	ADC0->PC = 0x01;										// 125 ksps
	ADC0->ACTSS |= (1<<3);              // Enable Sample Sequencer 3
	// END of ATD Config for ATD0 for PB4
	
	// Initialize ATD1 for PB5
	SYSCTL->RCGCADC |= (1<<1);    			  // Enable clock for ADC0
	while((SYSCTL->PRADC & (1<<1)) == 0); // wait for clock to stabilize

	ADC1->ACTSS &= ~(1<<3);             // Disable Sample Sequencer 3
	ADC1->EMUX &= ~(0xF<<12);						// Clear bit 15-12 of SS3: software defined sampling
	ADC1->SSMUX3 |= 0xB;								// Input MUL Select: writing 0xB to section 0 to indicate PB5 pin
	ADC1->SSCTL3 |= (1<<2);							// Setting IE0
	ADC1->SSCTL3 |= (1<<1);							// Setting END0
	ADC1->PC = 0x01;										// 125 ksps
	ADC1->ACTSS |= (1<<3);              // Enable Sample Sequencer 3
	// END of ATD Config for ATD1 for PB5
}

void timer_init(void)
{
	// Implement
	// Initialize TIMER1
	SYSCTL->RCGCTIMER |= (1<<1);      // enable Timer1 clock
	__ASM("NOP");
	__ASM("NOP"); 
	__ASM("NOP");

	TIMER1->CTL &= ~(1<<0);            // disable Timer1A during config
	TIMER1->CFG = 0x0;                 // 32-bit timer
	TIMER1->TAMR = (0x02<<0)|(0<<4);   // periodic mode, count down
	TIMER1->TAILR = TIMER1A_RELOAD_50HZ;
	TIMER1->TAPR = 0;                  // prescaler = 0

	TIMER1->ICR = 0x1;                 // clear timeout flag
	TIMER1->IMR |= 0x1;                // interrupt mask register

	// NVIC enable for Timer1A (IRQ 21)
	NVIC->IPR[21] = 0x40;              // priority (optional)
	NVIC->ISER[0] = (1<<21);           // enable IRQ 21

	TIMER1->CTL |= (1<<0);             // enable Timer1A 
	// END of TIMER Config for TIMER1
}

void systick_init(void)
{
	// Implement
	SysTick->CTRL = 0;										// disable systick
	SysTick->LOAD = systick_reload_value; // systick reload value
	NVIC_SetPriority(SysTick_IRQn, 0);
	SysTick->CTRL = 3; 										// enable interrupt; enable systick
}