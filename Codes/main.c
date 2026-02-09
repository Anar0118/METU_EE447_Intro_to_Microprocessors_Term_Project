#include "stdio.h"
#include "init.h"
#include "sampling.h"
#include "pot_led_switch.h"
#include "Pulse_init.h"
#include "screen.h"
#include "motor.h"

extern void IOInit(void);
extern void OutStr(char*);

extern volatile int sampled_data;
extern volatile int32_t centered;

//static int print_div = 0;

//************************************
//Variables for Sampling and FFT
q15_t sample_array[1024] = {0};
volatile int update_index = 0;
volatile char array_full_flag = 0;

uint16_t index;
uint32_t power;
uint32_t frequency;
volatile double amplitude;
//************************************

//************************************
//Variables for Threshold
volatile int data = 0;
//volatile int pot_sample_request = 0;
volatile double threshold_amp = 0;

volatile int HIGH = 0;
volatile int LOW = 99;

volatile uint32_t LOW_FREQ = 500;
volatile uint32_t HIGH_FREQ = 1500;
//************************************


//************************************
//Variables for Printing
char msg[80];
char new_line[]= "\r\4";
//************************************

unsigned int motor_reload = 999999;
volatile motor_dir_t motor_dir = MOTOR_CCW;	

extern volatile uint8_t lcd_update_flag;

int main()
{
	// Setup
	IOInit();
	gpio_init();
	atd_init();
	timer_init();
	systick_init();
	pulse_init();
	switch_init();
	
	lcd_init();
	lcd_clear();
	
	motor_init();
	//__enable_irq();

	
	//while loop
	while(1)
	{
		// Implement
		update_motor_direction();
		
		if(array_full_flag)
		{
			fft();
			calculate_dominant(sample_array, &index, &power, &frequency);
			amplitude = sqrt(power);
			/*
			if (++print_div >= 20) {
				print_div = 0;
				sprintf(msg, "Freq:%d Hz | Amplitude:%.3f | Threshold:%.3f\4", frequency
																																		, amplitude
																																		, threshold_amp);
				OutStr(msg);
				OutStr(new_line);
			}
			*/
			update_pwm();
			array_full_flag = 0;
		}
		
		motor_reload = freq_to_reload();
		
		if (lcd_update_flag)
		{
			lcd_update_flag = 0;

			lcd_clear();
			lcd_set_xy(0,0);
			lcd_print("F: ");
			lcd_print_uint(frequency);
			lcd_print(" HZ");

			lcd_set_xy(0,1);
			lcd_print("A: ");
			lcd_print_amp(amplitude);

			lcd_set_xy(0,2);
			lcd_print("AT:");
			lcd_print_amp(threshold_amp);
			
			lcd_set_xy(0,3);
			lcd_print("FMIN: ");
			lcd_print_uint(LOW_FREQ);
			
			lcd_set_xy(0,4);
			lcd_print("FMAX: ");
			lcd_print_uint(HIGH_FREQ);
		}
	}
}