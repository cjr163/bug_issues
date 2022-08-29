/*--------------------------------------------------------------------------------
  @Author: ChenJiaRan
  @Date: 2022-06-28 16:11:26
  @LastEditTime  2022-08-29 11:39
  @LastEditors  ChenJiaRan
  @Description: 智融SW3516H A+C双口 输出100W 输出5A
  @Version: V1.0

  在上电初始化的时候，为保证通讯正常，建议 Master 先让 IIC 保持一定时间的高电平后再开始通讯，避免因为不同芯片初始化时间不一致而发生通讯失败的现象。
  单字节读写，不支持连续读写
I2C 100/400KHZ (测试目前为66KHZ)
为了保证 Sink 设备在动态分配输出功率前后不出现异常，需要在 HardReset 期间更改 PDO 配置

--------------------------------------------------------------------------------*/
#include "all_head.h"

Device_enum Switch_Device;        //切换当前通讯IC
SW3516_Setting_st SW3516_Setting; //保存设定值
/*
  @brief    设置src_change
  @tparam
  @param
  @return
*/
void set_src_change()
{
    SW3516H_r0x73_st src_change;
    src_change.w = CCC_I2C_ReadReg((u32) & (SW3516H->src_change));
    src_change.SRC_Change = 1;
    CCC_I2C_WriteReg((u32) & (SW3516H->src_change), src_change.w);
    HAL_Delay(100);
    // I2cWrite(sw_Device, (u8) & (SW3516H->I2C_REG_Write), 0x00);
}
/*
  @brief    解锁寄存器 reg0xB0~BF 注,不只B0-BF 断CC0X76也要解锁 大循环开始就解锁
  @param
  @return
*/
void Enable_I2C_Write()
{
    if (Switch_Device == SW_IC_1)
    {
        if (SW3516_Setting.Enable_IC1_Write == false)
        {
            SW3516_Setting.Enable_IC1_Write = true;
            CCC_I2C_WriteReg((u32) & (SW3516H->I2C_REG_Write), 0x20);
            CCC_I2C_WriteReg((u32) & (SW3516H->I2C_REG_Write), 0x40);
            CCC_I2C_WriteReg((u32) & (SW3516H->I2C_REG_Write), 0x80);
        }
    }
    else
    {
        if (SW3516_Setting.Enable_IC2_Write == false)
        {
            SW3516_Setting.Enable_IC2_Write = true;
            CCC_I2C_WriteReg((u32) & (SW3516H->I2C_REG_Write), 0x20);
            CCC_I2C_WriteReg((u32) & (SW3516H->I2C_REG_Write), 0x40);
            CCC_I2C_WriteReg((u32) & (SW3516H->I2C_REG_Write), 0x80);
        }
    }
}

