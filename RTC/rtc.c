#include "rtc.h"



void configureRTC(RTC_HandleTypeDef *hrtc){
		 /*##-1- Configure the RTC peripheral #######################################*/
  /* Configure RTC prescaler and RTC data registers */
  /* RTC configured as follows:
      - Hour Format    = Format 24
      - Asynch Prediv  = Value according to source clock
      - Synch Prediv   = Value according to source clock
      - OutPut         = Output Disable
      - OutPutPolarity = High Polarity
      - OutPutType     = Open Drain */ 
  hrtc->Instance = RTC; 
  hrtc->Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc->Init.AsynchPrediv = RTC_ASYNCH_PREDIV;
  hrtc->Init.SynchPrediv = RTC_SYNCH_PREDIV;
  hrtc->Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc->Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc->Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  __HAL_RTC_RESET_HANDLE_STATE(hrtc);
  if (HAL_RTC_Init(hrtc) != HAL_OK)
  {
    /* Initialization Error */
    //Error_Handler();
  }
}

void checkBackUp(RTC_HandleTypeDef *hrtc){
	
  /*##-2- Check if Data stored in BackUp register1: No Need to reconfigure RTC#*/
  /* Read the Back Up Register 1 Data */
  if (HAL_RTCEx_BKUPRead(hrtc, RTC_BKP_DR1) != 0x32F2)
  {
    /* Configure RTC Calendar */
		
		RTC_DateTypeDef sdatestructure;
		RTC_TimeTypeDef stimestructure;
		
		sdatestructure.Year = 0x14;
		sdatestructure.Month = RTC_MONTH_FEBRUARY;
		sdatestructure.Date = 0x18;
		sdatestructure.WeekDay = RTC_WEEKDAY_TUESDAY;
		
		stimestructure.Hours = 0x02;
		stimestructure.Minutes = 0x00;
		stimestructure.Seconds = 0x00;
		stimestructure.TimeFormat = RTC_HOURFORMAT12_AM;
		stimestructure.DayLightSaving = RTC_DAYLIGHTSAVING_NONE ;
		stimestructure.StoreOperation = RTC_STOREOPERATION_RESET;
		
    RTC_CalendarConfig(&hrtc, sdatestructure, stimestructure);
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
	
}




/**
  * @brief  Configure the current time and date.
  * @param  None
  * @retval None
  */
void RTC_CalendarConfig(RTC_HandleTypeDef *hrtc, RTC_DateTypeDef sdatestructure, RTC_TimeTypeDef stimestructure)
{

  /*##-1- Configure the Date #################################################*/
  /* Set Date: Tuesday February 18th 2014 */
//  sdatestructure.Year = 0x14;
//  sdatestructure.Month = RTC_MONTH_FEBRUARY;
//  sdatestructure.Date = 0x18;
//  sdatestructure.WeekDay = RTC_WEEKDAY_TUESDAY;
  
  if(HAL_RTC_SetDate(hrtc,&sdatestructure,RTC_FORMAT_BCD) != HAL_OK)
  {
    /* Initialization Error */
    //Error_Handler();
  }

  /*##-2- Configure the Time #################################################*/
  /* Set Time: 02:00:00 */
//  stimestructure.Hours = 0x02;
//  stimestructure.Minutes = 0x00;
//  stimestructure.Seconds = 0x00;
//  stimestructure.TimeFormat = RTC_HOURFORMAT12_AM;
//  stimestructure.DayLightSaving = RTC_DAYLIGHTSAVING_NONE ;
//  stimestructure.StoreOperation = RTC_STOREOPERATION_RESET;

  if (HAL_RTC_SetTime(hrtc, &stimestructure, RTC_FORMAT_BCD) != HAL_OK)
  {
    /* Initialization Error */
    //Error_Handler();
  }

  /*##-3- Writes a data in a RTC Backup data Register1 #######################*/
  HAL_RTCEx_BKUPWrite(hrtc, RTC_BKP_DR1, 0x32F2);
}


/**
  * @brief  Display the current time and date.
  * @param  showtime : pointer to buffer
  * @param  showdate : pointer to buffer
  * @retval None
  */
void RTC_CalendarShow(RTC_HandleTypeDef *hrtc, uint8_t *showtime, uint8_t *showdate)
{
  RTC_DateTypeDef sdatestructureget;
  RTC_TimeTypeDef stimestructureget;

  /* Get the RTC current Time */
  HAL_RTC_GetTime(hrtc, &stimestructureget, RTC_FORMAT_BIN);
  /* Get the RTC current Date */
  HAL_RTC_GetDate(hrtc, &sdatestructureget, RTC_FORMAT_BIN);
  /* Display time Format : hh:mm:ss */
  sprintf((char *)showtime, "%02d:%02d:%02d", stimestructureget.Hours, stimestructureget.Minutes, stimestructureget.Seconds);
  /* Display date Format : mm-dd-yy */
  sprintf((char *)showdate, "%02d-%02d-%02d", sdatestructureget.Month, sdatestructureget.Date, 2000 + sdatestructureget.Year);
}






void HAL_RTC_MspInit(RTC_HandleTypeDef *hrtc)
{
  RCC_OscInitTypeDef        RCC_OscInitStruct;
  RCC_PeriphCLKInitTypeDef  PeriphClkInitStruct;

  /*##-1- Enables the PWR Clock and Enables access to the backup domain ###################################*/
  /* To change the source clock of the RTC feature (LSE, LSI), You have to:
     - Enable the power clock using __HAL_RCC_PWR_CLK_ENABLE()
     - Enable write access using HAL_PWR_EnableBkUpAccess() function before to 
       configure the RTC clock source (to be done once after reset).
     - Reset the Back up Domain using __HAL_RCC_BACKUPRESET_FORCE() and 
       __HAL_RCC_BACKUPRESET_RELEASE().
     - Configure the needed RTc clock source */
  __HAL_RCC_PWR_CLK_ENABLE();
  HAL_PWR_EnableBkUpAccess();

  
  /*##-2- Configure LSE as RTC clock source ###################################*/
  RCC_OscInitStruct.OscillatorType =  RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  { 
  //  Error_Handler();
  }
  
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  if(HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  { 
  //  Error_Handler();
  }
  
  /*##-3- Enable RTC peripheral Clocks #######################################*/
  /* Enable RTC Clock */
  __HAL_RCC_RTC_ENABLE();
}

/**
  * @brief RTC MSP De-Initialization
  *        This function frees the hardware resources used in this example:
  *          - Disable the Peripheral's clock
  * @param hrtc: RTC handle pointer
  * @retval None
  */
void HAL_RTC_MspDeInit(RTC_HandleTypeDef *hrtc)
{
  /*##-1- Reset peripherals ##################################################*/
  __HAL_RCC_RTC_DISABLE();

  /*##-2- Disables the PWR Clock and Disables access to the backup domain ###################################*/
  HAL_PWR_DisableBkUpAccess();
  __HAL_RCC_PWR_CLK_DISABLE();
  
}