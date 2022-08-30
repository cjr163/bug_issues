/*--------------------------------------------------------------------------------
  @Author: ChenJiaRan
  @Date: 2022-06-28 16:11:26
  @LastEditTime  2022-08-29 20:13
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
  @brief    设置src_change=不重启直接广播新PDO ?
  @tparam
  @param
  @return
*/
void SW3516H_src_change()
{
    SW3516H_r0x73_st src_change;
    src_change.w = CCC_I2C_ReadReg((u32) & (SW3516H->src_change));
    src_change.SRC_Change = 1;
    CCC_I2C_WriteReg((u32) & (SW3516H->src_change), src_change.w);

    HAL_Delay(100);
}
/*
  @brief    解锁寄存器 reg0xB0~BF 注,不只B0-BF 断CC0X76也要解锁 大循环开始就解锁
  @param
  @return
*/
void SW3516H_Enable_I2C_Write()
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
void SW3516H_Disable_I2C_Write()
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
void SW3516H_Set_Mode(bool force_edit)
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
    }
}

/*
  @brief    更改PD功率=更改20V PDO电流
  @tparam   注意 当Read_Emarker使能(reg0xB7[1]=0)时，电流的变化都需要写src_change命令或断CC才会重新广播
  @tparam   需3秒后检测无电流再断CC
  @param
  @return
*/
bool SW3516H_Set_20V_PDO_Curr(bool force_edit)
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
        SW3516H_Enable_I2C_Write();
        PD_Config4.Fixed_20V_current = new_0xB4.Fixed_20V_current;
        CCC_I2C_WriteReg((u32) & (SW3516H->PD_Config4), PD_Config4.w);

        HAL_Delay(100);

        return true; //并延时 800ms(600ms 及以上)；在外面延时
    }
    else
        return false;
}
/*
  @brief    发送复位指令
  @param
  @return
*/
void SW3516H_HardReset()
{
    SW3516H_r0x70_st PD_Command;
    PD_Command.w = CCC_I2C_ReadReg((u32) & (SW3516H->PD_Command));
    PD_Command.PD_Command_bit = 1; // HardReset；
    PD_Command.PD_Command_en = 1;
    CCC_I2C_WriteReg((u32) & (SW3516H->Connect), PD_Command.w);

    HAL_Delay(100);//800?
}

/*
  @brief    更改非 PD 功率  SW351XS可以通过写寄存器设置非 PD 快充以及低压直充和双口在线以外的功率输出；
  @tparam   特别注意： 对于需要更改 C 口所有协议的功率时， 除修改 PD 功率外， 还需要加入下面步骤的(2)~(4)， 以修改 C口其他协议的功率。
  @param    force_edit  是否不比较直写
  @return
*/
void SW3516H_Except_PD_W(bool force_edit)
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
    }
}

/*
  @brief    最高输出电压(除 PD 以外的协议)
  @param    force_edit  是否不比较直写
  @return
*/
void SW3516H_Except_PD_V(bool force_edit)
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
        SW3516H_Enable_I2C_Write();

        FC_Config2.EXCEPT_PD_MAX_Vol = new_0xBA.EXCEPT_PD_MAX_Vol;
        CCC_I2C_WriteReg((u32) & (SW3516H->FC_Config2), FC_Config2.w);

        HAL_Delay(100);
    }
}

/*
  @brief    设置C口是否检测空载
  @param    force_edit  是否不比较直写
  @return
*/
void SW3516H_set_PortC_Empty_Check(bool force_edit)
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
        SW3516H_Enable_I2C_Write();

        FC_Config3.PortC_Empty_Check = new_0xBC.PortC_Empty_Check;
        CCC_I2C_WriteReg((u32) & (SW3516H->FC_Config3), FC_Config3.w);

        HAL_Delay(100);
    }
}
/*
  @brief    是否使能SCP
  @param    force_edit  是否不比较直写
  @return
*/
void SW3516H_Enable__SCP(bool force_edit)
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
        SW3516H_Enable_I2C_Write();

        FC_Config1.SCP_EN = new_0xB9.SCP_EN;
        CCC_I2C_WriteReg((u32) & (SW3516H->FC_Config1), FC_Config1.w);

        HAL_Delay(100);
    }
}

/*
  @brief    双口同时打开时的每个端口限流值
  @param    force_edit      是否不比较直写
  @return
*/
void SW3516H_Both_CUR_LIM(bool force_edit)
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
        SW3516H_Enable_I2C_Write();

        CL_Config.Both_On_Current_Limiting = new_0xBD.Both_On_Current_Limiting;
        CCC_I2C_WriteReg((u32) & (SW3516H->CL_Config), CL_Config.w);

        HAL_Delay(100);
    }
}
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/*
  @brief    C口状态检测1 (AC口产品)
  @param
  @return   true=打开状态,false=关闭状态
*/
bool SW3516H_Check_Port_C_1()
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
bool SW3516H_Check_Port_C_2()
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
SW3516H_r0x07_st SW3516H_Check_Port()
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
bool SW3516H_Check_Port_A_1()
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
u8 SW3516H_Read_AD_Value_H()
{
    return CCC_I2C_ReadReg((u32) & (SW3516H->ADC_Val_H));
}