/*
  @brief    锁寄存器
  @param
  @return
*/
void Disable_I2C_Write()
{
    SW3516_Setting.Enable_IC1_Write = false;
    Switch_Device = SW_IC_1;
    CCC_I2C_WriteReg((u32) & (SW3516H->I2C_REG_Write), 0x00);

    SW3516_Setting.Enable_IC2_Write = false;
    Switch_Device = SW_IC_2;
    CCC_I2C_WriteReg((u32) & (SW3516H->I2C_REG_Write), 0x00);
}
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/*
  @brief    芯片端口设置
  @param    force_edit  是否不比较直写
  @return
*/
void sw3516_Set_Mode(bool force_edit)
{
    SW3516H_r0xAB_st new_0xAB;
    if (Switch_Device == SW_IC_1)
        new_0xAB = SW3516_Setting.IC1.u0xAB;
    else
        new_0xAB = SW3516_Setting.IC2.u0xAB;

    SW3516H_r0xAB_st Port_Config;
    Port_Config.w = CCC_I2C_ReadReg((u32) & (SW3516H->Port_Config));
    if (force_edit || Port_Config.port_cfg != new_0xAB.port_cfg)
    {
        // Enable_I2C_Write();

        Port_Config.port_cfg = new_0xAB.port_cfg;
        CCC_I2C_WriteReg((u32) & (SW3516H->Port_Config), Port_Config.w);
        HAL_Delay(100);

        // Disable_I2C_Write(sw_Device);
    }
}
/*
  @brief    更改PD功率方式一
  @param    sw_Device I2C引脚选择 此方案只能是sw_Device_1C 或 sw_Device_2AC
  @param    power_num 新的电流值/50mA(最大值127) 例:1000mA/50mA = 20  注:为0时，广播的电流由PPS0_current决定，需要写 src change 命令
  @return
*/
void sw3516_Change_PD_1()
{
    SW3516H_r0x70_st PD_Command;
    PD_Command.w = CCC_I2C_ReadReg((u32) & (SW3516H->PD_Command));
    PD_Command.PD_Command_bit = 1; // HardReset；
    PD_Command.PD_Command_en = 1;
    CCC_I2C_WriteReg((u32) & (SW3516H->Connect), PD_Command.w);
    HAL_Delay(100);

    // u8 temp = I2cRead(sw_Device, (u8) & (SW3516->PD_Config4));
    // (*(PD_Config4_st *)&temp).Fixed_20V_current = power_num_7bit; //配置的功率参数；
    // I2cWrite(sw_Device, (u8) & (SW3516->PD_Config4), temp);

    // DelayMs(100);
    // I2cWrite(sw_Device, (u8) & (SW3516->I2C_REG_Write), 0x00);
}

/*
  @brief    更改PD功率方式二 : 多口方案中使用切断CC的方式，优化兼容性(推荐使用)
  @param    force_edit  是否不比较直写
  @return
*/
bool sw3516_Change_PD_2(bool force_edit)
{
    SW3516H_r0xB4_st new_0xB4;
    if (Switch_Device == SW_IC_1)
        new_0xB4 = SW3516_Setting.IC1.u0xB4;
    else
        new_0xB4 = SW3516_Setting.IC2.u0xB4;

    SW3516H_r0xB4_st PD_Config4;
    PD_Config4.w = CCC_I2C_ReadReg((u32) & (SW3516H->PD_Config4));

    if (force_edit || PD_Config4.Fixed_20V_current != new_0xB4.Fixed_20V_current)
    {
        Enable_I2C_Write();
        // CCC_I2C_WriteReg((u32) & (SW3516H->Power_REG_Write), 0x20);
        // CCC_I2C_WriteReg((u32) & (SW3516H->Power_REG_Write), 0x40);
        // CCC_I2C_WriteReg((u32) & (SW3516H->Power_REG_Write), 0x80);

        // SW3516H_r0x76_st Connect;

        // Connect.w = CCC_I2C_ReadReg((u32) & (SW3516H->Connect));
        // Connect.Prohibit_CC = 1;
        // Connect.Prohibit_BC = 1; //强制断开 CC 以及 DP/DM；
        // CCC_I2C_WriteReg((u32) & (SW3516H->Connect), Connect.w);

        PD_Config4.Fixed_20V_current = new_0xB4.Fixed_20V_current; //配置的功率参数；
        CCC_I2C_WriteReg((u32) & (SW3516H->PD_Config4), PD_Config4.w);

        // Connect.w = CCC_I2C_ReadReg((u32) & (SW3516H->Connect));
        // Connect.Prohibit_CC = 0;
        // Connect.Prohibit_BC = 0;
        // CCC_I2C_WriteReg((u32) & (SW3516H->Connect), Connect.w); // 恢复 CC 连接以及 DP/DM；

        // CCC_I2C_WriteReg((u32) & (SW3516H->Power_REG_Write), 0x00);

        // I2cWrite(sw_Device, (u8) & (SW3516H->I2C_REG_Write), 0x00);

        return true; //并延时 800ms(600ms 及以上)；在外面延时
    }
    else
        return false;
}

