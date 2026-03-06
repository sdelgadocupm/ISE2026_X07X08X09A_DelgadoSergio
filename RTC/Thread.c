#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "stm32f4xx_hal.h"
#include "Thread.h"
#include "lcd.h"


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

static void RTC_CalendarConfig(void);
static void RTC_CalendarShow(uint8_t *showtime, uint8_t *showdate);
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
	
	 /*##-1- Configure the RTC peripheral #######################################*/
  /* Configure RTC prescaler and RTC data registers */
  /* RTC configured as follows:
      - Hour Format    = Format 24
      - Asynch Prediv  = Value according to source clock
      - Synch Prediv   = Value according to source clock
      - OutPut         = Output Disable
      - OutPutPolarity = High Polarity
      - OutPutType     = Open Drain */ 
  RtcHandle.Instance = RTC; 
  RtcHandle.Init.HourFormat = RTC_HOURFORMAT_24;
  RtcHandle.Init.AsynchPrediv = RTC_ASYNCH_PREDIV;
  RtcHandle.Init.SynchPrediv = RTC_SYNCH_PREDIV;
  RtcHandle.Init.OutPut = RTC_OUTPUT_DISABLE;
  RtcHandle.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  RtcHandle.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  __HAL_RTC_RESET_HANDLE_STATE(&RtcHandle);
  if (HAL_RTC_Init(&RtcHandle) != HAL_OK)
  {
    /* Initialization Error */
    //Error_Handler();
  }
 

  /*##-2- Check if Data stored in BackUp register1: No Need to reconfigure RTC#*/
  /* Read the Back Up Register 1 Data */
  if (HAL_RTCEx_BKUPRead(&RtcHandle, RTC_BKP_DR1) != 0x32F2)
  {
    /* Configure RTC Calendar */
    RTC_CalendarConfig();
  }
  else
  {
    /* Check if the Power On Reset flag is set */
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_PORRST) != RESET)
    {
      
    }
    /* Check if Pin Reset flag is set */
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_PINRST) != RESET)
    {
     
    }
    /* Clear source Reset Flag */
    __HAL_RCC_CLEAR_RESET_FLAGS();
  }
	
	HAL_NVIC_EnableIRQ(RTC_Alarm_IRQn);
	
	sAlarm.Alarm=RTC_ALARM_A;
	sAlarm.AlarmMask=RTC_ALARMMASK_DATEWEEKDAY|RTC_ALARMMASK_HOURS|RTC_ALARMMASK_MINUTES;
	sAlarm.AlarmTime.Seconds=0;
	
	HAL_RTC_SetAlarm_IT(&RtcHandle, &sAlarm, RTC_FORMAT_BIN);
	
  while (1) {

		/*##-3- Display the updated Time and Date ################################*/
    RTC_CalendarShow(aShowTime, aShowDate);
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


/**
  * @brief  Configure the current time and date.
  * @param  None
  * @retval None
  */
static void RTC_CalendarConfig(void)
{
  RTC_DateTypeDef sdatestructure;
  RTC_TimeTypeDef stimestructure;

  /*##-1- Configure the Date #################################################*/
  /* Set Date: Tuesday February 18th 2014 */
  sdatestructure.Year = 0x14;
  sdatestructure.Month = RTC_MONTH_FEBRUARY;
  sdatestructure.Date = 0x18;
  sdatestructure.WeekDay = RTC_WEEKDAY_TUESDAY;
  
  if(HAL_RTC_SetDate(&RtcHandle,&sdatestructure,RTC_FORMAT_BCD) != HAL_OK)
  {
    /* Initialization Error */
    //Error_Handler();
  }

  /*##-2- Configure the Time #################################################*/
  /* Set Time: 02:00:00 */
  stimestructure.Hours = 0x02;
  stimestructure.Minutes = 0x00;
  stimestructure.Seconds = 0x00;
  stimestructure.TimeFormat = RTC_HOURFORMAT12_AM;
  stimestructure.DayLightSaving = RTC_DAYLIGHTSAVING_NONE ;
  stimestructure.StoreOperation = RTC_STOREOPERATION_RESET;

  if (HAL_RTC_SetTime(&RtcHandle, &stimestructure, RTC_FORMAT_BCD) != HAL_OK)
  {
    /* Initialization Error */
    //Error_Handler();
  }

  /*##-3- Writes a data in a RTC Backup data Register1 #######################*/
  HAL_RTCEx_BKUPWrite(&RtcHandle, RTC_BKP_DR1, 0x32F2);
}


/**
  * @brief  Display the current time and date.
  * @param  showtime : pointer to buffer
  * @param  showdate : pointer to buffer
  * @retval None
  */
static void RTC_CalendarShow(uint8_t *showtime, uint8_t *showdate)
{
  RTC_DateTypeDef sdatestructureget;
  RTC_TimeTypeDef stimestructureget;

  /* Get the RTC current Time */
  HAL_RTC_GetTime(&RtcHandle, &stimestructureget, RTC_FORMAT_BIN);
  /* Get the RTC current Date */
  HAL_RTC_GetDate(&RtcHandle, &sdatestructureget, RTC_FORMAT_BIN);
  /* Display time Format : hh:mm:ss */
  sprintf((char *)showtime, "%02d:%02d:%02d", stimestructureget.Hours, stimestructureget.Minutes, stimestructureget.Seconds);
  /* Display date Format : mm-dd-yy */
  sprintf((char *)showdate, "%02d-%02d-%02d", sdatestructureget.Month, sdatestructureget.Date, 2000 + sdatestructureget.Year);
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



