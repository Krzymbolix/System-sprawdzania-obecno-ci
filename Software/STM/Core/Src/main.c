/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ILI9486Lib.h"
#include "database.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

RTC_HandleTypeDef hrtc;

SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef hspi2;

TIM_HandleTypeDef htim1;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_RTC_Init(void);
static void MX_SPI2_Init(void);
static void MX_TIM1_Init(void);
static void MX_ADC1_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
Nauczyciel Nauczyciele[2];
Student Studenci[4];
Terminarz Terminarze[32];
uint8_t buff[255];
uint8_t uid[10];
uint8_t key_a[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t rtccounter = 0;
uint32_t pn532_error = PN532_ERROR_NONE;
char list[50][9] = {0};
uint8_t list_len = 0;
int32_t uid_len = 0;
PN532 pn532;
volatile uint8_t response_ready = 0;
time_t timestamp = 0;
uint16_t lesson_ids[10] = {0};
void* nauczyciel = NULL;
uint16_t wybrana_lekcja = 0;
uint16_t lessons_len = 0;
uint16_t id_lekcji = 0;
volatile uint32_t last_button_time = 0;
uint8_t status = 0;	// 1 jeżeli czeka na tak nauczyciela, 2 jeżeli czeka na wybór listy, 3 jeżeli czeka na tagi studentów lub zatwierdzenie listy, 0 dla reszty

uint8_t rx_buffer[128];
char* msg = "Wiadomość z STM32\r\n";

void buzzer(void)
{
	BUZZER_ON
	HAL_Delay(3000);
	BUZZER_OFF
}

void Set_Pin_HiZ(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;      // tryb wejścia = Hi-Z
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}

void Set_Pin_Low(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_RESET); // ustaw na LOW przed zmianą trybu!

    GPIO_InitStruct.Pin = GPIO_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;   // push-pull output
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}

void sendList(void)
{
	  char buff[50] = {0};
	  char ack[4] = {0};
	  char len[4] = {0};
	  char ack_server[45] = {0};
	  uint8_t id_1 = 1;
	  uint8_t id_2 = 2;
	  sprintf(buff, "/4/[\"%d\",\"%d\"]", id_1, id_2);
	  ILI_WriteString(3,160,buff,0);
//	  printf("Buff: %s\r\n", buff);
	  uint8_t len_num = strlen(buff);
//	  printf("len_num: %d\r\n", len_num);
	  sprintf(len, "%d", len_num);
//	  printf("len: %s\r\n", len);
	  ILI_WriteString(3,180,len,0);
	  HAL_UART_Transmit(&huart1, (uint8_t *)len, strlen(len), HAL_MAX_DELAY);
	  HAL_StatusTypeDef status3 = HAL_UART_Receive(&huart1, (uint8_t *)ack, 3, HAL_MAX_DELAY);
	  if (status3 == 0 && strstr(ack, "ACK") != NULL){
//		  printf("Otrzymano ACK\r\n");
		  ILI_WriteString(3,200,"Otrzymano ACK",0);
		  HAL_UART_Transmit(&huart1, (uint8_t *)buff, len_num, HAL_MAX_DELAY);
		  HAL_StatusTypeDef status4 = HAL_UART_Receive(&huart1, (uint8_t *)ack, 3, HAL_MAX_DELAY);
		  if (status4 == 0 && strstr(ack, "ACK") != NULL){
//				  printf("Otrzymano ACK\r\n");
			  ILI_WriteString(3,220,"Otrzymano ACK",0);
				HAL_StatusTypeDef status5 = HAL_UART_Receive(&huart1, (uint8_t *)ack_server, 42, HAL_MAX_DELAY);
//				printf("ack_server: %s", ack_server);
				ILI_WriteString(3,240,ack_server, 0);
		  }
	  }
}

float getBatteryVoltage(void)
{
//	BUZZER_ON
//	HAL_Delay(500);
//	BUZZER_OFF
	uint32_t adc_value = 0;
	float voltage = 0.0f;
	BATTERY_DIVIDER_ON
	HAL_Delay(10);
    HAL_ADC_Start(&hadc1);                              // Uruchom ADC
    if (HAL_ADC_PollForConversion(&hadc1, 100) == HAL_OK) // Czekaj na zakończenie konwersji
    {
        adc_value = HAL_ADC_GetValue(&hadc1);           // Pobierz wartość
        voltage = ((float)adc_value / 4095.0f) * 3.3f;  // Zakładając napięcie referencyjne 3.3V
    }
    HAL_ADC_Stop(&hadc1);
    HAL_Delay(10);
    BATTERY_DIVIDER_OFF

	HAL_Delay(500);
    voltage = (8.0/3.0)*voltage;
//    BUZZER_OFF
    return voltage;
}


