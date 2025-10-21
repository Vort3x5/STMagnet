#ifndef SSD1306_H
#define SSD1306_H

#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <string.h>

#define SSD1306_I2C_ADDR    (0x3C << 1)

#define SSD1306_WIDTH       128
#define SSD1306_HEIGHT      64

#define SSD1306_BLACK       0
#define SSD1306_WHITE       1

uint8_t SSD1306_Init(I2C_HandleTypeDef *hi2c);
void SSD1306_Fill(uint8_t color);
void SSD1306_UpdateScreen(void);
void SSD1306_DrawPixel(uint8_t x, uint8_t y, uint8_t color);
void SSD1306_GotoXY(uint8_t x, uint8_t y);
char SSD1306_Putc(char ch, uint8_t color);
void SSD1306_Puts(char *str, uint8_t color);
void SSD1306_DrawLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t color);
void SSD1306_DrawRectangle(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color);
void SSD1306_DrawFilledRectangle(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color);
void SSD1306_DrawCircle(uint8_t x0, uint8_t y0, uint8_t r, uint8_t color);
void SSD1306_Clear(void);

void SSD1306_Printf(uint8_t x, uint8_t y, uint8_t color, const char *format, ...);

#endif
