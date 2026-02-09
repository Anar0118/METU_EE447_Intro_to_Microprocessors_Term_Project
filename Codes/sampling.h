#include "TM4C123GH6PM.h"
#include "arm_math.h"
#include "arm_const_structs.h"

#define FFT_N    		512
#define FS_HZ 			4000
#define FFT_LEN_Q15 (2 * FFT_N)

extern uint16_t ADC0_SS3_ReadRaw(void);
extern int16_t  ADC0_SS3_ReadCentered(void);

void SysTick_Handler(void);
void fft(void);
void calculate_dominant(const q15_t *fft_output,
												uint16_t *dominant_index,
												uint32_t *dominant_power,
												uint32_t *dominant_freq);