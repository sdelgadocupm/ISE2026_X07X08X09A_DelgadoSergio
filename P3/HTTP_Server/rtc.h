#include "stm32f4xx_hal.h"

#ifndef __rtc_H
#define __rtc_H

	/* Defines related to Clock configuration */
#define RTC_ASYNCH_PREDIV  0x7F   /* LSE as RTC clock */
#define RTC_SYNCH_PREDIV   0x00FF /* LSE as RTC clock */

void configureRTC(RTC_HandleTypeDef *hrtc);
void checkBackUp(RTC_HandleTypeDef *hrtc);
void RTC_CalendarConfig(RTC_HandleTypeDef *hrtc, RTC_DateTypeDef sdatestructure, RTC_TimeTypeDef stimestructure);
void RTC_CalendarShow(RTC_HandleTypeDef *hrtc, uint8_t *showtime, uint8_t *showdate);

void HAL_RTC_MspInit(RTC_HandleTypeDef *hrtc);
void HAL_RTC_MspDeInit(RTC_HandleTypeDef *hrtc);


#endif
