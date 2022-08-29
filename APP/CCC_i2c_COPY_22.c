#include "all_head.h"
//-----------------------------------------------

// #define I2C_SCL GPIOA->PX0_DOUT
// #define I2C_SDA GPIOB->PX10_DOUT

#define CCC_I2C_SCL GPIOD, GPIO_PIN_5
#define CCC_I2C_SDA GPIOD, GPIO_PIN_4

#define I2C_SCL_H GPIOD->ODSET = GPIO_PIN_5 // HAL_GPIO_WritePin(CCC_I2C_SCL, GPIO_PIN_SET)
#define I2C_SCL_L GPIOD->ODCLR = GPIO_PIN_5 // HAL_GPIO_WritePin(CCC_I2C_SCL, GPIO_PIN_RESET)
#define I2C_SDA_H GPIOD->ODSET = GPIO_PIN_4 // HAL_GPIO_WritePin(CCC_I2C_SDA, GPIO_PIN_SET)
#define I2C_SDA_L GPIOD->ODCLR = GPIO_PIN_4 // HAL_GPIO_WritePin(CCC_I2C_SDA, GPIO_PIN_RESET)

#define SET_I2C_SDA_Out GPIOD->DIRCR |= GPIO_PIN_4 // Set_SDA_Out()
#define SET_I2C_SDA_In GPIOD->DIRCR &= ~(GPIO_PIN_4)
/*
  @brief    初始化I2C
  @tparam
  @param
  @return
*/
void CCC_I2C_Init_COPY_22(void)
{
    // GPIO_ModeConfig(CCC_I2C_SCL, GPIO_Mode_OUT_PP); //推挽输出
    // GPIO_ModeConfig(CCC_I2C_SDA, GPIO_Mode_OUT_PP); //推挽输出
    // GPIO_PupdConfig(CCC_I2C_SCL, GPIO_PuPd_UP);
    // GPIO_PupdConfig(CCC_I2C_SDA, GPIO_PuPd_UP);

    GPIO_InitTypeDef gpioinitstruct = {0};
    gpioinitstruct.Pin = GPIO_PIN_4 | GPIO_PIN_5;
    gpioinitstruct.Mode = GPIO_MODE_OUTPUT;
    gpioinitstruct.OpenDrain = GPIO_PUSHPULL;
    gpioinitstruct.Debounce.Enable = GPIO_DEBOUNCE_DISABLE;
    gpioinitstruct.SlewRate = GPIO_SLEW_RATE_HIGH;
    gpioinitstruct.DrvStrength = GPIO_DRV_STRENGTH_HIGH;
    gpioinitstruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOD, &gpioinitstruct);

    I2C_SCL_H;
    I2C_SDA_H;
}

//===============================================
// I2C协议----起始信号
//===============================================
static void CCC_I2C_Start(void)
{
    I2C_SDA_H;
    I2C_SCL_H;
    I2C_SDA_L;
}
//===============================================
// I2C协议----结束信号
//===============================================
static void CCC_I2C_Stop(void)
{
    I2C_SCL_L;
    I2C_SDA_L;
    I2C_SCL_H;
    I2C_SDA_H;
}
//===============================================
// I2C协议----写字节
//===============================================
static void CCC_I2C_WriteByte(uint8_t data)
{
    uint8_t R_Count;

    for (R_Count = 8; R_Count > 0; R_Count--)
    {
        I2C_SCL_L; //原外
        if ((data & 0x80) != 0)
        {
            I2C_SDA_H; //核
            I2C_SCL_H; //原外
        }
        else
        {
            I2C_SDA_L; //核
            I2C_SCL_H; //原外
            // I2C_SDA_H; //省电 //不能!是停止信号
        }
        data <<= 1;
    }
}

/*
  @brief    等待应答信号到来
  @tparam
  @param
  @return   有无应答
*/
static bool CCC_IIC_WaitAck()
{
    //u8 ucErrTime = 0;

    I2C_SCL_L; //先低再输入,防止停止信号
    SET_I2C_SDA_In;
    I2C_SCL_H;

    // while (HAL_GPIO_ReadPin(CCC_I2C_SDA))
    // {
    // if (++ucErrTime > 250) //等待超时
    // }
    if (GPIOD->IDR & GPIO_PIN_4)
    {
        I2C_SCL_L; //时钟拉低,否则停止信号
        SET_I2C_SDA_Out;
        return false;
    }

    I2C_SCL_L; //时钟拉低,否则停止信号, 去掉不能正常通信
    SET_I2C_SDA_Out;
    return true;
}

//===============================================
// I2C协议----读字节
//===============================================
static uint8_t CCC_I2C_ReadByte(void)
{
    uint8_t R_Count, data = 0;
    I2C_SCL_L; //先低再输入,防止停止信号
    SET_I2C_SDA_In;

    for (R_Count = 8; R_Count > 0; R_Count--)
    {
        data <<= 1;
        I2C_SCL_H;
        // if (HAL_GPIO_ReadPin(CCC_I2C_SDA))
        if (GPIOD->IDR & GPIO_PIN_4)
            data++;
        I2C_SCL_L;
    }
    SET_I2C_SDA_Out; // NACK();
    I2C_SDA_H;       // NACK();
    I2C_SCL_H;       //应答位
    return data;
}

//===============================================
// I2C协议----写寄存器
//===============================================
void CCC_I2C_WriteReg_COPY_22(uint8_t WriteAddr, uint8_t WriteData)
{
IIC_WRITE_Begin:
    CCC_I2C_Start();
    CCC_I2C_WriteByte(SW3516H_WriteAddR); //发送
    if (CCC_IIC_WaitAck() == false)
    {
        CCC_I2C_Stop();
        goto IIC_WRITE_Begin;
    }
    CCC_I2C_WriteByte(WriteAddr);
    if (CCC_IIC_WaitAck() == false)
    {
        CCC_I2C_Stop();
        goto IIC_WRITE_Begin;
    }
    CCC_I2C_WriteByte(WriteData);
    if (CCC_IIC_WaitAck() == false)
    {
        CCC_I2C_Stop();
        goto IIC_WRITE_Begin;
    }
    CCC_I2C_Stop();
}

//===============================================
// I2C协议----读寄存器
//===============================================
uint8_t CCC_I2C_ReadReg_COPY_22(uint8_t ReadAddr)
{
IIC_READ_Begin:
    CCC_I2C_Start();
    CCC_I2C_WriteByte(SW3516H_WriteAddR); //发送
    if (CCC_IIC_WaitAck() == false)
    {
        CCC_I2C_Stop();
        goto IIC_READ_Begin;
    }
    CCC_I2C_WriteByte(ReadAddr);
    if (CCC_IIC_WaitAck() == false)
    {
        CCC_I2C_Stop();
        goto IIC_READ_Begin;
    }

    CCC_I2C_Start();
    CCC_I2C_WriteByte(SW3516H_ReadAddR); //接收
    if (CCC_IIC_WaitAck() == false)
    {
        CCC_I2C_Stop();
        goto IIC_READ_Begin;
    }
    u8 ReadData = CCC_I2C_ReadByte();
    CCC_I2C_Stop();
    return ReadData;
}