/*
  @brief    更改PD功率方式三 : 不重启直接广播新PDO
  @tparam   需2秒后检测无电流再重启
  @param
  @return
*/
void sw3516_Change_PD_3(bool force_edit)
{
    SW3516H_r0xB4_st new_0xB4;
    if (Switch_Device == SW_IC_1)
        new_0xB4 = SW3516_Setting.IC1.u0xB4;
    else
        new_0xB4 = SW3516_Setting.IC2.u0xB4;

    SW3516H_r0xB4_st PD_Config4;
    PD_Config4.w = CCC_I2C_ReadReg((u32) & (SW3516H->PD_Config4));

    if (force_edit || PD_Config4.Fixed_20V_current != new_0xB4.Fixed_20V_current)
    {
        Enable_I2C_Write();

        // if (Switch_Device == SW_IC_1)
        // {
        //         Check_Curr.C1_check_Curr_2S = true;
        //         Check_Curr.C1_cnt = 0;
        // }
        // else
        // {
        //         Check_Curr.C2_check_Curr_2S = true;
        //         Check_Curr.C2_cnt = 0;
        // }

        PD_Config4.Fixed_20V_current = new_0xB4.Fixed_20V_current;
        CCC_I2C_WriteReg((u32) & (SW3516H->PD_Config4), PD_Config4.w);

        set_src_change();

        HAL_Delay(100);
    }
}
/*
  @brief    更改非 PD 功率  SW351XS可以通过写寄存器设置非 PD 快充以及低压直充和双口在线以外的功率输出；
  @tparam   特别注意： 对于需要更改 C 口所有协议的功率时， 除修改 PD 功率外， 还需要加入下面步骤的(2)~(4)， 以修改 C口其他协议的功率。
  @param    force_edit  是否不比较直写
  @return
*/
void sw3516_Except_PD_W(bool force_edit)
{
    SW3516H_r0xA6_st new_0xA6;
    if (Switch_Device == SW_IC_1)
        new_0xA6 = SW3516_Setting.IC1.u0xA6;
    else
        new_0xA6 = SW3516_Setting.IC2.u0xA6;

    SW3516H_r0xA6_st Power_Config;
    Power_Config.w = CCC_I2C_ReadReg((u32) & (SW3516H->Power_Config));

    if (force_edit || Power_Config.Power_Watt != new_0xA6.Power_Watt)
    {
        // Enable_I2C_Write();
        if (force_edit)
            Power_Config.budin = new_0xA6.budin; //上电写一次补丁

        Power_Config.Power_Watt = new_0xA6.Power_Watt;
        CCC_I2C_WriteReg((u32) & (SW3516H->Power_Config), Power_Config.w);

        HAL_Delay(100);
        // I2cWrite(sw_Device, (u8) & (SW3516H->I2C_REG_Write), 0x00);
    }
}

/*
  @brief    最高输出电压(除 PD 以外的协议)
  @param    force_edit  是否不比较直写
  @return
*/
void sw3516_Except_PD_V(bool force_edit)
{
    SW3516H_r0xBA_st new_0xBA;
    if (Switch_Device == SW_IC_1)
        new_0xBA = SW3516_Setting.IC1.u0xBA;
    else
        new_0xBA = SW3516_Setting.IC2.u0xBA;

    SW3516H_r0xBA_st FC_Config2;
    FC_Config2.w = CCC_I2C_ReadReg((u32) & (SW3516H->FC_Config2));

    if (force_edit || FC_Config2.EXCEPT_PD_MAX_Vol != new_0xBA.EXCEPT_PD_MAX_Vol)
    {
        Enable_I2C_Write();

        FC_Config2.EXCEPT_PD_MAX_Vol = new_0xBA.EXCEPT_PD_MAX_Vol;
        CCC_I2C_WriteReg((u32) & (SW3516H->FC_Config2), FC_Config2.w);

        HAL_Delay(100);
        // I2cWrite(sw_Device, (u8) & (SW3516H->I2C_REG_Write), 0x00);
    }
}