/*
  @brief    读取AD值12位 （A口状态检测2 (AC口或者 AA口产品)） (判断 A 口是否工作和被使用， 电流门限建议设置在 20~500ma)
  @param    sw_Device   通讯IC选择
  @return   12bit
*/
u16 SW3516H_Read_AD_Value()
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
u16 SW3516H_Read_Vin_Vol(Device_enum sw_Device)
{
    // Enable_I2C_Write();

    SW3516H_r0x13_st ADC_Vin_ctr;
    ADC_Vin_ctr.w = CCC_I2C_ReadReg((u32) & (SW3516H->ADC_Vin_ctr));
    ADC_Vin_ctr.ADC_Vin_bit = 1;
    CCC_I2C_WriteReg((u32) & (SW3516H->ADC_Vin_ctr), ADC_Vin_ctr.w);

    SW3516H_set_ADC_Source(adc_vin);
    // SW3516H_r0x3A_st ADC_ctr;
    // ADC_ctr.w = CCC_I2C_ReadReg((u32) & (SW3516H->ADC_ctr));
    // ADC_ctr.ADC_SEL = adc_vin;
    // CCC_I2C_WriteReg((u32) & (SW3516H->ADC_ctr), ADC_ctr.w);

    u16 ADC_Val_H = CCC_I2C_ReadReg((u32) & (SW3516H->ADC_Val_H));
    ADC_Val_H <<= 4;
    SW3516H_r0x3C_st ADC_Val_L;
    ADC_Val_L.w = CCC_I2C_ReadReg((u32) & (SW3516H->ADC_Val_L));
    ADC_Val_H += ADC_Val_L.b4;
    // HAL_Delay(100);
    return ADC_Val_H;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/*
  @brief    设置要读的ADC数据来源
  @param    adc_sel     设置数据源
  @return
*/
void SW3516H_set_ADC_Source(ADC_SEL_enum adc_sel)
{
    // Enable_I2C_Write();

    SW3516H_r0x3A_st ADC_ctr;
    ADC_ctr.w = CCC_I2C_ReadReg((u32) & (SW3516H->ADC_ctr));
    ADC_ctr.ADC_SEL = adc_sel;
    CCC_I2C_WriteReg((u32) & (SW3516H->ADC_ctr), ADC_ctr.w);

    HAL_Delay(100);
}

/*
  @brief    是否开启PDO 20V电压
  @param    force_edit  是否不比较直写
  @return
*/
void SW3516H_OpenPDO_20V(bool force_edit)
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
        //PD_Config7.Read_Emarker=new_0xB7.Read_Emarker;
        // PD_Config7.PD_9V_PDO=new_0xB7.PD_9V_PDO;
        // PD_Config7.PD_12V_PDO=new_0xB7.;
        // PD_Config7.Read_Emarker=new_0xB7.Read_Emarker;
        // PD_Config7.Read_Emarker=new_0xB7.Read_Emarker;
        // PD_Config7.Read_Emarker=new_0xB7.Read_Emarker;
        // PD_Config7.Read_Emarker=new_0xB7.Read_Emarker;
        // PD_Config7.Read_Emarker=new_0xB7.Read_Emarker;
        // PD_Config7.Read_Emarker=new_0xB7.Read_Emarker;
        SW3516H_Enable_I2C_Write();
        CCC_I2C_WriteReg((u32) & (SW3516H->PD_Config7), PD_Config7.w);

        HAL_Delay(100);

        SW3516H_src_change();
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
        SW3516H_Enable_I2C_Write();
        PD_Config9.w = new_0xBE.w;
        CCC_I2C_WriteReg((u32) & (SW3516H->PD_Config9), PD_Config9.w);
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
        SW3516H_Enable_I2C_Write();
        PPS0_Config5.w = new_0xB5.w;
        CCC_I2C_WriteReg((u32) & (SW3516H->PPS0_Config5), PPS0_Config5.w);
    }

    //本案无PPS1
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
        SW3516H_Enable_I2C_Write();
        PPS1_Config6.w = new_0xB6.w;
        CCC_I2C_WriteReg((u32) & (SW3516H->PPS1_Config6), PPS1_Config6.w);
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
        SW3516H_Enable_I2C_Write();
        PD_Config8.PPS_Auto_DIS = new_0xB8.PPS_Auto_DIS;
        CCC_I2C_WriteReg((u32) & (SW3516H->PD_Config8), PD_Config8.w);
    }

    //开关PPS要写src_change
    if (is_modif)
    {
        SW3516H_src_change();
    }
}

/*
  @brief    单口转双口时,DMDP是否有效 (无效时C口1.5A)
  @param    force_edit  是否不比较直写
  @return
*/
void SW3516H_Set_DMDP(bool force_edit)
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
        SW3516H_Enable_I2C_Write();

        CL_Config.DPDM_connfig = new_0xBD.DPDM_connfig;
        CCC_I2C_WriteReg((u32) & (SW3516H->CL_Config), CL_Config.w);

        HAL_Delay(100);
    }
}

/*
  @brief    强制断开CC再重连 并延时800ms
  @param
  @return
*/
void SW3516H_Disconnect_CC()
{
    SW3516H_Enable_I2C_Write();

    SW3516H_r0x76_st Connect;
    Connect.w = CCC_I2C_ReadReg((u32) & (SW3516H->Connect));
    Connect.Prohibit_CC = 1; //强制断开 CC；
    Connect.Prohibit_BC = 1;
    CCC_I2C_WriteReg((u32) & (SW3516H->Connect), Connect.w);

    HAL_Delay(100);

    Connect.Prohibit_CC = 0; // 恢复 CC 连接以及 DP/DM
    Connect.Prohibit_BC = 0;
    CCC_I2C_WriteReg((u32) & (SW3516H->Connect), Connect.w); // 恢复 CC 连接以及 DP/DM；

    HAL_Delay(800); //并延时 800ms(600ms 及以上)；
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

