/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "tim.h"
#include "usb_device.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usbd_cdc_if.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct
{
  char *data;
  uint32_t length;
} String;
typedef struct
{
  char *name;
  void (*handler)(int argc, char **argv);
  char *description;
} Command_t;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define STEP_PWM_PERIOD 1000
#define SERIAL_BUFFER_SIZE 256
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
static uint8_t serial_buffer[SERIAL_BUFFER_SIZE];
static uint32_t serial_buffer_index = 0;
static uint8_t serial_available_flag = 0;
uint8_t motor_speed = 0;
uint8_t stepper_enabled = 0;
uint8_t stepper_direction = 0;
uint32_t stepper_speed_hz = 100; // steps/seccond
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
uint8_t Serial_available(void);
int16_t Serial_read(void);
uint32_t Serial_readBytes(uint8_t *buffer, uint32_t length);
String Serial_readStringUntil(char terminator);
String Serial_readString(void);
int32_t Serial_parseInt(void);
float Serial_parseFloat(void);
// void Serial_print(const char *format, ...);
// void Serial_println(const char *format, ...);
// void Serial_print_int(int32_t value);
// void Serial_print_float(float value, uint8_t decimal_places);
// void Serial_println_void(void);
void Serial_flush(void);
void USB_CDC_RxHandler(uint8_t *Buf, uint32_t Len);
static void Stepper_Enable(void);
static void Stepper_Disable(void);
static void Stepper_Set_Direction(uint8_t dir);
static void Stepper_Set_Speed(uint32_t hz);
static void Stepper_Start(void);
static void Stepper_Stop(void);
// static void DC_Start(void);
// static void DC_Stop(void);
// static void DC_Set_Speed(uint8_t speed);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void USB_CDC_RxHandler(uint8_t *Buf, uint32_t Len)
{
  if (Len > 0)
  {
    for (uint32_t i = 0; i < Len; i++)
    {
      if (serial_buffer_index < SERIAL_BUFFER_SIZE)
      {
        serial_buffer[serial_buffer_index++] = Buf[i];
      }
    }
    serial_available_flag = 1;
  }
}
// Serial available - kiểm tra dữ liệu có sẵn
uint8_t Serial_available(void)
{
  return (serial_available_flag && (serial_buffer_index > 0));
}

static void Serial_write(uint8_t *data, uint16_t len)
{
  // gửi nếu USB rảnh, không thì bỏ
  if (CDC_Transmit_FS(data, len) == USBD_OK)
  {
  }
}
void Serial_print(const char *fmt, ...)
{
  char buffer[128];

  va_list args;
  va_start(args, fmt);
  vsnprintf(buffer, sizeof(buffer), fmt, args);
  va_end(args);

  Serial_write((uint8_t *)buffer, strlen(buffer));
}
void Serial_println(const char *fmt, ...)
{
  char buffer[128];

  va_list args;
  va_start(args, fmt);
  vsnprintf(buffer, sizeof(buffer), fmt, args);
  va_end(args);

  uint16_t len = strlen(buffer);

  // thêm newline
  buffer[len++] = '\r';
  buffer[len++] = '\n';
  buffer[len] = '\0';

  Serial_write((uint8_t *)buffer, len);
}

// Đọc 1 byte từ buffer
int16_t Serial_read(void)
{
  if (!Serial_available())
    return -1;

  uint8_t data = serial_buffer[0];

  // Dịch buffer lên
  for (uint32_t i = 0; i < serial_buffer_index - 1; i++)
  {
    serial_buffer[i] = serial_buffer[i + 1];
  }
  serial_buffer_index--;

  if (serial_buffer_index == 0)
  {
    serial_available_flag = 0;
  }

  return data;
}

// Đọc dữ liệu vào buffer
uint32_t Serial_readBytes(uint8_t *buffer, uint32_t length)
{
  uint32_t count = 0;
  while (count < length && Serial_available())
  {
    int16_t data = Serial_read();
    if (data >= 0)
    {
      buffer[count++] = (uint8_t)data;
    }
    else
    {
      break;
    }
  }
  return count;
}

