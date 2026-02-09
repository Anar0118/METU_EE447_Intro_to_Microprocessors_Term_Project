#include "pot_led_switch.h"

#define MAX_THRESHOLD   5000.0
#define NUM_LEVELS      1000
#define PWM_PERIOD_US   10000

extern volatile int data;
extern volatile double threshold_amp;
//extern volatile int pot_sample_request;
extern volatile double amplitude;
extern volatile int HIGH;
extern volatile int LOW;

void threshold_range(int num, volatile double* amp)
{
	if(amp) *amp = ((float)num / 4095.0f) * 20000;
}

void threshold_range_quantized(int adc, volatile double *threshold)
{
    if (!threshold) return;

    int level = (adc * NUM_LEVELS) / 4096;  // 0 .. 31
    if (level >= NUM_LEVELS) level = NUM_LEVELS - 1;

    *threshold = (level * MAX_THRESHOLD) / (NUM_LEVELS - 1);
}

void threshold_sampling(volatile int *data)
{
	// Implementation
	ADC1->PSSI |= (1<<3);              // Process Sample Sequence 3 Initialize
	while((ADC1->RIS & (1<<3)) == 0){} // wait for interrupt for SS3
	*data = ADC1->SSFIFO3;							 // Store Sampling value to data
	ADC1->ISC |= (1<<3); 							 // Clear Interrupt to continue sampling
}

void TIMER1A_Handler(void)
{
	// Implement
	threshold_sampling(&data);
	threshold_range_quantized(data, &threshold_amp);
	
	TIMER1->ICR = 0x1;                 // clear timer interrupt
}

void update_pwm(void)
{
	double temp = amplitude;
	if (temp < 0) temp = 0;
	if (temp > 20000) temp = 20000;

	int high_us = (int)(temp * PWM_PERIOD_US / 20000.0);
	int low_us  = PWM_PERIOD_US - high_us;

	// Avoid 0 (prevents interrupt storm / weird behavior)
	if (high_us < 50) high_us = 50;
	if (low_us  < 50) low_us  = 50;

	HIGH = high_us;
	LOW  = low_us;
}

void switch_init(void)
{
	SYSCTL->RCGCGPIO |= (1<<5);   // GPIOF clock
	__ASM("NOP"); __ASM("NOP"); __ASM("NOP");

	// Unlock PF0
	GPIOF->LOCK = 0x4C4F434B;
	GPIOF->CR |= (1<<0);

	// PF4 (SW1), PF0 (SW2) inputs
	GPIOF->DIR   &= ~((1U<<4) | (1U<<0));
	GPIOF->DEN   |=  ((1U<<4) | (1U<<0));
	GPIOF->AFSEL &= ~((1U<<4) | (1U<<0));
	GPIOF->AMSEL &= ~((1U<<4) | (1U<<0));
	GPIOF->PCTL  &= ~((0xFU<<(4*4)) | (0xFU<<(4*0)));

	// Make inputs stable
	GPIOF->PUR   |=  ((1U<<4) | (1U<<0));  // enable pull-ups
}