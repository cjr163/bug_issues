
#ifndef __MAIN_H
#define __MAIN_H

/* Includes ------------------------------------------------------------------*/

/* Private includes ----------------------------------------------------------*/


typedef struct
{
    u8 Tick_cnt; //计数
    bool Tick1ms;
    bool Tick100ms;
    // u8 TimeShare; //分时
} SYSTEM_Tick_st;
extern SYSTEM_Tick_st SysTick_c ;
//extern bool power_on_first;
void Error_Handler(void);




#endif /* __MAIN_H */