// Đọc 1 dòng (đến \n hoặc \r\n)
String Serial_readStringUntil(char terminator)
{
  String result = {0};
  char temp[SERIAL_BUFFER_SIZE];
  uint32_t index = 0;

  while (index < SERIAL_BUFFER_SIZE - 1 && Serial_available())
  {
    int16_t c = Serial_read();
    if (c < 0)
      break;

    if (c == terminator || c == '\n')
    {
      if (c == '\n' && index > 0 && temp[index - 1] == '\r')
      {
        index--; // Bỏ qua \r
      }
      break;
    }

    temp[index++] = (char)c;
  }

  temp[index] = '\0';
  result.data = malloc(index + 1);
  if (result.data)
  {
    strcpy(result.data, temp);
    result.length = index;
  }

  return result;
}

// Đọc chuỗi (không có ký tự kết thúc)
String Serial_readString(void)
{
  return Serial_readStringUntil('\n');
}

// Parse string thành số nguyên
int32_t Serial_parseInt(void)
{
  String str = Serial_readStringUntil('\n');
  if (str.data)
  {
    int32_t value = atoi(str.data);
    free(str.data);
    return value;
  }
  return 0;
}

// Parse string thành số thực
float Serial_parseFloat(void)
{
  String str = Serial_readStringUntil('\n');
  if (str.data)
  {
    float value = atof(str.data);
    free(str.data);
    return value;
  }
  return 0;
}

// // In ra Serial (giống Serial.print)
// void Serial_print(const char *format, ...)
// {
//   char buffer[256];
//   va_list args;
//   va_start(args, format);
//   vsnprintf(buffer, sizeof(buffer), format, args);
//   va_end(args);

//   if (CDC_Transmit_FS((uint8_t *)buffer, strlen(buffer)) == USBD_OK){}
// }

// // In ra Serial có xuống dòng
// void Serial_println(const char *format, ...)
// {
//   char buffer[256];
//   va_list args;
//   va_start(args, format);
//   vsnprintf(buffer, sizeof(buffer), format, args);
//   va_end(args);

//   char output[260];
//   snprintf(output, sizeof(output), "%s\r\n", buffer);
//   if (CDC_Transmit_FS((uint8_t *)output, strlen(output)) == USBD_OK){}
// }

// // In số nguyên
// void Serial_print_int(int32_t value)
// {
//   char buffer[32];
//   snprintf(buffer, sizeof(buffer), "%ld", value);
//   if (CDC_Transmit_FS((uint8_t *)buffer, strlen(buffer)) == USBD_OK){}
// }

// // In số thực
// void Serial_print_float(float value, uint8_t decimal_places)
// {
//   char buffer[64];
//   snprintf(buffer, sizeof(buffer), "%.*f", decimal_places, value);
//   if (CDC_Transmit_FS((uint8_t *)buffer, strlen(buffer)) == USBD_OK){}
// }

// // In xuống dòng
// void Serial_println_void(void)
// {
//   if (CDC_Transmit_FS((uint8_t *)"\r\n", 2) == USBD_OK){}
// }

// Flush buffer
void Serial_flush(void)
{
  serial_buffer_index = 0;
  serial_available_flag = 0;
}
static void Stepper_Enable(void)
{
  stepper_enabled = 1;
  HAL_GPIO_WritePin(STEP_EN_GPIO_Port, STEP_EN_Pin, GPIO_PIN_RESET);
}
static void Stepper_Disable(void)
{
  stepper_enabled = 0;
  HAL_GPIO_WritePin(STEP_EN_GPIO_Port, STEP_EN_Pin, GPIO_PIN_SET);
  HAL_TIM_PWM_Stop(&htim4, TIM_CHANNEL_3);
}
static void Stepper_Set_Direction(uint8_t dir)
{
  stepper_direction = dir;
  HAL_GPIO_WritePin(STEP_DIR_GPIO_Port, STEP_DIR_Pin, dir ? GPIO_PIN_SET : GPIO_PIN_RESET);
}
static void Stepper_Set_Speed(uint32_t hz)
{
  if (hz > 0 && hz <= 10000) // Giới hạn 10kHz
  {
    stepper_speed_hz = hz;

    // Tính toán lại tần số PWM
    // Clock TIM4 = 72MHz / (Prescaler+1) = 72MHz / 72 = 1MHz
    // Period = 1MHz / (hz * 2) vì cần 2 xung PWM cho 1 step?
    // Thực tế: mỗi chu kỳ PWM = 1 step, nhưng cần 50% duty để có xung lên xuống
    // Nên tần số PWM = tần số step
    uint32_t period = 1000000 / hz; // 1MHz / hz

    if (period < 2)
      period = 2; // Giới hạn period tối thiểu

    __HAL_TIM_SET_AUTORELOAD(&htim4, period - 1);
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, (period - 1) / 2);
  }
}
static void Stepper_Start(void)
{
  if (stepper_enabled)
  {
    HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);
  }
}
static void Stepper_Stop(void)
{
  HAL_TIM_PWM_Stop(&htim4, TIM_CHANNEL_3);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET);
}

