/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "pn532_stm32f1.h"
#include "pn532.h"
#include "stdio.h"
#include "time.h"
#include <stdlib.h>
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
float getBatteryVoltage(void);

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define BTN3_Pin GPIO_PIN_0
#define BTN3_GPIO_Port GPIOA
#define BAT_Pin GPIO_PIN_1
#define BAT_GPIO_Port GPIOA
#define RSTOUT_Pin GPIO_PIN_2
#define RSTOUT_GPIO_Port GPIOA
#define PN_IRQ_Pin GPIO_PIN_3
#define PN_IRQ_GPIO_Port GPIOA
#define PN_IRQ_EXTI_IRQn EXTI3_IRQn
#define NSS_Pin GPIO_PIN_4
#define NSS_GPIO_Port GPIOA
#define BTN2_Pin GPIO_PIN_0
#define BTN2_GPIO_Port GPIOB
#define BTN2_EXTI_IRQn EXTI0_IRQn
#define BTN1_Pin GPIO_PIN_1
#define BTN1_GPIO_Port GPIOB
#define BTN1_EXTI_IRQn EXTI1_IRQn
#define PN_RST_Pin GPIO_PIN_2
#define PN_RST_GPIO_Port GPIOB
#define CS_Pin GPIO_PIN_12
#define CS_GPIO_Port GPIOB
#define RST_Pin GPIO_PIN_13
#define RST_GPIO_Port GPIOB
#define DC_Pin GPIO_PIN_14
#define DC_GPIO_Port GPIOB
#define BTN4_Pin GPIO_PIN_12
#define BTN4_GPIO_Port GPIOA
#define BTN4_EXTI_IRQn EXTI15_10_IRQn
#define LED_Pin GPIO_PIN_9
#define LED_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */
#define BUZZER_ON __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 24);
#define BUZZER_OFF __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);
#define BATTERY_DIVIDER_ON  Set_Pin_Low(GPIOB, GPIO_PIN_5);
#define BATTERY_DIVIDER_OFF Set_Pin_HiZ(GPIOB, GPIO_PIN_5);

extern volatile uint8_t response_ready;
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
