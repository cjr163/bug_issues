

// #include "SYSCFG.h"
// #include "Lib\CJ_Config.h"
// #include "Lib\CJ_FT61F02X.h"
// #include "Lib\SW3516.h"

#include "all_head.h"

#if DEBUG_UART
#include "Lib\CJ_UART.h"
#endif

typedef struct
{
    bool C1 : 1;
    bool C2 : 1;
    bool A2 : 1;
} Port_State_st;
Port_State_st Port_state = {0}; //稳态

typedef enum
{
    normal,
    SINGLE_C1,            //单C1插入 提前分配C2
    SINGLE_C2,            //单C1插入 提前分配C2
    SINGLE_A2,            //单C1插入 提前分配C2
    State_C1_45W__C2_20W, // C1和C2口插入 //C1先插入 大于16V
    State_C1_20W__C2_45W, // C1和C2口插入 //C2先插入 大于16V
    State_C1_30W__C2_30W  // C1和C2口插入
} State_enum;
State_enum Work_mod = {normal}; //根据各口状态切换功率

Check_Curr_st Check_Curr = {false, false,  0, 0};

/*
  @brief    主函数
  @param
  @return
*/
// void main()
// {
//  Switch_Device = SW_IC_1;
// CCC_I2C_ReadReg((u32) & (SW3516H->Chip_ver)); //可以先读一下版本，SW未应答会卡在这
// DelayMs(50);

// init_sw3516();
//    while (1)
//    {
//        check_Samsung();

//        if (Sys.Tick) // 100ms
//        {
//            Sys.Tick = false;
//            test_uart();

//            check_Port();
//            change_Power();
//            cycle_check(); // 周期检查芯片值是否正常
//        }
//        Disable_I2C_Write();
//        NOP();
//    }
// }