// Hàm split string thành các token
uint8_t split_command(char *command, char **argv, uint8_t max_args)
{
  uint8_t argc = 0;
  char *token = strtok(command, " ");

  while (token != NULL && argc < max_args)
  {
    argv[argc++] = token;
    token = strtok(NULL, " ");
  }

  return argc;
}

// Xử lý lệnh "help"
void cmd_help(int argc, char **argv)
{
  Serial_println("======= Available Commands =======");
  Serial_println("  help              - Show this help");
  Serial_println("  status            - Show motor status");
  Serial_println("  step on/off       - Enable/disable stepper motor");
  Serial_println("  step dir [0/1]    - Set stepper direction (0=CCW, 1=CW)");
  Serial_println("  step speed [hz]   - Set stepper speed (1-10000 Hz)");
  Serial_println("  step start/stop   - Start/stop stepper motor");
  Serial_println("  dc speed [0-255]  - Set DC motor speed (PWM value)");
  Serial_println("  dc ch [1-3] [val] - Set DC motor channel value");
  Serial_println("  echo [text]       - Echo back text");
  Serial_println("======= Available Commands =======");
}

// Xử lý lệnh "status"
void cmd_status(int argc, char **argv)
{
  Serial_println("======= Motor Status =======");
  Serial_print("Stepper Enabled: ");
  Serial_println(stepper_enabled ? "YES" : "NO");
  Serial_print("Stepper Direction: ");
  Serial_println(stepper_direction ? "CW" : "CCW");
  Serial_print("Stepper Speed: ", stepper_speed_hz);
  // Serial_print(stepper_speed_hz);
  Serial_println(" Hz");
  Serial_print("Stepper PWM State: ");
  Serial_println(HAL_TIM_PWM_GetState(&htim4) == HAL_TIM_STATE_READY ? "Stopped" : "Running");
  Serial_println("======= Motor Status =======");
}

// Xử lý lệnh "step"
void cmd_step(int argc, char **argv)
{
  if (argc < 2)
  {
    Serial_println("Usage: step [on/off/dir/speed/start/stop]");
    return;
  }

  if (strcmp(argv[1], "on") == 0)
  {
    Stepper_Enable();
    Serial_println("Stepper enabled");
  }
  else if (strcmp(argv[1], "off") == 0)
  {
    Stepper_Disable();
    Serial_println("Stepper disabled");
  }
  else if (strcmp(argv[1], "dir") == 0 && argc >= 3)
  {
    uint8_t dir = atoi(argv[2]);
    Stepper_Set_Direction(dir);
    Serial_print("Stepper direction set to ");
    Serial_println(dir ? "CW" : "CCW");
  }
  else if (strcmp(argv[1], "speed") == 0 && argc >= 3)
  {
    uint32_t speed = atoi(argv[2]);
    Stepper_Set_Speed(speed);
    Serial_print("Stepper speed set to ", speed);
    // Serial_print(speed);
    Serial_println(" Hz");
  }
  else if (strcmp(argv[1], "start") == 0)
  {
    Stepper_Start();
    Serial_println("Stepper started");
  }
  else if (strcmp(argv[1], "stop") == 0)
  {
    Stepper_Stop();
    Serial_println("Stepper stopped");
  }
  else if (strcmp(argv[1], "ms") == 0 && argc >= 4)
  {
    uint8_t ms_pin = atoi(argv[2]); // 1 hoặc 2
    uint8_t state = atoi(argv[3]);  // 0 hoặc 1

    if (state > 1)
    {
      Serial_println("Invalid state (0 or 1)");
      return;
    }

    GPIO_PinState gpio_state = state ? GPIO_PIN_SET : GPIO_PIN_RESET;

    switch (ms_pin)
    {
    case 1:
      HAL_GPIO_WritePin(STEP_MS1_GPIO_Port, STEP_MS1_Pin, gpio_state);
      Serial_println("MS1 set to %d", state);
      break;

    case 2:
      HAL_GPIO_WritePin(STEP_MS2_GPIO_Port, STEP_MS2_Pin, gpio_state);
      Serial_println("MS2 set to %d", state);
      break;

    default:
      Serial_println("Invalid MS pin (1 or 2)");
      break;
    }
  }
  else
  {
    Serial_println("Invalid step subcommand");
  }
}