/*
  @brief    设置C口是否检测空载
  @param    force_edit  是否不比较直写
  @return
*/
void sw3516_set_PortC_Empty_Check(bool force_edit)
{
    SW3516H_r0xBC_st new_0xBC;
    if (Switch_Device == SW_IC_1)
        new_0xBC = SW3516_Setting.IC1.u0xBC;
    else
        new_0xBC = SW3516_Setting.IC2.u0xBC;

    SW3516H_r0xBC_st FC_Config3;
    FC_Config3.w = CCC_I2C_ReadReg((u32) & (SW3516H->FC_Config3));

    if (force_edit || FC_Config3.PortC_Empty_Check != new_0xBC.PortC_Empty_Check)
    {
        Enable_I2C_Write();

        FC_Config3.PortC_Empty_Check = new_0xBC.PortC_Empty_Check;
        CCC_I2C_WriteReg((u32) & (SW3516H->FC_Config3), FC_Config3.w);

        HAL_Delay(100);
        // I2cWrite(sw_Device, (u8) & (SW3516H->I2C_REG_Write), 0x00);
    }
}
/*
  @brief    是否使能SCP
  @param    force_edit  是否不比较直写
  @return
*/
void sw3516_Enable__SCP(bool force_edit)
{
    SW3516H_r0xB9_st new_0xB9;
    if (Switch_Device == SW_IC_1)
        new_0xB9 = SW3516_Setting.IC1.u0xB9;
    else
        new_0xB9 = SW3516_Setting.IC2.u0xB9;

    SW3516H_r0xB9_st FC_Config1;
    FC_Config1.w = CCC_I2C_ReadReg((u32) & (SW3516H->FC_Config1));

    if (force_edit || FC_Config1.SCP_EN != new_0xB9.SCP_EN)
    {
        Enable_I2C_Write();

        FC_Config1.SCP_EN = new_0xB9.SCP_EN;
        CCC_I2C_WriteReg((u32) & (SW3516H->FC_Config1), FC_Config1.w);

        HAL_Delay(100);
        // I2cWrite(sw_Device, (u8) & (SW3516H->I2C_REG_Write), 0x00);
    }
}

/*
  @brief    双口同时打开时的每个端口限流值
  @param    force_edit      是否不比较直写
  @return
*/
void sw3516_Both_CUR_LIM(bool force_edit)
{
    SW3516H_r0xBD_st new_0xBD;
    if (Switch_Device == SW_IC_1)
        new_0xBD = SW3516_Setting.IC1.u0xBD;
    else
        new_0xBD = SW3516_Setting.IC2.u0xBD;

    SW3516H_r0xBD_st CL_Config;
    CL_Config.w = CCC_I2C_ReadReg((u32) & (SW3516H->CL_Config));

    if (force_edit || CL_Config.Both_On_Current_Limiting != new_0xBD.Both_On_Current_Limiting)
    {
        Enable_I2C_Write();

        CL_Config.Both_On_Current_Limiting = new_0xBD.Both_On_Current_Limiting;
        CCC_I2C_WriteReg((u32) & (SW3516H->CL_Config), CL_Config.w);

        HAL_Delay(100);
        // I2cWrite(sw_Device, (u8) & (SW3516H->I2C_REG_Write), 0x00);
    }
}
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/*
  @brief    C口状态检测1 (AC口产品)
  @param
  @return   true=打开状态,false=关闭状态
*/
bool sw3516_Check_Port_C_1()
{
    SW3516H_r0x08_st System_state1;
    System_state1.w = CCC_I2C_ReadReg((u32) & (SW3516H->System_state1));
    if ((System_state1.Device_Exists == AC_ONLY_C) || (System_state1.Device_Exists == AC_BOTH))
        return true;
    else
        return false;
}
/*
  @brief    C口状态检测2 (单C口产品)
  @tparam
  @param
  @return   true=打开状态,false=关闭状态
*/
bool sw3516_Check_Port_C_2()
{
    SW3516H_r0x08_st System_state1;
    System_state1.w = CCC_I2C_ReadReg((u32) & (SW3516H->System_state1));
    if (System_state1.Device_Exists == SINGLE_C_EXIST)
        return true;
    else
        return false;
}

