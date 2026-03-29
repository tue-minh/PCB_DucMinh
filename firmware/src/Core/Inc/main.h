/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED_PC13_Pin GPIO_PIN_13
#define LED_PC13_GPIO_Port GPIOC
#define DCMotor_PWM3_Pin GPIO_PIN_6
#define DCMotor_PWM3_GPIO_Port GPIOC
#define DCMotor_PWM2_Pin GPIO_PIN_7
#define DCMotor_PWM2_GPIO_Port GPIOC
#define DCMotor_PWM1_Pin GPIO_PIN_8
#define DCMotor_PWM1_GPIO_Port GPIOC
#define STEP_EN_Pin GPIO_PIN_5
#define STEP_EN_GPIO_Port GPIOB
#define STEP_MS1_Pin GPIO_PIN_6
#define STEP_MS1_GPIO_Port GPIOB
#define STEP_MS2_Pin GPIO_PIN_7
#define STEP_MS2_GPIO_Port GPIOB
#define STEP_PWM_Pin GPIO_PIN_8
#define STEP_PWM_GPIO_Port GPIOB
#define STEP_DIR_Pin GPIO_PIN_9
#define STEP_DIR_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