// Xử lý lệnh "dc"
void cmd_dc(int argc, char **argv)
{
  if (argc < 2)
  {
    Serial_println("Usage: dc [speed/ch/start/stop] ...");
    return;
  }

  // ===== dc ch =====
  else if (strcmp(argv[1], "ch") == 0 && argc >= 4)
  {
    uint8_t channel = atoi(argv[2]);
    uint32_t value = atoi(argv[3]);

    if (value > 999)
      value = 999;

    switch (channel)
    {
    case 1:
      __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_3, value);
      Serial_println("Channel 1 set to %lu", value);
      break;

    case 2:
      __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_2, value);
      Serial_println("Channel 2 set to %lu", value);
      break;

    case 3:
      __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_1, value);
      Serial_println("Channel 3 set to %lu", value);
      break;

    default:
      Serial_println("Invalid channel (1-3)");
    }
  }

  // ===== dc start =====
  else if (strcmp(argv[1], "start") == 0 && argc >= 3)
  {
    uint8_t channel = atoi(argv[2]);

    switch (channel)
    {
    case 1:
      HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_3);
      Serial_println("Channel 1 STARTED");
      break;

    case 2:
      HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_2);
      Serial_println("Channel 2 STARTED");
      break;

    case 3:
      HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_1);
      Serial_println("Channel 3 STARTED");
      break;

    default:
      Serial_println("Invalid channel (1-3)");
    }
  }

  // ===== dc stop =====
  else if (strcmp(argv[1], "stop") == 0 && argc >= 3)
  {
    uint8_t channel = atoi(argv[2]);

    switch (channel)
    {
    case 1:
      HAL_TIM_PWM_Stop(&htim8, TIM_CHANNEL_3);
      Serial_println("Channel 1 STOPPED");
      break;

    case 2:
      HAL_TIM_PWM_Stop(&htim8, TIM_CHANNEL_2);
      Serial_println("Channel 2 STOPPED");
      break;

    case 3:
      HAL_TIM_PWM_Stop(&htim8, TIM_CHANNEL_1);
      Serial_println("Channel 3 STOPPED");
      break;

    default:
      Serial_println("Invalid channel (1-3)");
    }
  }

  else
  {
    Serial_println("Invalid dc command");
  }
}

// Xử lý lệnh "echo"
void cmd_echo(int argc, char **argv)
{
  if (argc >= 2)
  {
    for (int i = 1; i < argc; i++)
    {
      Serial_print("%s", argv[i]);
      if (i < argc - 1)
        Serial_print(" ");
    }
    if (CDC_Transmit_FS((uint8_t *)"\r\n", 2) == USBD_OK)
    {
    }
  }
}

// Danh sách các lệnh
Command_t commands[] = {
    {"help", cmd_help, "Show help"},
    {"status", cmd_status, "Show status"},
    {"step", cmd_step, "Control stepper motor"},
    {"dc", cmd_dc, "Control DC motor"},
    {"echo", cmd_echo, "Echo text"},
    {NULL, NULL, NULL}};