/*
  @brief    读取0x07寄存器 ( C口状态检测2 (C口产品) )
  @param
  @return   SW3516 0x07寄存器
*/
SW3516H_r0x07_st sw3516_Check_Port()
{
    SW3516H_r0x07_st System_state0;
    System_state0.w = CCC_I2C_ReadReg((u32) & (SW3516H->System_state0));
    return System_state0;
}

/*
  @brief    A口状态检测1 (AC口 或者 AA口产品)
  @param
  @return   true=A口有设备,false=没有设备接入
*/
bool sw3516_Check_Port_A_1()
{

    SW3516H_r0x08_st System_state1;
    System_state1.w = CCC_I2C_ReadReg((u32) & (SW3516H->System_state1));
    if ((System_state1.Device_Exists == AC_ONLY_A) || (System_state1.Device_Exists == AC_BOTH))
        return true;
    else
        return false;
}
/*
  @brief    读取AD值高8位
  @param    sw_Device   通讯IC选择
  @return   8bit
*/
u8 sw3516_Read_AD_Value_H()
{
    return CCC_I2C_ReadReg((u32) & (SW3516H->ADC_Val_H));
}

/*
  @brief    读取AD值12位 （A口状态检测2 (AC口或者 AA口产品)） (判断 A 口是否工作和被使用， 电流门限建议设置在 20~500ma)
  @param    sw_Device   通讯IC选择
  @return   12bit
*/
u16 sw3516_Read_AD_Value()
{
    u16 ADC_Val_H = CCC_I2C_ReadReg((u32) & (SW3516H->ADC_Val_H));
    ADC_Val_H <<= 4;
    u8 ADC_Val_L = CCC_I2C_ReadReg((u32) & (SW3516H->ADC_Val_L));
    ADC_Val_H += ADC_Val_L;
    return ADC_Val_H;
}

