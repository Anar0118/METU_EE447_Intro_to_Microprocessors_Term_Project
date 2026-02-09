#include "TM4C123GH6PM.h"

void threshold_range(int num, volatile double* amp);
void threshold_range_quantized(int adc, volatile double *threshold);
void threshold_sampling(volatile int *data); // not used
void TIMER1A_Handler(void);
void update_pwm(void);
void switch_init(void);