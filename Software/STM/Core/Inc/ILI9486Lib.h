/*
 * ILI9486Lib.h
 *
 *  Created on: Apr 27, 2025
 *      Author: Krzymbolix
 */

#include "stm32l4xx.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef INC_ILI9486LIB_H_
#define INC_ILI9486LIB_H_

#define DC1_Pin GPIO_PIN_14
#define DC1_GPIO_Port GPIOB
#define RST1_Pin GPIO_PIN_13
#define RST1_GPIO_Port GPIOB
#define CS1_Pin GPIO_PIN_12
#define CS1_GPIO_Port GPIOB
#define LED1_Pin GPIO_PIN_9
#define LED1_GPIO_Port GPIOB

extern const uint8_t graphics_Niedzielni[164472];
extern const uint8_t graphics_Konfident[39732];

void ILI_SendCommand(uint8_t command);
void ILI_SendParam(uint8_t param);
void ILI_Init(void);
void ILI_DrawPixel(uint16_t x, uint16_t y, uint32_t color);
void ILI_CLS(void);
void ILI_WriteChar(uint16_t x, uint16_t y, char c, uint32_t color);
void ILI_WriteBigChar(uint16_t x, uint16_t y, char c, uint32_t color);
void ILI_WriteString(uint16_t x, uint16_t y, const char* text, uint32_t color);
void ILI_WriteBigString(uint16_t x, uint16_t y, const char* text, uint32_t color);
void ILI_Invert(void);
void ILI_DrawPicture(uint16_t start_x, uint16_t start_y, const uint8_t* picture, uint16_t dim_x, uint16_t dim_y);
void ILI_DrawBlackSector(uint16_t start_x, uint16_t start_y, uint16_t dim_x, uint16_t dim_y);
void ILI_DrawWhiteSector(uint16_t start_x, uint16_t start_y, uint16_t dim_x, uint16_t dim_y);
void ILI_DrawBattery(void);
void ILI_DrawInnerBattery(void);
void ILI_SmallCLS(void);
void ILI_LEDOFF(void);
void ILI_LEDON(void);

#endif /* INC_ILI9486LIB_H_ */
