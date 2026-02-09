#pragma once
#include "TM4C123GH6PM.h"

typedef enum
{
	MOTOR_CW = 0,
	MOTOR_CCW = 1
} motor_dir_t;


extern volatile motor_dir_t motor_dir;

void motor_init(void);
unsigned int freq_to_reload(void);
void update_motor_direction(void);