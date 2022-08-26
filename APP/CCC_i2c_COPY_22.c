#include "all_head.h"
//-----------------------------------------------

// #define I2C_SCL GPIOA->PX0_DOUT
// #define I2C_SDA GPIOB->PX10_DOUT

#define CCC_I2C_SCL GPIOD, GPIO_PIN_5
#define CCC_I2C_SDA GPIOD, GPIO_PIN_4

#define I2C_SCL_H HAL_GPIO_WritePin(CCC_I2C_SCL, GPIO_PIN_SET)
#define I2C_SCL_L HAL_GPIO_WritePin(CCC_I2C_SCL, GPIO_PIN_RESET)
#define I2C_SDA_H HAL_GPIO_WritePin(CCC_I2C_SDA, GPIO_PIN_SET)
#define I2C_SDA_L HAL_GPIO_WritePin(CCC_I2C_SDA, GPIO_PIN_RESET)

#define SET_I2C_SDA_Out Set_SDA_Out()
#define SET_I2C_SDA_In Set_SDA_In()

static void Set_SDA_In()
{
    GPIO_InitTypeDef gpioinitstruct;
    gpioinitstruct.Pin = GPIO_PIN_4;
    gpioinitstruct.Mode = GPIO_MODE_INPUT;
    gpioinitstruct.OpenDrain = GPIO_PUSHPULL;
    gpioinitstruct.Debounce.Enable = GPIO_DEBOUNCE_DISABLE;
    gpioinitstruct.SlewRate = GPIO_SLEW_RATE_HIGH;
    gpioinitstruct.DrvStrength = GPIO_DRV_STRENGTH_HIGH;
    gpioinitstruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOD, &gpioinitstruct);
}
static void Set_SDA_Out()
{
    GPIO_InitTypeDef gpioinitstruct;
    gpioinitstruct.Pin = GPIO_PIN_4;
    gpioinitstruct.Mode = GPIO_MODE_OUTPUT;
    gpioinitstruct.OpenDrain = GPIO_PUSHPULL;
    gpioinitstruct.Debounce.Enable = GPIO_DEBOUNCE_DISABLE;
    gpioinitstruct.SlewRate = GPIO_SLEW_RATE_HIGH;
    gpioinitstruct.DrvStrength = GPIO_DRV_STRENGTH_HIGH;
    gpioinitstruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOD, &gpioinitstruct);
}

static void Delay()
{
    uint32_t delay = 20;//20=100K 5=200k
    while (delay-- > 0)
    {
        __NOP();
    }
}
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

    // #define I2C_SCL GPIOA->PX0_DOUT
    // #define I2C_SDA GPIOB->PX10_DOUT
    /*
        另一个m0芯片
        GCR->PA_L_MFP |= 0xF;        // PA0_SCL
        GCR->PB_H_MFP |= (0xF << 8); // PB10_SDA
        GPIOB->PX_PMD |= GPIO_PMD_OUT_OD << 20;
        GPIOA->PX_PMD |= GPIO_PMD_OUT_OD << 0;
        GPIOB->PX_DOUT |= (1 << 10);
        GPIOA->PX_DOUT &= ~(1 << 0);
        // SDA_OUT;
        // SCL_OUT;
        //    SDA_SET1;
        //    SCL_SET1;
        */
}