//int __io_putchar(int ch)
//{
//	char buffer[20];
////    HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
//    sprintf(buffer, "%d", ch);
//    ILI_WriteString(410,100,buffer,0);
//    return ch;
//}

void uart_test(void)
{
	char buffer[11], response[13];
	sprintf(buffer, "0%s", ((Nauczyciel*)nauczyciel)->uuid);
//	__HAL_UART_FLUSH_DRREGISTER(&huart1); // dla HAL
//	__HAL_UART_CLEAR_OREFLAG(&huart1);
//	__HAL_UART_CLEAR_FEFLAG(&huart1);
//	__HAL_UART_CLEAR_NEFLAG(&huart1);
	HAL_UART_Transmit(&huart1, (uint8_t*)buffer, 15, HAL_MAX_DELAY);
	HAL_StatusTypeDef status;
	status = HAL_UART_Receive(&huart1, (uint8_t*)response, 13, HAL_MAX_DELAY);
	response[12] = '\0';
	if (status == 0)
	{
		ILI_WriteString(50,50,response,0);
	}
}

void PN532_IRQ_Teacher_Handler(void)
{
	BUZZER_ON
	HAL_Delay(500);
	BUZZER_OFF
	if (uid_len == PN532_STATUS_ERROR) {
	} else {
//		uart_test();
		char buffer[9];
		char bufor[50];
		uint8_t i = 0;
		for (i = 0; i < uid_len; i++)
		{
			sprintf(&buffer[i*2], "%02x", uid[i]);
		}
		buffer[i*2] = '\0';
		nauczyciel = findTeacherId(buffer);
		ILI_SmallCLS();
		if (nauczyciel == NULL)
		{
			ILI_WriteBigString(40,195,"Zeskanowany tag nie",0xFF0000);
			ILI_WriteBigString(60,225,"należy do żadnego",0xFF0000);
			ILI_WriteBigString(50,255,"pracownika uczelni",0xFF0000);
			return;
		}
		ILI_DrawPicture(110,80,((Nauczyciel*)nauczyciel)->zdjecie,100,125);
		ILI_WriteBigString(3,280,((Nauczyciel*)nauczyciel)->imie,0);
		ILI_WriteBigString(3,300,((Nauczyciel*)nauczyciel)->nazwisko,0);
		ILI_DrawWhiteSector(3,466,200,13);
		sprintf(bufor,"%c.%s",((Nauczyciel*)nauczyciel)->imie[0],((Nauczyciel*)nauczyciel)->nazwisko);
		ILI_WriteString(3,466,bufor,0);
		status = 2;
		//printf("\r\n");
		//		HAL_UART_Transmit(&hlpuart1, uid, uid_len, 1000);
		//		printf("Wyslano UART RFID\r\n");
	}
}

void PN532_IRQ_Student_Handler(void)
{
	BUZZER_ON
	HAL_Delay(500);
	BUZZER_OFF
	if (uid_len == PN532_STATUS_ERROR) {
	} else {
		char buffer[9];
		char bufor[100];
		uint8_t i = 0;
		for (i = 0; i < uid_len; i++)
		{
			sprintf(&buffer[i*2], "%02x", uid[i]);
		}
		buffer[i*2] = '\0';
		Student* student = findStudentId(buffer);
		ILI_SmallCLS();
		if (student == NULL)
		{
			ILI_WriteBigString(40,215,"Zeskanowany tag nie",0xFF0000);
			ILI_WriteBigString(3,245,"należy do żadnego studenta",0xFF0000);
			return;
		}

		ILI_DrawPicture(110,80,student->zdjecie,100,125);
		ILI_WriteBigString(3,280,student->imie,0);
		ILI_WriteBigString(3,300,student->nazwisko,0);
		uint8_t status = isStudentOnList(student->uuid, id_lekcji);
		if (status == 0)
		{
			ILI_WriteBigString(3,330,"Student nie znajduje się",0xFF0000);
			ILI_WriteBigString(3,360,"na liście",0xFF0000);
			return;
		}
		ILI_WriteBigString(3,330,"Zatwierdzono obecność",0x00FF00);
		for(i = 0; i < list_len; i++)
				{
					sprintf(bufor,"%02x%02x%02x%02x",list[i][0],list[i][1],list[i][2],list[i][3]);
					ILI_WriteString(3,360+(i*20),bufor,0);
				}
		for(i = 0; i < list_len; i++)
		{
			if(strcmp(list[i],student->uuid) == 0)	return;
		}
		strcpy(list[list_len],student->uuid);
		list_len++;
		//printf("\r\n");
		//		HAL_UART_Transmit(&hlpuart1, uid, uid_len, 1000);
		//		printf("Wyslano UART RFID\r\n");
	}
}

