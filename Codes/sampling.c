#include "sampling.h"

volatile int sampled_data = 0;
volatile int32_t centered = 0;
const arm_cfft_instance_q15 *S = &arm_cfft_sR_q15_len512;


extern q15_t sample_array[];
extern volatile int update_index;
extern volatile char array_full_flag;

volatile uint32_t systick_count = 0;
volatile uint8_t lcd_update_flag = 0;

void SysTick_Handler(void)
{
	// Implement
	systick_count++;
	if (systick_count >= 4000) 
	{   
		// 4000 Hz -> 1 second
		systick_count = 0;
		lcd_update_flag = 1;
	}
	
	if(array_full_flag == 0){  //array_full_flag == 0
		ADC0->PSSI |= (1<<3);              				 // Process Sample Sequence 3 Initialize
		while((ADC0->RIS & (1<<3)) == 0){} 				 // wait for interrupt for SS3
		sampled_data = ADC0_SS3_ReadRaw();		 // Store Sampling value to data
		centered = sampled_data - 2048;  	 				 // remove DC offset
		ADC0->ISC |= (1<<3); 							 				 // Clear Interrupt to continue sampling
		sample_array[update_index] = (q15_t)(centered << 4);
		sample_array[update_index+1] = 0;	
		update_index += 2;
		if(update_index >= 1024)
		{
			array_full_flag = 1;
			update_index = 0;
		}
	}
}

void fft(void)
{
	// Implement
	arm_cfft_q15(S,sample_array,0,1);
}

void calculate_dominant(const q15_t *fft_output,
												uint16_t *dominant_index,
												uint32_t *dominant_power,
												uint32_t *dominant_freq)
{
	// Implement
	uint32_t max_power = 0;
	uint16_t max_index =1; // start from 1 to skip DC
	
	// Check only first half since second half is complex conjugate
	for(uint16_t index = 1; index < (FFT_N/2); index++)
	{
		int32_t real_part = fft_output[2 * index];        // real part
		int32_t imag_part = fft_output[2 * index + 1];    // imaginary part
		
		uint32_t power = (uint32_t)(real_part*real_part + imag_part*imag_part);
		
		if(power > max_power)
		{
			max_power = power;
			max_index = index;
		}
	}
	// End of for loop: found dominat frequency index and power of magnitude
	uint32_t freq = (max_index * FS_HZ) / FFT_N;
	
	if (dominant_index)      *dominant_index = max_index;
	if (dominant_power)      *dominant_power = max_power;
	if (dominant_freq)       *dominant_freq = freq;
}