//===============================================
// I2C协议----起始信号
//===============================================
static void CCC_I2C_Start(void)
{
    I2C_SDA_H;
    Delay();
    I2C_SCL_H;
    Delay();
    I2C_SDA_L;
    Delay();
    I2C_SCL_L;
    Delay();
}
//===============================================
// I2C协议----结束信号
//===============================================
static void CCC_I2C_Stop(void)
{
    I2C_SDA_L;
    Delay();
    I2C_SCL_H;
    Delay();
    I2C_SDA_H;
    Delay();
}
//===============================================
// I2C协议----写字节
//===============================================
static void CCC_I2C_WriteByte(uint8_t data)
{
    uint8_t R_Count;

    I2C_SDA_H;
    for (R_Count = 8; R_Count > 0; R_Count--)
    {
        I2C_SCL_L;
        if ((data & 0x80) != 0)
            I2C_SDA_H;
        else
            I2C_SDA_L;
        Delay();
        I2C_SCL_H;
        Delay();
        data <<= 1;
    }

    I2C_SCL_L;
    Delay();
    // __set_PRIMASK(1);
    // // GPIOB->PX_PMD &= ~(3 << (10 * 2));
    // // GPIOB->PX_PMD |= (GPIO_Mode_IN << (10 * 2));
    // GPIO_ModeConfig(CCC_I2C_SDA, GPIO_Mode_IN);
    // __set_PRIMASK(0);

    // I2C_SCL_H;
    // I2C_SCL_L; /////////////////////////////没读取?不检查应答
    // I2C_SDA_H;
    // __set_PRIMASK(1);
    // // GPIOB->PX_PMD &= ~(3 << (10 * 2));
    // // GPIOB->PX_PMD |= (GPIO_Mode_OUT_PP << (10 * 2));
    // GPIO_ModeConfig(CCC_I2C_SDA, GPIO_Mode_OUT_PP);
    // __set_PRIMASK(0);
}

/*
  @brief    等待应答信号到来
  @tparam
  @param
  @return   有无应答
*/
static bool CCC_IIC_WaitAck()
{
    u8 ucErrTime = 0;
    SET_I2C_SDA_In;
    // __set_PRIMASK(1);
    // // GPIOB->PX_PMD &= ~(3 << (10 * 2));
    // // GPIOB->PX_PMD |= (GPIO_Mode_IN << (10 * 2));
    // GPIO_ModeConfig(CCC_I2C_SDA, GPIO_Mode_IN);
    // __set_PRIMASK(0);

    I2C_SDA_H;
    Delay();
    // DelayUs(1);
    I2C_SCL_H;
    // DelayUs(1);
    while (HAL_GPIO_ReadPin(CCC_I2C_SDA))
    {
        if (++ucErrTime > 250) //等待超时
        {
            SET_I2C_SDA_Out;
            return false;
        }
    }
    I2C_SCL_L; //时钟输出0

    SET_I2C_SDA_Out;
    // __set_PRIMASK(1);
    // // GPIOB->PX_PMD &= ~(3 << (10 * 2));
    // // GPIOB->PX_PMD |= (GPIO_Mode_OUT_PP << (10 * 2));
    // GPIO_ModeConfig(CCC_I2C_SDA, GPIO_Mode_OUT_PP);
    // __set_PRIMASK(0);
    return true;
}

//===============================================
// I2C协议----读字节
//===============================================
static uint8_t CCC_I2C_ReadByte(void)
{
    uint8_t R_Count, data = 0;
    SET_I2C_SDA_In;
    // __set_PRIMASK(1);
    // // GPIOB->PX_PMD &= ~(3 << (10 * 2));
    // // GPIOB->PX_PMD |= (GPIO_Mode_IN << (10 * 2));
    // GPIO_ModeConfig(CCC_I2C_SDA, GPIO_Mode_IN);
    // __set_PRIMASK(0);

    for (R_Count = 8; R_Count > 0; R_Count--)
    {
        data <<= 1;
        I2C_SCL_H;
        if (HAL_GPIO_ReadPin(CCC_I2C_SDA))
            data++;
        I2C_SCL_L;
        Delay();
    }
    I2C_SDA_H; //高不应答

    SET_I2C_SDA_Out;
    // __set_PRIMASK(1);
    // // GPIOB->PX_PMD &= ~(3 << (10 * 2));
    // // GPIOB->PX_PMD |= (GPIO_Mode_OUT_PP << (10 * 2));
    // GPIO_ModeConfig(CCC_I2C_SDA, GPIO_Mode_OUT_PP);
    // __set_PRIMASK(0);

    I2C_SCL_H;
    Delay();
    I2C_SCL_L;
    Delay();
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
