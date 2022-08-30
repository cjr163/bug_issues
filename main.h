
#ifndef __MAIN_H
#define __MAIN_H

/* Includes ------------------------------------------------------------------*/

/* Private includes ----------------------------------------------------------*/


typedef struct
{
    u32 Tick_cnt; //计数
    bool Tick1ms;
    bool Tick100ms;
    // u8 TimeShare; //分时
} SYSTEM_Tick_st;
extern SYSTEM_Tick_st SysTick_c ;
//extern bool power_on_first;
void Error_Handler(void);

uint16_t Slide_Average(uint16_t ADVal, uint32_t *pADSum, uint16_t ADAverage);


#endif /* __MAIN_H */

