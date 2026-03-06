#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "stm32f4xx_hal.h"
#include "Thread.h"
#include "lcd.h"
#include "rtc.h"


/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
osThreadId_t tid_Thread;                        // thread id

uint8_t cnt_led=0;
uint8_t alarma=0;

/*----- Periodic Timer Example -----*/
osTimerId_t tim_id2;                            // timer id
static uint32_t exec2;                          // argument for the timer call back function
static void Timer2_Callback (void const *arg);

 
RTC_HandleTypeDef RtcHandle;
RTC_AlarmTypeDef sAlarm;

/* Buffers used for displaying Time and Date */
uint8_t aShowTime[50] = {0};
uint8_t aShowDate[50] = {0};

static void initLEDs(void);
 
void Thread (void *argument);                   // thread function
 
int Init_Thread (void) {
 
  tid_Thread = osThreadNew(Thread, NULL, NULL);
  if (tid_Thread == NULL) {
    return(-1);
  }
	
	// Create periodic timer
  exec2 = 2U;
  tim_id2 = osTimerNew((osTimerFunc_t)&Timer2_Callback, osTimerPeriodic, &exec2, NULL);
 
  return(0);
}
 
void Thread (void *argument) {
	
	LCD_reset();
	LCD_Init();
	LCD_clean();
	
	initLEDs();
	
	configureRTC(&RtcHandle);
	checkBackUp(&RtcHandle);

	HAL_NVIC_EnableIRQ(RTC_Alarm_IRQn);
	
	sAlarm.Alarm=RTC_ALARM_A;
	sAlarm.AlarmMask=RTC_ALARMMASK_DATEWEEKDAY|RTC_ALARMMASK_HOURS|RTC_ALARMMASK_MINUTES;
	sAlarm.AlarmTime.Seconds=0;
	
	HAL_RTC_SetAlarm_IT(&RtcHandle, &sAlarm, RTC_FORMAT_BIN);
	
  while (1) {

		/*##-3- Display the updated Time and Date ################################*/
    RTC_CalendarShow(&RtcHandle, aShowTime, aShowDate);
		printLCD(aShowTime, 1);
		printLCD(aShowDate, 2);
		if(alarma==1){
			osTimerStart(tim_id2, 500U);
			alarma=0;
		}
		osDelay(250);
    osThreadYield();                            // suspend thread
  }
}

void HAL_RTC_AlarmAEventCallback (RTC_HandleTypeDef * hrtc){
		
	sAlarm.Alarm=RTC_ALARM_A;
	sAlarm.AlarmMask=RTC_ALARMMASK_DATEWEEKDAY|RTC_ALARMMASK_HOURS|RTC_ALARMMASK_MINUTES;
	sAlarm.AlarmTime.Seconds=0;
	
	HAL_RTC_SetAlarm_IT(&RtcHandle, &sAlarm, RTC_FORMAT_BIN);
	alarma=1;
	
}


// Periodic Timer Function
static void Timer2_Callback (void const *arg) {
  
	HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);
	if(cnt_led==9){
		cnt_led=0;
		osTimerStop(tim_id2);
	}else{
		cnt_led++;
	}
}


static void initLEDs(void){
  
  GPIO_InitTypeDef GPIOB_InitStruct;
  
  __HAL_RCC_GPIOB_CLK_ENABLE();
	
	GPIOB_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;	
	GPIOB_InitStruct.Pull = GPIO_PULLUP;
	GPIOB_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	
	//LD1
	GPIOB_InitStruct.Pin = GPIO_PIN_0;
	HAL_GPIO_Init(GPIOB, &GPIOB_InitStruct);
	
	//LD2
	GPIOB_InitStruct.Pin = GPIO_PIN_7;
	HAL_GPIO_Init(GPIOB, &GPIOB_InitStruct);
	
	//LD3
	GPIOB_InitStruct.Pin = GPIO_PIN_14;
	HAL_GPIO_Init(GPIOB, &GPIOB_InitStruct);
}