//void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
//{
//    if (GPIO_Pin == PN532_IRQ_Pin)
//    {
//    	printf("IRQ from PN532 triggered!\r\n");
//    	printf("response_ready callback: %d\r\n", response_ready);
//    	if (response_ready == 1){
//    		printf("response_ready = 1 handler\r\n");
//    		PN532_IRQ_Handler();
//    	}
//    }
//}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == BTN1_Pin)
	{
        uint32_t now = HAL_GetTick();
        if (now - last_button_time < 200) return; // debounce: 200 ms
        last_button_time = now;
		if (status == 3)	sendList();
		if (status == 2 && wybrana_lekcja > 0)	wybrana_lekcja--;
	}
	if (GPIO_Pin == BTN2_Pin)
	{
		uint32_t now = HAL_GetTick();
		if (now - last_button_time < 200) return; // debounce: 200 ms
		last_button_time = now;
		if (status == 2)
		{
			id_lekcji = Terminarze[lesson_ids[wybrana_lekcja]].id;
			ILI_DrawWhiteSector(200,466,119,13);
			ILI_WriteString(317  - (7 * strlen(Terminarze[lesson_ids[wybrana_lekcja]].nazwa_przedmiotu)),466,Terminarze[lesson_ids[wybrana_lekcja]].nazwa_przedmiotu,0);
			status = 3;
		}
	}
	if (GPIO_Pin == BTN4_Pin)
	{
		uint32_t now = HAL_GetTick();
		if (now - last_button_time < 200) return;
		last_button_time = now;
		if (status == 2 && wybrana_lekcja < lessons_len - 1)	wybrana_lekcja++;
	}
	if (GPIO_Pin == PN_IRQ_Pin)
	{
		if (response_ready == 0)	return;
		response_ready = 0;
		if (status == 1)	PN532_IRQ_Teacher_Handler();
		if (status == 3)	PN532_IRQ_Student_Handler();
	}
}

void HAL_RTCEx_WakeUpTimerEventCallback(RTC_HandleTypeDef *hrtc)
{
	ILI_DrawInnerBattery();
	timestamp += 30;
	struct tm *dt;
	dt = localtime(&timestamp);
	char date[17];
	sprintf(date,"%02d.%02d.%04d %d:%02d",dt->tm_mday,dt->tm_mon+1,dt->tm_year+1900,dt->tm_hour,dt->tm_min);
	ILI_DrawWhiteSector(3,1,115,12);
	ILI_WriteString(3,1,date,0);
}

