#include "motor.h"

#define F_STEP_MIN 10
#define F_STEP_MAX 400
#define MAX_FREQ 2000

unsigned char counter = 0;
static uint8_t last_sw1 = 1;
static uint8_t last_sw2 = 1;
extern uint32_t frequency;
extern volatile double amplitude;
extern volatile double threshold_amp;
extern unsigned int motor_reload;
extern volatile motor_dir_t motor_dir;

void motor_gpio_init(void)
{
	SYSCTL->RCGCGPIO |= (1<<1); // Initialize clock for GPIOB
	__ASM("NOP");
	__ASM("NOP");
	__ASM("NOP");
	
	GPIOB->DIR |= (0xF<<0); // Output: PB3-PB0
	GPIOB->DEN |= (0xF<<0); // Digital Enable: PB3-PB0
	GPIOB->AFSEL &= ~(0xF<<0); // No Alternate Function: PB3-PB0
	GPIOB->DATA &= ~(0xF<<0); // Clear output Data: PB3-PB0
}

void motor_timer_init(void)
{
	SYSCTL->RCGCTIMER |= (1<<2); // Initialize clock for TIMER2
	__ASM("NOP");
	__ASM("NOP"); 
	__ASM("NOP");

	TIMER2->CTL &= ~(1<<0);            // disable Timer2A during config
	TIMER2->CFG = 0x04;                 // 16-bit timer
	TIMER2->TAMR = (0x02<<0)|(0<<4);   // periodic mode, count down
	TIMER2->TAILR = 999999;
	TIMER2->TAPR = 15;                  // prescaler = 16

	TIMER2->ICR = 0x1;                 // clear timeout flag
	TIMER2->IMR |= 0x1;                // interrupt mask register

	// NVIC enable for Timer1A (IRQ 21)
	NVIC->IPR[23] = 0x70;              // priority (optional)
	NVIC->ISER[0] = (1<<23);           // enable IRQ 21

	TIMER2->CTL |= (1<<0);             // enable Timer1A 
	// END of TIMER Config for TIMER1
}

void motor_init(void)
{
	motor_gpio_init();
	motor_timer_init();
}

unsigned int freq_to_reload(void)
{
	unsigned int f_step = F_STEP_MIN + 
		                    ((frequency * (F_STEP_MAX - F_STEP_MIN)) / 2000);
	
	return (unsigned int)((1000000/f_step) - 1);
}

void set_out(unsigned char step_no)
{
	if(step_no == 0) 
	{
		GPIOB->DATA &= ~0x0F;
		GPIOB->DATA |= 0x08;
	}
	else if(step_no == 1) 
	{
		GPIOB->DATA &= ~0x0F;
		GPIOB->DATA |= 0x04;
	}
	else if(step_no == 2) 
	{
		GPIOB->DATA &= ~0x0F;
		GPIOB->DATA |= 0x02;
	}
	else if(step_no == 3) 
	{
		GPIOB->DATA &= ~0x0F;
		GPIOB->DATA |= 0x01;
	}
	else ; // do nothing
}

void update_motor_direction(void)
{
    uint8_t sw1 = (GPIOF->DATA & (1<<4)) != 0; // 1 = released
    uint8_t sw2 = (GPIOF->DATA & (1<<0)) != 0;

    // Detect falling edge (released -> pressed)
    if (last_sw1 && !sw1)
    {
        motor_dir = MOTOR_CCW;
        counter = 0;     // reset step index for clean reversal
    }
    else if (last_sw2 && !sw2)
    {
        motor_dir = MOTOR_CW;
        counter = 0;
    }

    last_sw1 = sw1;
    last_sw2 = sw2;
}


void TIMER2A_Handler(void)
{
	if(motor_dir == MOTOR_CCW)
	{
		set_out(3-counter);
	}
	else if(motor_dir == MOTOR_CW)
	{
		set_out(counter);
	}
	
	// Update counter
	counter += 1;
	if (counter == 4) counter = 0;
	if(amplitude > threshold_amp) TIMER2->TAILR = motor_reload;
	TIMER2->ICR = 0x1;                 // clear timer interrupt
}