/*
  @brief    初始设定值
  @param
  @return
*/
void init_setting_reg()
{
    //单C1=65W；非PD功率60W，打开20V、SCP。
    // SW3516_Setting.IC1.u0x76.w = 0b00000011; // Prohibit_CC Prohibit_BC
    SW3516_Setting.IC1.u0xA6.Power_Watt = OTHER_THAN_PD_60W;
    SW3516_Setting.IC1.u0xA6.budin = 0; //补丁
    SW3516_Setting.IC1.u0xAB.port_cfg = MODE_SINGLE_C;

    SW3516_Setting.IC1.u0xB4.Fixed_20V_current = 65000 / 20 / 50; // 65W/20V/50unit

    SW3516_Setting.IC1.u0xB5.PPS0_current = 5000 / 50; //初始5A
    SW3516_Setting.IC1.u0xB5.PPS0_EN = 0;
    SW3516_Setting.IC1.u0xB6.PPS1_current = 5000 / 50; //初始5A
    SW3516_Setting.IC1.u0xB6.PPS1_EN = 0;

    SW3516_Setting.IC1.u0xB7.w = 0B01111101;   // PD_20V_PDO
    SW3516_Setting.IC1.u0xB8.PPS_Auto_DIS = 1; //补丁
    SW3516_Setting.IC1.u0xB9.SCP_EN = true;
    SW3516_Setting.IC1.u0xBA.EXCEPT_PD_MAX_Vol = EXCEPT_PD_MAX_20V;

    SW3516_Setting.IC1.u0xBC.PortC_Empty_Check = 0;
    SW3516_Setting.IC1.u0xBD.Both_On_Current_Limiting = BOTH_CUR_LIM_2p2A;
    SW3516_Setting.IC1.u0xBD.DPDM_connfig = true;

    SW3516_Setting.IC1.u0xBE.PPS0_VOL = 1;
    SW3516_Setting.IC1.u0xBE.PPS0_POWER_restrict = 0;
    SW3516_Setting.IC1.u0xBE.PPS0_H_VOL_EN = 0;
    SW3516_Setting.IC1.u0xBE.PPS1_VOL = 1;
    SW3516_Setting.IC1.u0xBE.PPS1_POWER_restrict = 0;
    SW3516_Setting.IC1.u0xBE.PPS1_H_VOL_EN = 0;

    SW3516_Setting.Enable_IC1_Write = false;

    // C2=65W， A2=24W（关闭20V，打开SCP），双口限流2.2A。

    SW3516_Setting.IC2.u0xA6.Power_Watt = OTHER_THAN_PD_18W;
    SW3516_Setting.IC2.u0xA6.budin = 0; //补丁
    SW3516_Setting.IC2.u0xAB.port_cfg = MODE_COMBINE_AC;

    SW3516_Setting.IC2.u0xB4.Fixed_20V_current = 65000 / 20 / 50; // 65W/20V/50unit

    SW3516_Setting.IC2.u0xB5.PPS0_current = 5000 / 50; //初始5A
    SW3516_Setting.IC2.u0xB5.PPS0_EN = 0;
    SW3516_Setting.IC2.u0xB6.PPS1_current = 5000 / 50; //初始5A
    SW3516_Setting.IC2.u0xB6.PPS1_EN = 0;
    SW3516_Setting.IC2.u0xB7.w = 0B01111101;   // PD_20V_PDO
    SW3516_Setting.IC2.u0xB8.PPS_Auto_DIS = 1; //补丁
    SW3516_Setting.IC2.u0xB9.SCP_EN = true;
    SW3516_Setting.IC2.u0xBA.EXCEPT_PD_MAX_Vol = EXCEPT_PD_MAX_12V;

    SW3516_Setting.IC2.u0xBC.PortC_Empty_Check = 0;
    SW3516_Setting.IC2.u0xBD.Both_On_Current_Limiting = BOTH_CUR_LIM_2p2A;
    SW3516_Setting.IC2.u0xBD.DPDM_connfig = true;

    SW3516_Setting.IC2.u0xBE.PPS0_VOL = 1;
    SW3516_Setting.IC2.u0xBE.PPS0_POWER_restrict = 0;
    SW3516_Setting.IC2.u0xBE.PPS0_H_VOL_EN = 0;
    SW3516_Setting.IC2.u0xBE.PPS1_VOL = 1;
    SW3516_Setting.IC2.u0xBE.PPS1_POWER_restrict = 0;
    SW3516_Setting.IC2.u0xBE.PPS1_H_VOL_EN = 0;

    SW3516_Setting.Enable_IC2_Write = false;
}
/*
  @brief    初始化SW3516
  @param
  @return
*/
void init_sw3516()
{
    init_setting_reg();

    Switch_Device = SW_IC_1;
    sw3516_Except_PD_W(true);
    sw3516_Set_Mode(true);
    sw3516_Change_PD_2(true);
    HAL_Delay(10); //跟后面一起延时

    Switch_Device = SW_IC_2;
    sw3516_Except_PD_W(true);
    sw3516_Set_Mode(true);
    sw3516_Change_PD_2(true);
    //HAL_Delay(800); //并延时 800ms(600ms 及以上)；
HAL_Delay(100);

    Switch_Device = SW_IC_1;
    SW3516H_Set_PPS(true);
    sw3516_Set_DMDP(true);
    sw3516_Except_PD_V(true);
    sw3516_OpenPDO_20V(true);
    sw3516_Enable__SCP(true);
    sw3516_Both_CUR_LIM(true);
    sw3516_set_PortC_Empty_Check(true);

    Switch_Device = SW_IC_2;
    SW3516H_Set_PPS(true);
    sw3516_Set_DMDP(true);
    sw3516_Except_PD_V(true);
    sw3516_OpenPDO_20V(true);
    sw3516_Enable__SCP(true);
    sw3516_Both_CUR_LIM(true);
    sw3516_set_PortC_Empty_Check(true);
    // sw3516_Init_PDO();
}