// Tìm và thực thi lệnh
void process_command(char *command_str)
{
  // Xóa ký tự xuống dòng
  char *newline = strchr(command_str, '\n');
  if (newline)
    *newline = '\0';
  newline = strchr(command_str, '\r');
  if (newline)
    *newline = '\0';

  if (strlen(command_str) == 0)
    return;

  char *argv[10];
  uint8_t argc = split_command(command_str, argv, 10);

  if (argc == 0)
    return;

  // Tìm và thực thi lệnh
  for (int i = 0; commands[i].name != NULL; i++)
  {
    if (strcmp(argv[0], commands[i].name) == 0)
    {
      commands[i].handler(argc, argv);
      return;
    }
  }

  // Không tìm thấy lệnh
  Serial_print("Unknown command: ");
  Serial_println("%s", argv[0]);
  Serial_println("Type 'help' for available commands");
}

// Hàm xử lý chính cho USB commands (gọi trong main loop)
void Serial_process_commands(void)
{
  if (Serial_available())
  {
    String cmd = Serial_readString();
    if (cmd.data)
    {
      process_command(cmd.data);
      free(cmd.data);
    }
  }
}

// Hàm tiện ích: đọc số từ Serial (blocking với timeout)
int32_t Serial_readInt_timeout(uint32_t timeout_ms)
{
  uint32_t start = HAL_GetTick();
  String str = {0};
  char temp[32];
  uint32_t index = 0;

  while ((HAL_GetTick() - start) < timeout_ms)
  {
    if (Serial_available())
    {
      int16_t c = Serial_read();
      if (c >= 0)
      {
        if ((c >= '0' && c <= '9') || c == '-' || (c == '.' && index > 0))
        {
          if (index < 31)
            temp[index++] = (char)c;
        }
        else if (c == '\n' || c == ' ')
        {
          break;
        }
      }
    }
  }

  temp[index] = '\0';
  return atoi(temp);
}

// Hàm đọc chuỗi từ Serial (non-blocking)
uint8_t Serial_readString_nonblocking(char *buffer, uint32_t max_len)
{
  static uint32_t last_time = 0;
  static uint32_t index = 0;

  if (Serial_available())
  {
    int16_t c = Serial_read();
    if (c >= 0)
    {
      if (c == '\n' || c == '\r')
      {
        buffer[index] = '\0';
        index = 0;
        return 1;
      }
      else if (index < max_len - 1)
      {
        buffer[index++] = (char)c;
      }
    }
  }

  return 0;
}
/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USB_DEVICE_Init();
  MX_TIM4_Init();
  MX_TIM8_Init();
  /* USER CODE BEGIN 2 */
  for (int i = 0; i < 20; i++)
  {
    HAL_GPIO_TogglePin(LED_PC13_GPIO_Port, LED_PC13_Pin);
    HAL_Delay(80);
  }
  HAL_GPIO_WritePin(LED_PC13_GPIO_Port, LED_PC13_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(STEP_EN_GPIO_Port, STEP_EN_Pin, GPIO_PIN_SET);

  HAL_GPIO_WritePin(STEP_MS1_GPIO_Port, STEP_MS1_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(STEP_MS2_GPIO_Port, STEP_MS2_Pin, GPIO_PIN_SET);
  // MS2/MS1 = 0/0 => MicroStep 1/8
  // MS2/MS1 = 0/1 => MicroStep 1/32
  // MS2/MS1 = 1/0 => MicroStep 1/64
  // MS2/MS1 = 1/1 => MicroStep 1/16

  HAL_TIM_PWM_Stop(&htim8, TIM_CHANNEL_1);
  HAL_TIM_PWM_Stop(&htim8, TIM_CHANNEL_2);
  HAL_TIM_PWM_Stop(&htim8, TIM_CHANNEL_3);

  // Stepper_Start();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    Serial_process_commands();
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  Period elapsed callback in non blocking mode
 * @note   This function is called  when TIM2 interrupt took place, inside
 * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
 * a global variable "uwTick" used as application time base.
 * @param  htim : TIM handle
 * @retval None
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM2)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
