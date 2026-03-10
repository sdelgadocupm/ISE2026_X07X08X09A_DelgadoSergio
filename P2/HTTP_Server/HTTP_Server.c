/*------------------------------------------------------------------------------
 * MDK Middleware - Component ::Network
 * Copyright (c) 2004-2019 Arm Limited (or its affiliates). All rights reserved.
 *------------------------------------------------------------------------------
 * Name:    HTTP_Server.c
 * Purpose: HTTP Server example
 *----------------------------------------------------------------------------*/

#include <stdio.h>
#include <time.h>
#include <string.h>

#include "main.h"

#include "rl_net.h"                     // Keil.MDK-Pro::Network:CORE

#include "stm32f4xx_hal.h"              // Keil::Device:STM32Cube HAL:Common
#include "Board_LED.h"                  // ::Board Support:LED
#include "Board_Buttons.h"              // ::Board Support:Buttons
#include "adc.h"                  
#include "lcd.h"        
#include "rtc.h"

// Main stack size must be multiple of 8 Bytes
#define APP_MAIN_STK_SZ (1024U)
uint64_t app_main_stk[APP_MAIN_STK_SZ / 8];
const osThreadAttr_t app_main_attr = {
  .stack_mem  = &app_main_stk[0],
  .stack_size = sizeof(app_main_stk)
};


RTC_HandleTypeDef RtcHandle;
//uint8_t aShowTime[50] = {0};
//uint8_t aShowDate[50] = {0};

//SNTP
const NET_ADDR4 ntp_server = {NET_ADDR_IP4, 0, 178, 215, 228, 24 };
static void time_callback (uint32_t seconds, uint32_t seconds_fraction);


/*----- Periodic Timer Example -----*/
osTimerId_t tim_id2;                            // timer id
static uint32_t exec2;                          // argument for the timer call back function
static void Timer2_Callback (void const *arg);

uint8_t cnt_led=0;

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin); 


extern uint16_t AD_in          (uint32_t ch);
extern uint8_t  get_button     (void);
extern void     netDHCP_Notify (uint32_t if_num, uint8_t option, const uint8_t *val, uint32_t len);

extern bool LEDrun;
extern char lcd_text[2][20+1];

extern osThreadId_t TID_Display;
extern osThreadId_t TID_Led;
extern osThreadId_t TID_Rtc;
extern osThreadId_t TID_Sntp;

static void initB1(void);

bool LEDrun;
char lcd_text[2][20+1] = { "LCD line 1",
                           "LCD line 2" };

/* Thread IDs */
osThreadId_t TID_Display;
osThreadId_t TID_Led;
osThreadId_t TID_Rtc;
osThreadId_t TID_Sntp;

													 
/* Thread declarations */
static void BlinkLed (void *arg);
static void Display  (void *arg);
static void Rtc  (void *arg);
static void Sntp  (void *arg);


__NO_RETURN void app_main (void *arg);

/* Read analog inputs */
uint16_t AD_in (uint32_t ch) {
  int32_t val = 0;

	
  if (ch == 0) {
	ADC_HandleTypeDef adchandle; //handler definition
		ADC1_pins_F429ZI_config();
		ADC_Init_Single_Conversion(&adchandle , ADC1); //ADC1 configuration
		val = (int32_t)ADC_getVoltage(&adchandle , 10 );
  }
  return ((uint16_t)val);
}

/* Read digital inputs */
uint8_t get_button (void) {
//  return ((uint8_t)Buttons_GetState ());
}



/*----------------------------------------------------------------------------
  Thread 'Display': LCD display handler
 *---------------------------------------------------------------------------*/
static __NO_RETURN void Display (void *arg) {
  static uint8_t ip_addr[NET_ADDR_IP6_LEN];
  static char    ip_ascii[40];
  static char    buf[24];
	
	char lcd_text_aux[2][20+1];

  (void)arg;

	LCD_reset();
	LCD_Init();
	LCD_clean();
	
	
	

	printLCD("       MDK-MW       ", 1);	
	printLCD("HTTP Server example ", 2);


  while(1) {

    /* Retrieve and print IPv4 address */
    netIF_GetOption (NET_IF_CLASS_ETH,
                     netIF_OptionIP4_Address, ip_addr, sizeof(ip_addr));

    netIP_ntoa (NET_ADDR_IP4, ip_addr, ip_ascii, sizeof(ip_ascii));


    /* Display user text lines */
		if(strcmp(lcd_text[0],lcd_text_aux[0])){
			printLCD(lcd_text[0], 1);
			strcpy(lcd_text_aux[0], lcd_text[0]);
		}
	
		if(strcmp(lcd_text[1],lcd_text_aux[1])){
			printLCD(lcd_text[1], 2);
			strcpy(lcd_text_aux[1], lcd_text[1]);
		}		

  }
}

/*----------------------------------------------------------------------------
  Thread 'BlinkLed': Blink the LEDs on an eval board
 *---------------------------------------------------------------------------*/