/*
  @brief    检测各口插入状态
  @param
  @return
*/
void check_Port()
{

    // A口插入根据电流判断
    Switch_Device = SW_IC_2;
    sw3516_set_ADC_Source(adc_vout); //读输出电压
    u8 AD_V = sw3516_Read_AD_Value_H();

    sw3516_set_ADC_Source(adc_iout2);
    u16 AD_I_W = sw3516_Read_AD_Value(); //读输出电流

    u16 adv_const;
    if (AD_I_W >> 8 > 0)
    {
        AD_I_W >>= 4;
        adv_const = (u16)(2500000 / (6 * 16) / (2.5 * 16)); // 2.5W = 651
    }
    else
        adv_const = (u16)(2500000 / (6 * 16) / 2.5); // 2.5W = 10416

    bool new_A2 = false; // A2口插入状态
    AD_I_W *= AD_V;
    if (AD_I_W >= adv_const)
        new_A2 = true; // A口电流大于2.5W判断插入
    else
    {
        // 拔出用状态位判断
        if (sw3516_Check_Port_A_1() == false)
            new_A2 = false;
    }

    bool new_C2 = sw3516_Check_Port_C_1(); // C2口读状态位

    Switch_Device = SW_IC_1;
    // bool new_C1 = sw3516_Check_Port().PortC_state; // C1口读状态位
    bool new_C1 = sw3516_Check_Port_C_2();

    static u8 C1_cnt = 0; //消抖计数 =20上电快速反应
    if (Port_state.C1 != new_C1)
    {
        if (++C1_cnt > 2)
        {
            C1_cnt = 0;
            Port_state.C1 = new_C1;

            if (Port_state.C1)
            { // C1插入
                // if (!Check_Curr.C1_breaking)
                //     Check_Curr.C1_new_insert = true;

                if (Port_state.C2 || Port_state.A2)
                { // C2已插入,是否大于16V
                    Switch_Device = SW_IC_2;
                    sw3516_set_ADC_Source(adc_vout); //读输出电压
                    u32 AD_V = sw3516_Read_AD_Value_H();
                    AD_V = AD_V * 6 * 16;
                    if (AD_V > 16000)
                        Work_mod = State_C1_20W__C2_45W;
                    else
                        Work_mod = State_C1_30W__C2_30W;
                }
                else
                    Work_mod = SINGLE_C1;
            }
            else
            { // C1拔出
//                if (!Check_Curr.C1_breaking)
//                {
                    if (Port_state.C2 || Port_state.A2)
                        Work_mod = SINGLE_C2;
                    else
                        Work_mod = normal;
//                }
            }
        }
    }
    else
        C1_cnt = 0;

    static u8 C2_cnt = 0; //消抖计数 =20上电快速反应
    if (Port_state.C2 != new_C2)
    {
        if (++C2_cnt > 2)
        {
            C2_cnt = 0;
            Port_state.C2 = new_C2;

            if (Port_state.C2)
            { // C2插入
                // if (!Check_Curr.C2_breaking)
                //     Check_Curr.C2_new_insert = true;

                if (!Port_state.A2)
                { //如果A2没插入
                    if (Port_state.C1)
                    { // C1已插入,是否大于16V
                        Switch_Device = SW_IC_1;
                        sw3516_set_ADC_Source(adc_vout); //读输出电压
                        u32 AD_V = sw3516_Read_AD_Value_H();
                        AD_V = AD_V * 6 * 16;
                        if (AD_V > 16000)
                            Work_mod = State_C1_45W__C2_20W;
                        else
                            Work_mod = State_C1_30W__C2_30W;
                    }
                    else
                        Work_mod = SINGLE_C2;
                }
            }
            else
            { // C2拔出
//                if (!Check_Curr.C2_breaking)
//                {
                    if (!Port_state.A2)
                    {
                        if (Port_state.C1)
                            Work_mod = SINGLE_C1;
                        else
                            Work_mod = normal;
                    }
//                }
            }
        }
    }
    else
        C2_cnt = 0;

    static u8 A2_cnt = 0; //消抖计数 =20上电快速反应
    if (Port_state.A2 != new_A2)
    {
        if (++A2_cnt > 2)
        {
            A2_cnt = 0;
            Port_state.A2 = new_A2;

            if (Port_state.A2)
            { // A2插入
                // if (!Check_Curr.C2_breaking)
                //     Check_Curr.C2_new_insert = true; // A2与C2共协议?暂不处理

                if (!Port_state.C2)
                { //如果C2没插入
                    if (Port_state.C1)
                    { // C1已插入,是否大于16V
                        Switch_Device = SW_IC_1;
                        sw3516_set_ADC_Source(adc_vout); //读输出电压
                        u32 AD_V = sw3516_Read_AD_Value_H();
                        AD_V = AD_V * 6 * 16;
                        if (AD_V > 16000)
                            Work_mod = State_C1_45W__C2_20W;
                        else
                            Work_mod = State_C1_30W__C2_30W;
                    }
                    else
                        Work_mod = SINGLE_A2;
                }
            }
            else
            { // A2拔出
                if (!Port_state.C2)
                {
                    if (Port_state.C1)
                        Work_mod = SINGLE_C1;
                    else
                        Work_mod = normal;
                }
            }
        }
    }
    else
        A2_cnt = 0;

    // static u8 cnt = 0; //消抖计数 =20上电快速反应
    // //组合状态消抖
    // if (Port_state.C1 && Port_state.A2)
    // {
    //     if (Work_mod != State_C1A2)
    //     {
    //         if (++cnt >= 20) // 1 > 0.1秒
    //         {
    //             cnt = 0;
    //             Work_mod = State_C1A2;
    //         }
    //     }
    //     else
    //         cnt = 0;
    // }
    // else if (new_C1 && new_C2)
    // {
    //     if (Work_mod != State_C1C2)
    //     {
    //         if (++cnt >= 20) // 1 > 0.1秒
    //         {
    //             cnt = 0;
    //             Work_mod = State_C1C2;
    //         }
    //     }
    //     else
    //         cnt = 0;
    // }
    // else
    // {
    //     if (Work_mod != State_SINGLE)
    //     {
    //         if (++cnt >= 45) // 1 > 0.1秒
    //         {
    //             cnt = 0;
    //             Work_mod = State_SINGLE;
    //         }
    //     }
    //     else
    //         cnt = 0;
    // }
}