void bytes_to_hex_string(const unsigned char *bytes, size_t length, char *output) {
    for (size_t i = 0; i < length; i++) {
        // %02X - formatowanie na 2 cyfry hex, wielkie litery
        sprintf(output + i*2, "%02X", bytes[i]);
    }
    output[length * 2] = '\0';  // zakończenie stringa
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
  MX_RTC_Init();
  MX_SPI2_Init();
  MX_TIM1_Init();
  MX_ADC1_Init();
  MX_SPI1_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  PN532_SPI_Init(&pn532);
  PN532_SamConfiguration(&pn532);
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  init_database();
  ILI_Init();
  ILI_Intro();
  ILI_CLS();
  setenv("TZ", "CET-1CEST,M3.5.0/2,M10.5.0/3", 1);
  tzset();
//  ILI_WriteString(3,1,"04.06.2025 11:22 MY_SSID",0);
//  ILI_DrawBlackSector(0,15,320,3);
//  ILI_DrawBattery();
//  ILI_DrawInnerBattery();
//  ILI_WriteBigString(3,280,"Kazimierz",0);
//  ILI_WriteBigString(3,300,"BRZĘCZYSZCZYKIEWICZ",0);
//  ILI_DrawBlackSector(0,462,320,3);
//  ILI_WriteString(3,466,"P.Korpas",0);
//  ILI_WriteString(290,466,"PBL4",0);
//  ILI_WriteString(90,350,"1. Zatwierdź listę",0);
//  ILI_WriteString(65,370,"2. Anuluj wybór przedmiotu",0);
//  ILI_WriteString(120,390,"3. Uśpij",0);
//  ILI_SmallCLS();
//  ILI_WriteBigString(3,280,"Kazimierz",0);
//  ILI_WriteBigString(3,300,"BRZĘCZYSZCZYKIEWICZ",0);
  ILI_CLS();
  ILI_WriteBigString(3, 220, "Połącz urządzenie do sieci Wi-Fi",0);
//  	  printf("Waiting for RFID/NFC card...\r\n");

//  	  printf("response ready waiting: %d\r\n", response_ready);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  // POBIRANIE DATY
     uint8_t timestamp1[10] = {0};
	  uint8_t ssid[16] = {0};
	  __HAL_UART_FLUSH_DRREGISTER(&huart1); // dla HAL
	  __HAL_UART_CLEAR_OREFLAG(&huart1);
	  __HAL_UART_CLEAR_FEFLAG(&huart1);
	  __HAL_UART_CLEAR_NEFLAG(&huart1);

//
//	  // Odczyt timestamp
	  while(1){
		  HAL_StatusTypeDef status;
		  status = HAL_UART_Receive(&huart1, timestamp1, 10, HAL_MAX_DELAY);
//		  printf("status: %d\r\n", status);
		  if (status == 0){
//			  for(uint8_t i=0; i < 10; i++){
//				  printf("%c", timestamp[i]);
//			  }
//			  printf("\r\n");
			  char timestamp_str[11];
			  memcpy(timestamp_str, timestamp1, 10);
			  timestamp_str[10] = '\0';
			  unsigned long timestamp_val = strtoul(timestamp_str, NULL, 10);
			  timestamp = timestamp_val - 30;
			  ILI_DrawBlackSector(0,15,320,3);
			  ILI_DrawBattery();
			  ILI_DrawBlackSector(0,462,320,3);
			  HAL_RTCEx_WakeUpTimerEventCallback(&hrtc);
			  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
			  HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, 61440, RTC_WAKEUPCLOCK_RTCCLK_DIV16);
//			  uint8_t i = 0;
//			  for (i = 0; i < 10; i++)
//			  {
//				  timestamp *= 2;
//				  timestamp += timestamp1[i] - 48;
//			  }
			  HAL_Delay(1000);
			  HAL_UART_Transmit(&huart1, (uint8_t*)"ACK", strlen("ACK"), HAL_MAX_DELAY);
			  break;
		  }
	  }
