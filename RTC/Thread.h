#ifndef __THREAD_H
#define __THREAD_H
	int Init_Thread (void);  
	
	/* Defines related to Clock configuration */
#define RTC_ASYNCH_PREDIV  0x7F   /* LSE as RTC clock */
#define RTC_SYNCH_PREDIV   0x00FF /* LSE as RTC clock */
#endif