/*
  @brief    切换功率设置
  @param
  @return
*/
void change_Power()
{

    static State_enum Work_mod_BK = normal;
    if (Work_mod_BK != Work_mod)
        Work_mod_BK = Work_mod;
    else
        return;

    switch (Work_mod)
    {
    case SINGLE_C1:
        SW3516_Setting.IC1.u0xB7.w = 0B01111101;
        SW3516_Setting.IC1.u0xB4.Fixed_20V_current = 65000 / 20 / 50;
        SW3516_Setting.IC1.u0xA6.Power_Watt = OTHER_THAN_PD_60W;
        SW3516_Setting.IC1.u0xBA.EXCEPT_PD_MAX_Vol = EXCEPT_PD_MAX_20V;
        SW3516_Setting.IC1.u0xB9.SCP_EN = true;
        SW3516_Setting.IC1.u0xBD.Both_On_Current_Limiting = BOTH_CUR_LIM_2p2A;
        SW3516_Setting.IC1.u0xB5.PPS0_current = 5000 / 50;

        SW3516_Setting.IC2.u0xB7.w = 0B01001101;
        SW3516_Setting.IC2.u0xB4.Fixed_20V_current = 20000 / 20 / 50;
        SW3516_Setting.IC2.u0xA6.Power_Watt = OTHER_THAN_PD_18W;
        SW3516_Setting.IC2.u0xBA.EXCEPT_PD_MAX_Vol = EXCEPT_PD_MAX_12V;
        SW3516_Setting.IC2.u0xB9.SCP_EN = true;
        SW3516_Setting.IC2.u0xBD.Both_On_Current_Limiting = BOTH_CUR_LIM_2p2A;
        SW3516_Setting.IC2.u0xB5.PPS0_current = 2250 / 50;
        break;

    case SINGLE_C2:
    case SINGLE_A2:
        SW3516_Setting.IC1.u0xB7.w = 0B01001101;
        SW3516_Setting.IC1.u0xB4.Fixed_20V_current = 20000 / 20 / 50;
        SW3516_Setting.IC1.u0xA6.Power_Watt = OTHER_THAN_PD_18W;
        SW3516_Setting.IC1.u0xBA.EXCEPT_PD_MAX_Vol = EXCEPT_PD_MAX_20V;
        SW3516_Setting.IC1.u0xB9.SCP_EN = true;
        SW3516_Setting.IC1.u0xBD.Both_On_Current_Limiting = BOTH_CUR_LIM_2p2A;
        SW3516_Setting.IC1.u0xB5.PPS0_current = 2250 / 50;

        SW3516_Setting.IC2.u0xB7.w = 0B01111101;
        SW3516_Setting.IC2.u0xB4.Fixed_20V_current = 65000 / 20 / 50;
        SW3516_Setting.IC2.u0xA6.Power_Watt = OTHER_THAN_PD_60W;
        SW3516_Setting.IC2.u0xBA.EXCEPT_PD_MAX_Vol = EXCEPT_PD_MAX_12V;
        SW3516_Setting.IC2.u0xB9.SCP_EN = true;
        SW3516_Setting.IC2.u0xBD.Both_On_Current_Limiting = BOTH_CUR_LIM_2p2A;
        SW3516_Setting.IC2.u0xB5.PPS0_current = 5000 / 50;
        break;

    case State_C1_45W__C2_20W:
        // Check_Curr.C1_not_break_cc = true;
        // Check_Curr.C1_check_Curr_2S = true;
        // Check_Curr.C1_cnt = 0;
        SW3516_Setting.IC1.u0xB7.w = 0B01111101;
        SW3516_Setting.IC1.u0xB4.Fixed_20V_current = 45000 / 20 / 50;
        SW3516_Setting.IC1.u0xA6.Power_Watt = OTHER_THAN_PD_45W;
        SW3516_Setting.IC1.u0xBA.EXCEPT_PD_MAX_Vol = EXCEPT_PD_MAX_20V;
        SW3516_Setting.IC1.u0xB9.SCP_EN = true;
        SW3516_Setting.IC1.u0xBD.Both_On_Current_Limiting = BOTH_CUR_LIM_2p2A;
        SW3516_Setting.IC1.u0xB5.PPS0_current = 4000 / 50;

        SW3516_Setting.IC2.u0xB7.w = 0B01001101;
        SW3516_Setting.IC2.u0xB4.Fixed_20V_current = 20000 / 20 / 50;
        SW3516_Setting.IC2.u0xA6.Power_Watt = OTHER_THAN_PD_18W;
        SW3516_Setting.IC2.u0xBA.EXCEPT_PD_MAX_Vol = EXCEPT_PD_MAX_12V;
        SW3516_Setting.IC2.u0xB9.SCP_EN = true;
        SW3516_Setting.IC2.u0xBD.Both_On_Current_Limiting = BOTH_CUR_LIM_2p2A;
        SW3516_Setting.IC2.u0xB5.PPS0_current = 2250 / 50;
        break;

    case State_C1_20W__C2_45W:
        // Check_Curr.C2_not_break_cc = true;
        // Check_Curr.C2_check_Curr_2S = true;
        // Check_Curr.C2_cnt = 0;
        SW3516_Setting.IC1.u0xB7.w = 0B01001101;
        SW3516_Setting.IC1.u0xA6.Power_Watt = OTHER_THAN_PD_18W;
        SW3516_Setting.IC1.u0xB4.Fixed_20V_current = 20000 / 20 / 50;
        SW3516_Setting.IC1.u0xB9.SCP_EN = true;
        SW3516_Setting.IC1.u0xBA.EXCEPT_PD_MAX_Vol = EXCEPT_PD_MAX_20V;
        SW3516_Setting.IC1.u0xBD.Both_On_Current_Limiting = BOTH_CUR_LIM_2p2A;
        SW3516_Setting.IC1.u0xB5.PPS0_current = 2250 / 50;

        SW3516_Setting.IC2.u0xB7.w = 0B01111101;
        SW3516_Setting.IC2.u0xA6.Power_Watt = OTHER_THAN_PD_45W;
        SW3516_Setting.IC2.u0xB4.Fixed_20V_current = 45000 / 20 / 50;
        SW3516_Setting.IC2.u0xB9.SCP_EN = true;
        SW3516_Setting.IC2.u0xBA.EXCEPT_PD_MAX_Vol = EXCEPT_PD_MAX_12V;
        SW3516_Setting.IC2.u0xBD.Both_On_Current_Limiting = BOTH_CUR_LIM_2p2A;
        SW3516_Setting.IC2.u0xB5.PPS0_current = 4000 / 50;

        break;

    case State_C1_30W__C2_30W:
        SW3516_Setting.IC1.u0xB7.w = 0B01111101;
        SW3516_Setting.IC1.u0xA6.Power_Watt = OTHER_THAN_PD_24W;
        SW3516_Setting.IC1.u0xB4.Fixed_20V_current = 30000 / 20 / 50;
        SW3516_Setting.IC1.u0xB9.SCP_EN = true;
        SW3516_Setting.IC1.u0xBA.EXCEPT_PD_MAX_Vol = EXCEPT_PD_MAX_20V;
        SW3516_Setting.IC1.u0xBD.Both_On_Current_Limiting = BOTH_CUR_LIM_2p2A;
        SW3516_Setting.IC1.u0xB5.PPS0_current = 3000 / 50;

        SW3516_Setting.IC2.u0xB7.w = 0B01111101;
        SW3516_Setting.IC2.u0xA6.Power_Watt = OTHER_THAN_PD_24W;
        SW3516_Setting.IC2.u0xB4.Fixed_20V_current = 30000 / 20 / 50;
        SW3516_Setting.IC2.u0xB9.SCP_EN = true;
        SW3516_Setting.IC2.u0xBA.EXCEPT_PD_MAX_Vol = EXCEPT_PD_MAX_12V;
        SW3516_Setting.IC2.u0xBD.Both_On_Current_Limiting = BOTH_CUR_LIM_2p2A;
        SW3516_Setting.IC2.u0xB5.PPS0_current = 3000 / 50;

        break;

    case normal:
    default:
        init_setting_reg();
        break;
    }

    //功率改变,2秒后检测电流,除了新插入的
    static u32 C1_POWER_BK = 0;
    static u32 C2_POWER_BK = 0;

    if (C1_POWER_BK != SW3516_Setting.IC1.u0xB4.Fixed_20V_current)
    {
        C1_POWER_BK = SW3516_Setting.IC1.u0xB4.Fixed_20V_current;
        //if (!power_on_first)
            Check_Curr.C1_check_Curr_2S = true;
             Check_Curr.C1_cnt=0;
    }

    if (C2_POWER_BK != SW3516_Setting.IC2.u0xB4.Fixed_20V_current)
    {
        C2_POWER_BK = SW3516_Setting.IC2.u0xB4.Fixed_20V_current;
        //if (!power_on_first)
            Check_Curr.C2_check_Curr_2S = true;
            Check_Curr.C2_cnt=0;
    }
}