static __NO_RETURN void BlinkLed (void *arg) {
  const uint8_t led_val[16] = { 0x48,0x88,0x84,0x44,0x42,0x22,0x21,0x11,
                                0x12,0x0A,0x0C,0x14,0x18,0x28,0x30,0x50 };
  uint32_t cnt = 0U;

  (void)arg;

	
																
																
  LEDrun = false;
  while(1) {
    /* Every 100 ms */
    if (LEDrun == true) {
      LED_SetOut (led_val[cnt]);
      if (++cnt >= sizeof(led_val)) {
        cnt = 0U;
      }
    }
    osDelay (100);
  }
}

/*----------------------------------------------------------------------------
  Thread 'RTC':
 *---------------------------------------------------------------------------*/
static __NO_RETURN void Rtc (void *arg) {
	uint8_t aShowTime[50] = {0};
	uint8_t aShowDate[50] = {0};

  (void)arg;

	configureRTC(&RtcHandle);
	checkBackUp(&RtcHandle);
																
	while(1) {
		RTC_CalendarShow(&RtcHandle, aShowTime, aShowDate);
		printLCD(aShowTime, 1);
		printLCD(aShowDate, 2);
		osDelay (100);
  }
}

/*----------------------------------------------------------------------------
  Thread 'SNTP':
 *---------------------------------------------------------------------------*/
static __NO_RETURN void Sntp (void *arg) {

  (void)arg;
	
	osDelay(5000);
	netSNTPc_GetTime ((NET_ADDR *)&ntp_server, time_callback);	
	osTimerStart(tim_id2, 200U);

	while(1) {
		osDelay(180000);
		netSNTPc_GetTime ((NET_ADDR *)&ntp_server, time_callback);
		osTimerStart(tim_id2, 200U);
  }
}

/*----------------------------------------------------------------------------
  Main Thread 'main': Run Network
 *---------------------------------------------------------------------------*/
__NO_RETURN void app_main (void *arg) {
  (void)arg;

  LED_Initialize();
 // Buttons_Initialize();
	initB1();
	
	// Create periodic timer
  exec2 = 2U;
  tim_id2 = osTimerNew((osTimerFunc_t)&Timer2_Callback, osTimerPeriodic, &exec2, NULL);

  netInitialize ();

  TID_Led     = osThreadNew (BlinkLed, NULL, NULL);
  TID_Display = osThreadNew (Display,  NULL, NULL);
  TID_Rtc = osThreadNew (Rtc,  NULL, NULL);
  TID_Sntp = osThreadNew (Sntp,  NULL, NULL);
	

	
  osThreadExit();
}


static void time_callback (uint32_t seconds, uint32_t senconds_fraction){
	struct tm  ts;
  char       buf[80];
	
	
	if(seconds != 0){
	
		ts = *localtime(&seconds);
		
		RTC_DateTypeDef sdate;
		RTC_TimeTypeDef stime;
		
		sdate.Year = ts.tm_year-100;
		sdate.Month = ts.tm_mon+1;
		sdate.Date = ts.tm_mday;
		sdate.WeekDay = ts.tm_wday;	
		
		stime.Hours = ts.tm_hour+1;
		stime.Minutes = ts.tm_min;
		stime.Seconds = ts.tm_sec;
		stime.TimeFormat = RTC_HOURFORMAT12_AM;
		stime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE ;
		stime.StoreOperation = RTC_STOREOPERATION_RESET;
	
		RTC_CalendarConfig(&RtcHandle, sdate, stime);
		
	}
}

// Periodic Timer Function
static void Timer2_Callback (void const *arg) {
  
	HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_14);
	if(cnt_led==19){
		cnt_led=0;
		osTimerStop(tim_id2);
	}else{
		cnt_led++;
	}
}

//Funcion boton usuario(azul)
static void initB1(void){
  
  GPIO_InitTypeDef GPIOC_InitStruct;
  
  __HAL_RCC_GPIOC_CLK_ENABLE();
	
	GPIOC_InitStruct.Mode = GPIO_MODE_IT_RISING;	
	GPIOC_InitStruct.Pull = GPIO_NOPULL;
	
	GPIOC_InitStruct.Pin = GPIO_PIN_13;
	HAL_GPIO_Init(GPIOC, &GPIOC_InitStruct);
	
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
  
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	if(GPIO_Pin == GPIO_PIN_13){
		RTC_DateTypeDef sdate;
		RTC_TimeTypeDef stime;
		
		sdate.Year = 0;
		sdate.Month = 1;
		sdate.Date = 1;
		sdate.WeekDay = 6;	
		
		stime.Hours = 0;
		stime.Minutes = 0;
		stime.Seconds = 0;
		stime.TimeFormat = RTC_HOURFORMAT12_AM;
		stime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE ;
		stime.StoreOperation = RTC_STOREOPERATION_RESET;
	
		RTC_CalendarConfig(&RtcHandle, sdate, stime);
	}
}

