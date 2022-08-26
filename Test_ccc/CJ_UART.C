/*--------------------------------------------------------------------------------
  @Author: ChenJiaRan
  @Date: 2022-07-07 17:39:23
  @LastEditTime  2022-08-23 10:09
  @LastEditors  ChenJiaRan
  @Description:
  @Version: V1.0
--------------------------------------------------------------------------------*/
#include "all_head.h"

#define UART_IO_SEND PC4

void UART_Send_Byte(u8 input)
{
    UART_IO_SEND = 1;
    DelayUs(1);
    UART_IO_SEND = 0; //发送起始位
    NOP();
    NOP();
    NOP();
    NOP();

    NOP();
    NOP();
    NOP();
    NOP();
    NOP();
    NOP();
    NOP();
    //发送8位数据位
    // for (u8 i = 0; i < 8; i++)
    // {
    //     if (input & 0x01) //先传低位
    //         UART_IO_SEND = 1;
    //     else
    //     {
    //         UART_IO_SEND = 0;
    //         NOP();
    //         NOP();
    //         NOP();
    //     }
    //     NOP();
    //     input >>= 1;
    // }
    u8 i = 0;
    while (1)
    {
        if (input & 0x01) //先传低位
            UART_IO_SEND = 1;
        else
        {
            UART_IO_SEND = 0;
            NOP();
            NOP();
            NOP();
        }
        NOP();
        input >>= 1;

        if (++i >= 8)
        {
            NOP();
            NOP();
            break;
            // continue;
        }
    }

    NOP();
    // UART_IO_SEND = 1; //发送校验位(无)
    // NOP();
    // NOP();
    // NOP();
    // NOP();
    // NOP();
    // NOP();
    // NOP();
    // NOP();

    // NOP();
    // NOP();
    // NOP();
    // NOP();
    // NOP();
    // NOP();

    UART_IO_SEND = 1; //发送结束位

    DelayUs(2);
    // UART_IO_SEND = 1;
    // DelayUs(1);
    asm("NOP");
}