/*
  @brief    检查或写入设定值
  @param
  @return
*/
void cycle_check()
{
    Switch_Device = SW_IC_1;
    sw3516_Except_PD_W(false);
    sw3516_Set_Mode(false);
    // if (Check_Curr.C1_new_insert)
    // {
    //     if (sw3516_Change_PD_2(false))
    //         HAL_Delay(800); //并延时 800ms(600ms 及以上)；
    // }
    // else
    sw3516_Change_PD_3(false);

    SW3516H_Set_PPS(false);
    sw3516_Set_DMDP(false);
    sw3516_Except_PD_V(false);
    sw3516_OpenPDO_20V(false);
    //sw3516_Enable__SCP(false);
    //sw3516_Both_CUR_LIM(false);
    //sw3516_set_PortC_Empty_Check(false);

    Switch_Device = SW_IC_2;
    sw3516_Except_PD_W(false);
    sw3516_Set_Mode(false);
    // if (Check_Curr.C2_new_insert)
    // {
    //     if (sw3516_Change_PD_2(false))
    //         HAL_Delay(800); //并延时 800ms(600ms 及以上)；
    // }
    // else
    sw3516_Change_PD_3(false);

    SW3516H_Set_PPS(false);
    sw3516_Set_DMDP(false);
    sw3516_Except_PD_V(false);
    sw3516_OpenPDO_20V(false);
    //sw3516_Enable__SCP(false);
    //sw3516_Both_CUR_LIM(false);
    //sw3516_set_PortC_Empty_Check(false);
}
