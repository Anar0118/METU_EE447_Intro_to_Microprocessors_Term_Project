#pragma once
#include <stdint.h>

void lcd_init(void);
void lcd_clear(void);
void lcd_set_xy(uint8_t x, uint8_t y);   // x:0..83, y:0..5
void lcd_char(char c);
void lcd_print_uint(uint32_t v);
void lcd_print_amp(double a);
void lcd_print(const char *s);