//	  // Odczyt SSID
	  while(1){
		  HAL_StatusTypeDef status2 = HAL_UART_Receive(&huart1, ssid, 15, HAL_MAX_DELAY);
//		  printf("status2: %d\r\n", status2);
		  if (status2 == 0){
//			  for(uint8_t i=0; i < 32; i++){
//				  printf("%c", ssid[i]);
//			  }
//			  printf("\r\n");
			  ssid[15] = '\0';
			  ILI_DrawWhiteSector(120,1,170,12);
			  ILI_WriteString(130,1,ssid,0);
			  HAL_UART_Transmit(&huart1, (uint8_t*)"ACK", strlen("ACK"), HAL_MAX_DELAY);
			  ILI_SmallCLS();
			  ILI_WriteBigString(3, 220, "Zeskanuj tag prowadzącego",0);
  			  break;
  		  }
//  timestamp = 1749782311;
//  status = 1;
}
while (1)
{
	// Szukanie tagu RFID
	while (status == 1)
	{
//		ILI_LEDOFF();
		uid_len = PN532_ReadPassiveTarget(&pn532, uid, PN532_MIFARE_ISO14443A, 1000);
		//printf("uid_len: %d\r\n\n", uid_len);
		if (uid_len != PN532_STATUS_ERROR){
//			ILI_LEDON();
			response_ready = 1;
		}
	}
	HAL_Delay(5000);
	lessons_len = getLessonIds(((Nauczyciel*)nauczyciel)->uuid);
	if (lessons_len == 0)
	{
		char bufor[100] = {0};
		sprintf(bufor,"%s %s",((Nauczyciel*)nauczyciel)->imie,((Nauczyciel*)nauczyciel)->nazwisko);
		ILI_SmallCLS();
		ILI_WriteBigString(100,195,"Nauczyciel",0xFF0000);
		ILI_WriteBigString(160 - (6 * strlen(bufor)),225,bufor,0xFF0000);
		ILI_WriteBigString(10,255,"nie prowadzi teraz zajęć",0xFF0000);
		status = 1;
		continue;
	}
	uint16_t i = 0;
	while (status == 2)
	{
		ILI_SmallCLS();
		for (i = 0; i < lessons_len; i++)
		{
			if (wybrana_lekcja == i)
			{
				ILI_DrawBlackSector(0,20 + (30 * i),320,30);
				ILI_WriteBigString(160 - (10 * strlen(Terminarze[lesson_ids[i]].nazwa_przedmiotu)),22 + (30 * i),Terminarze[lesson_ids[i]].nazwa_przedmiotu,0xFFFFFF);
				continue;
			}
			ILI_WriteBigString(160 - (10 * strlen(Terminarze[lesson_ids[i]].nazwa_przedmiotu)),22 + (30 * i),Terminarze[lesson_ids[i]].nazwa_przedmiotu,0);
		}
		HAL_Delay(1000);
	}
	ILI_SmallCLS();
	ILI_WriteBigString(100,230,"Zeskanuj Tag",0);
	while (status == 3)
	{
		uid_len = PN532_ReadPassiveTarget(&pn532, uid, PN532_MIFARE_ISO14443A, 1000);
		//printf("uid_len: %d\r\n\n", uid_len);
		if (uid_len != PN532_STATUS_ERROR){
			response_ready = 1;
		}
	}

//	  uint8_t rx[260] = {0};
//	  HAL_UART_Transmit(&huart1, (uint8_t*)"Hello", strlen("Hello"), 1000);
//	  //printf("Wyslano UART\r\n");
//	  HAL_UART_Receive(&huart1, rx, 243, 1000);
//	  rx[258] = '\0';
//
//	  ILI_WriteString(3,20+index, (char*)rx,0);
//	  index+=20;
//
//	  HAL_Delay(2000);
//	  if(BTN1_PRESSED)
//	  {
//		  ILI_WriteString(120,300,"1",0);
//	  }
//	  else ILI_WriteString(120,300,"1",0xFFFFFF);
//	  if(BTN2_PRESSED)
//	  {
//		  ILI_WriteString(140,300,"2",0);
//	  }
//	  else ILI_WriteString(140,300,"2",0xFFFFFF);
//	  if(BTN3_PRESSED)
//	  {
//		  ILI_WriteString(160,300,"3",0);
//	  }
//	  else ILI_WriteString(160,300,"3",0xFFFFFF);
//	  if(BTN4_PRESSED)
//	  {
//		  ILI_WriteString(180,300,"4",0);
//	  }
//	  else ILI_WriteString(180,300,"4",0xFFFFFF);

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

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 20;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc1.Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_6;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_2CYCLES_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */

  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
  sTime.Hours = 0x20;
  sTime.Minutes = 0x10;
  sTime.Seconds = 0x0;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  sDate.WeekDay = RTC_WEEKDAY_TUESDAY;
  sDate.Month = RTC_MONTH_JUNE;
  sDate.Date = 0x10;
  sDate.Year = 0x25;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_LSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 7;
  hspi2.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi2.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 399;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 49;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 20000;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.BreakFilter = 0;
  sBreakDeadTimeConfig.Break2State = TIM_BREAK2_DISABLE;
  sBreakDeadTimeConfig.Break2Polarity = TIM_BREAK2POLARITY_HIGH;
  sBreakDeadTimeConfig.Break2Filter = 0;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */
  HAL_TIM_MspPostInit(&htim1);

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(NSS_GPIO_Port, NSS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, PN_RST_Pin|CS_Pin|RST_Pin|DC_Pin
                          |LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : RSTOUT_Pin */
  GPIO_InitStruct.Pin = RSTOUT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(RSTOUT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PN_IRQ_Pin */
  GPIO_InitStruct.Pin = PN_IRQ_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(PN_IRQ_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : NSS_Pin */
  GPIO_InitStruct.Pin = NSS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(NSS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : BTN2_Pin */
  GPIO_InitStruct.Pin = BTN2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(BTN2_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : BTN1_Pin */
  GPIO_InitStruct.Pin = BTN1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(BTN1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PN_RST_Pin CS_Pin RST_Pin DC_Pin
                           LED_Pin */
  GPIO_InitStruct.Pin = PN_RST_Pin|CS_Pin|RST_Pin|DC_Pin
                          |LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : BTN4_Pin */
  GPIO_InitStruct.Pin = BTN4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(BTN4_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

  HAL_NVIC_SetPriority(EXTI1_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);

  HAL_NVIC_SetPriority(EXTI3_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(EXTI3_IRQn);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

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

#ifdef  USE_FULL_ASSERT
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