/*
  @brief    Vin电压读取
  @param    sw_Device   通讯IC选择
  @return   12bit
*/
u16 SW3516_Read_Vin_Vol(Device_enum sw_Device)
{
    // Enable_I2C_Write();

    SW3516H_r0x13_st ADC_Vin_ctr;
    ADC_Vin_ctr.w = CCC_I2C_ReadReg((u32) & (SW3516H->ADC_Vin_ctr));
    ADC_Vin_ctr.ADC_Vin_bit = 1;
    CCC_I2C_WriteReg((u32) & (SW3516H->ADC_Vin_ctr), ADC_Vin_ctr.w);

    SW3516H_r0x3A_st ADC_ctr;
    ADC_ctr.w = CCC_I2C_ReadReg((u32) & (SW3516H->ADC_ctr));
    ADC_ctr.ADC_SEL = adc_vin;
    CCC_I2C_WriteReg((u32) & (SW3516H->ADC_ctr), ADC_ctr.w);

    u16 ADC_Val_H = CCC_I2C_ReadReg((u32) & (SW3516H->ADC_Val_H));
    ADC_Val_H <<= 4;
    SW3516H_r0x3C_st ADC_Val_L;
    ADC_Val_L.w = CCC_I2C_ReadReg((u32) & (SW3516H->ADC_Val_L));
    ADC_Val_H += ADC_Val_L.b4;
    HAL_Delay(100);
    return ADC_Val_H;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/*
  @brief    设置要读的ADC数据来源
  @param    adc_sel     设置数据源
  @return
*/
void sw3516_set_ADC_Source(ADC_SEL_enum adc_sel)
{
    // Enable_I2C_Write();

    SW3516H_r0x3A_st ADC_ctr;
    ADC_ctr.w = CCC_I2C_ReadReg((u32) & (SW3516H->ADC_ctr));
    ADC_ctr.ADC_SEL = adc_sel;
    CCC_I2C_WriteReg((u32) & (SW3516H->ADC_ctr), ADC_ctr.w);

    // DelayMs(1);
    //  I2cWrite(sw_Device, (u8) & (SW3516H->I2C_REG_Write), 0x00);
}

/*
  @brief    是否开启PDO 20V电压
  @param    force_edit  是否不比较直写
  @return
*/
void sw3516_OpenPDO_20V(bool force_edit)
{
    SW3516H_r0xB7_st new_0xB7;
    if (Switch_Device == SW_IC_1)
        new_0xB7 = SW3516_Setting.IC1.u0xB7;
    else
        new_0xB7 = SW3516_Setting.IC2.u0xB7;

    SW3516H_r0xB7_st PD_Config7;
    PD_Config7.w = CCC_I2C_ReadReg((u32) & (SW3516H->PD_Config7));

    if (force_edit || PD_Config7.w != new_0xB7.w)
    {
        PD_Config7.w = new_0xB7.w;
        Enable_I2C_Write();
        CCC_I2C_WriteReg((u32) & (SW3516H->PD_Config7), PD_Config7.w);

        HAL_Delay(100);
        // PD_Config7.w = CCC_I2C_ReadReg((u32) & (SW3516H->PD_Config7));

        // I2cWrite(sw_Device, (u8) & (SW3516H->I2C_REG_Write), 0x00);
        set_src_change();
    }
}

/*
  @brief    更改PPS设置
  @tparam   force_edit  是否不比较直写
  @param
  @return
*/
void SW3516H_Set_PPS(bool force_edit)
{
    bool is_modif = false;

    SW3516H_r0xBE_st new_0xBE;
    if (Switch_Device == SW_IC_1)
        new_0xBE = SW3516_Setting.IC1.u0xBE;
    else
        new_0xBE = SW3516_Setting.IC2.u0xBE;

    SW3516H_r0xBE_st PD_Config9;
    PD_Config9.w = CCC_I2C_ReadReg((u32) & (SW3516H->PD_Config9));

    if (force_edit || PD_Config9.w != new_0xBE.w)
    {
        is_modif = true;
        Enable_I2C_Write();
        PD_Config9.w = new_0xBE.w;
        CCC_I2C_WriteReg((u32) & (SW3516H->PD_Config9), PD_Config9.w);

        HAL_Delay(100);
        // I2cWrite(sw_Device, (u8) & (SW3516H->I2C_REG_Write), 0x00);
    }

    SW3516H_r0xB5_st new_0xB5;
    if (Switch_Device == SW_IC_1)
        new_0xB5 = SW3516_Setting.IC1.u0xB5;
    else
        new_0xB5 = SW3516_Setting.IC2.u0xB5;

    SW3516H_r0xB5_st PPS0_Config5;
    PPS0_Config5.w = CCC_I2C_ReadReg((u32) & (SW3516H->PPS0_Config5));

    if (force_edit || PPS0_Config5.w != new_0xB5.w)
    {
        is_modif = true;
        Enable_I2C_Write();
        PPS0_Config5.w = new_0xB5.w;
        CCC_I2C_WriteReg((u32) & (SW3516H->PPS0_Config5), PPS0_Config5.w);

        HAL_Delay(100);
        // I2cWrite(sw_Device, (u8) & (SW3516H->I2C_REG_Write), 0x00);
    }

    SW3516H_r0xB6_st new_0xB6;
    if (Switch_Device == SW_IC_1)
        new_0xB6 = SW3516_Setting.IC1.u0xB6;
    else
        new_0xB6 = SW3516_Setting.IC2.u0xB6;
    SW3516H_r0xB6_st PPS1_Config6;
    PPS1_Config6.w = CCC_I2C_ReadReg((u32) & (SW3516H->PPS1_Config6));

    if (force_edit || PPS1_Config6.w != new_0xB6.w)
    {
        is_modif = true;
        Enable_I2C_Write();
        PPS1_Config6.w = new_0xB6.w;
        CCC_I2C_WriteReg((u32) & (SW3516H->PPS1_Config6), PPS1_Config6.w);

        HAL_Delay(100);
        // I2cWrite(sw_Device, (u8) & (SW3516H->I2C_REG_Write), 0x00);
    }

    SW3516H_r0xB8_st new_0xB8;
    if (Switch_Device == SW_IC_1)
        new_0xB8 = SW3516_Setting.IC1.u0xB8;
    else
        new_0xB8 = SW3516_Setting.IC2.u0xB8;
    SW3516H_r0xB8_st PD_Config8;
    PD_Config8.w = CCC_I2C_ReadReg((u32) & (SW3516H->PD_Config8));
    if (force_edit || PD_Config8.PPS_Auto_DIS != new_0xB8.PPS_Auto_DIS)
    {
        is_modif = true;
        Enable_I2C_Write();
        PD_Config8.PPS_Auto_DIS = new_0xB8.PPS_Auto_DIS;
        CCC_I2C_WriteReg((u32) & (SW3516H->PD_Config8), PD_Config8.w);

        HAL_Delay(100);
        // I2cWrite(sw_Device, (u8) & (SW3516H->I2C_REG_Write), 0x00);
    }

    //开关PPS要写src_change
    if (is_modif)
    {
        set_src_change();
    }
}

/*
  @brief    单口转双口时,DMDP是否有效 (无效时C口1.5A)
  @param    force_edit  是否不比较直写
  @return
*/
void sw3516_Set_DMDP(bool force_edit)
{
    SW3516H_r0xBD_st new_0xBD;
    if (Switch_Device == SW_IC_1)
        new_0xBD = SW3516_Setting.IC1.u0xBD;
    else
        new_0xBD = SW3516_Setting.IC2.u0xBD;

    SW3516H_r0xBD_st CL_Config;
    CL_Config.w = CCC_I2C_ReadReg((u32) & (SW3516H->CL_Config));
    if (force_edit || CL_Config.DPDM_connfig != new_0xBD.DPDM_connfig)
    {
        Enable_I2C_Write();

        CL_Config.DPDM_connfig = new_0xBD.DPDM_connfig;
        CCC_I2C_WriteReg((u32) & (SW3516H->CL_Config), CL_Config.w);

        // I2cWrite(sw_Device, (u8) & (SW3516H->I2C_REG_Write), 0x00);
        //  DelayMs(100);
        //   DelayMs(200);
        //   DelayMs(200);
        //   DelayMs(200); //并延时 800ms(600ms 及以上)；
    }
}

/*
  @brief    (三星) 强制断开CC DP/DM无效 重启CC
  @param
  @return
*/
void sw3516_OFF_CC()
{
    Enable_I2C_Write();
    // CCC_I2C_WriteReg((u32) & (SW3516H->Power_REG_Write), 0x20);
    // CCC_I2C_WriteReg((u32) & (SW3516H->Power_REG_Write), 0x40);
    // CCC_I2C_WriteReg((u32) & (SW3516H->Power_REG_Write), 0x80);

    SW3516H_r0x76_st Connect;
    Connect.w = CCC_I2C_ReadReg((u32) & (SW3516H->Connect));
    Connect.Prohibit_CC = 1;
    Connect.Prohibit_BC = 1; //强制断开 CC 以及 DP/DM；
    CCC_I2C_WriteReg((u32) & (SW3516H->Connect), Connect.w);

    {
        SW3516H_r0xBD_st CL_Config;
        CL_Config.w = CCC_I2C_ReadReg((u32) & (SW3516H->CL_Config));
        CL_Config.DPDM_connfig = 0;
        CCC_I2C_WriteReg((u32) & (SW3516H->CL_Config), CL_Config.w);

        if (Switch_Device == SW_IC_1)
            SW3516_Setting.IC1.u0xBD.DPDM_connfig = true;
        else
            SW3516_Setting.IC2.u0xBD.DPDM_connfig = true;
    }
    Connect.w = CCC_I2C_ReadReg((u32) & (SW3516H->Connect));
    Connect.Prohibit_CC = 0;
    Connect.Prohibit_BC = 0;
    CCC_I2C_WriteReg((u32) & (SW3516H->Connect), Connect.w); // 恢复 CC 连接以及 DP/DM；

    // CCC_I2C_WriteReg((u32) & (SW3516H->Power_REG_Write), 0x00);

    // I2cWrite(sw_Device, (u8) & (SW3516H->I2C_REG_Write), 0x00);
    //  DelayMs(200);
    //  DelayMs(200);
    //  DelayMs(200);
    //  DelayMs(200); //并延时 800ms(600ms 及以上)；
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
// void Delay800Ms()
// {
//     HAL_Delay(200); //并延时 800ms(600ms 及以上)；
//     HAL_Delay(200);
//     HAL_Delay(200);
//     HAL_Delay(200);
// }

/*
  @brief    AC同时插入重启CC 适应三星快充
  @param
  @return
*/
void check_Samsung()
{
    typedef enum
    {
        Samsung_none_port = 0,   //无插入
        Samsung_single_port = 1, //单口
        Samsung_double_port = 2  //双口
    } Samsung_state_enum;
    static Samsung_state_enum Samsung_check = Samsung_none_port; //三星插入检测
    static u32 cnt = 0;

    // C2存在但A2不存在,检测大于20mA时插入A口,重启CC限流
    Switch_Device = SW_IC_2;

    // SW3516H_r0x07_st rrr07;
    // rrr07.w = CCC_I2C_ReadReg((u32) & (SW3516H->System_state0));///0x07寄存器反应慢,不能做为判断条件

    SW3516H_r0x08_st System_state1;
    System_state1.w = CCC_I2C_ReadReg((u32) & (SW3516H->System_state1));

    switch (Samsung_check)
    {
    case Samsung_none_port:
        //无插入
        if (System_state1.Device_Exists == AC_ONLY_C || System_state1.Device_Exists == AC_ONLY_A)
        {
            if (++cnt >= 2)
            {
                cnt = 0;
                Samsung_check = Samsung_single_port;
            }
        }
        else
            cnt = 0;

        break;
    case Samsung_single_port:
        //单口检测变两口插入
        if (System_state1.Device_Exists == AC_BOTH)
        {
            if (++cnt >= 2)
            {
                cnt = 0;

                sw3516_set_ADC_Source(adc_iout1);

                if (sw3516_Read_AD_Value() > (u16)(20 / 2.5)) //如果大于20mA，关闭DMDP
                {
                    sw3516_OFF_CC(); //重启过程中强制断开CC，0x76；C口广播1.5A，DP/DM无效，0xBD。
                }
                else
                {
                    SW3516_Setting.IC2.u0xBD.DPDM_connfig = false;
                    sw3516_Set_DMDP(true);
                }
                HAL_Delay(800); //并延时 800ms(600ms 及以上)；
                Samsung_check = Samsung_double_port;
            }
        }
        else
            cnt = 0;

        if (System_state1.Device_Exists == AC_NONE)
        {
            Samsung_check = Samsung_none_port;
        }

        break;
    case Samsung_double_port:
        //双口检测拔出
        if (System_state1.Device_Exists != AC_BOTH)
        {
            if (++cnt >= 2)
            {
                cnt = 0;

                SW3516_Setting.IC2.u0xBD.DPDM_connfig = true;
                sw3516_Set_DMDP(true);
                HAL_Delay(800); //并延时 800ms(600ms 及以上)；
                Samsung_check = Samsung_none_port;
            }
        }
        else
            cnt = 0;

        break;
    default:
        break;
    }
}
