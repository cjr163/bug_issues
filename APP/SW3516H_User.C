

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
Port_State_st Port_state = {0}; //各口插入状态

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

Check_Curr_st Check_Curr = {false, false};

/*
  @brief    更改PDO功率
  @param    force_edit  是否不比较直写
  @return
*/
static void SW3516H_Change_PDO(bool force_edit)
{
    if (SW3516H_Set_20V_PDO_Curr(force_edit))
    {
        SW3516H_src_change(); //直接广播PDO 3秒后检查无电流再断CC

        // sw3516_OFF_CC();     //更改PD功率方式二:立即断CC重启

        // SW3516_HardReset();  //更改PD功率方式一:发送复位指令
    }
}
/*
  @brief    初始设定值
  @param
  @return
*/
static void init_setting_reg()
{
    //单C1=65W；非PD功率60W，打开20V、SCP。
    // SW3516_Setting.IC1.u0x76.w = 0b00000011; // Prohibit_CC Prohibit_BC
    SW3516_Setting.IC1.u0xA6.Power_Watt = OTHER_THAN_PD_60W;
    SW3516_Setting.IC1.u0xA6.budin = 0; //补丁
    SW3516_Setting.IC1.u0xAB.port_cfg = MODE_SINGLE_C;

    SW3516_Setting.IC1.u0xB4.Fixed_20V_current = 65000 / 20 / 50; // 65W/20V/50unit

    SW3516_Setting.IC1.u0xB5.PPS0_current = 3000 / 50; //初始5A
    SW3516_Setting.IC1.u0xB5.PPS0_EN = 0;
    SW3516_Setting.IC1.u0xB6.PPS1_current = 3000 / 50; //初始5A
    SW3516_Setting.IC1.u0xB6.PPS1_EN = 0;

    SW3516_Setting.IC1.u0xB7.w = 0B01111101;   // PD_20V_PDO
    SW3516_Setting.IC1.u0xB8.PPS_Auto_DIS = 1; //补丁
    SW3516_Setting.IC1.u0xB9.SCP_EN = true;
    SW3516_Setting.IC1.u0xBA.EXCEPT_PD_MAX_Vol = EXCEPT_PD_MAX_20V;

    SW3516_Setting.IC1.u0xBC.PortC_Empty_Check = 0;
    SW3516_Setting.IC1.u0xBD.Both_On_Current_Limiting = BOTH_CUR_LIM_2p2A;
    SW3516_Setting.IC1.u0xBD.DPDM_connfig = true;

    SW3516_Setting.IC1.u0xBE.PPS0_VOL = 3;
    SW3516_Setting.IC1.u0xBE.PPS0_POWER_restrict = 0;
    SW3516_Setting.IC1.u0xBE.PPS0_H_VOL_EN = 0;
    SW3516_Setting.IC1.u0xBE.PPS1_VOL = 3;
    SW3516_Setting.IC1.u0xBE.PPS1_POWER_restrict = 0;
    SW3516_Setting.IC1.u0xBE.PPS1_H_VOL_EN = 0;

    SW3516_Setting.Enable_IC1_Write = false;

    // C2=65W， A2=24W（关闭20V，打开SCP），双口限流2.2A。

    SW3516_Setting.IC2.u0xA6.Power_Watt = OTHER_THAN_PD_18W;
    SW3516_Setting.IC2.u0xA6.budin = 0; //补丁
    SW3516_Setting.IC2.u0xAB.port_cfg = MODE_COMBINE_AC;

    SW3516_Setting.IC2.u0xB4.Fixed_20V_current = 65000 / 20 / 50; // 65W/20V/50unit

    SW3516_Setting.IC2.u0xB5.PPS0_current = 3000 / 50; //初始5A
    SW3516_Setting.IC2.u0xB5.PPS0_EN = 0;
    SW3516_Setting.IC2.u0xB6.PPS1_current = 3000 / 50; //初始5A
    SW3516_Setting.IC2.u0xB6.PPS1_EN = 0;
    SW3516_Setting.IC2.u0xB7.w = 0B11111101;   // PD_20V_PDO
    SW3516_Setting.IC2.u0xB8.PPS_Auto_DIS = 1; //补丁
    SW3516_Setting.IC2.u0xB9.SCP_EN = true;
    SW3516_Setting.IC2.u0xBA.EXCEPT_PD_MAX_Vol = EXCEPT_PD_MAX_12V;

    SW3516_Setting.IC2.u0xBC.PortC_Empty_Check = 0;
    SW3516_Setting.IC2.u0xBD.Both_On_Current_Limiting = BOTH_CUR_LIM_2p2A;
    SW3516_Setting.IC2.u0xBD.DPDM_connfig = true;

    SW3516_Setting.IC2.u0xBE.PPS0_VOL = 3;
    SW3516_Setting.IC2.u0xBE.PPS0_POWER_restrict = 0;
    SW3516_Setting.IC2.u0xBE.PPS0_H_VOL_EN = 0;
    SW3516_Setting.IC2.u0xBE.PPS1_VOL = 3;
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
    SW3516H_Except_PD_W(true);
    SW3516H_Set_Mode(true);
    SW3516H_Change_PDO(true);

    Switch_Device = SW_IC_2;
    SW3516H_Except_PD_W(true);
    SW3516H_Set_Mode(true);
    SW3516H_Change_PDO(true);

    Switch_Device = SW_IC_1;
    SW3516H_Set_PPS(true);
    SW3516H_Set_DMDP(true);
    SW3516H_Except_PD_V(true);
    SW3516H_OpenPDO_20V(true);
    SW3516H_Enable__SCP(true);
    SW3516H_Both_CUR_LIM(true);
    SW3516H_set_PortC_Empty_Check(true);

    Switch_Device = SW_IC_2;
    SW3516H_Set_PPS(true);
    SW3516H_Set_DMDP(true);
    SW3516H_Except_PD_V(true);
    SW3516H_OpenPDO_20V(true);
    SW3516H_Enable__SCP(true);
    SW3516H_Both_CUR_LIM(true);
    SW3516H_set_PortC_Empty_Check(true);
}

/*
  @brief    检测各口插入状态
  @tparam   loop    100ms-300ms
  @param
  @return
*/
void check_Port()
{
    Switch_Device = SW_IC_2;

    bool new_A2 = false; // A2口插入状态
    if (!Port_state.A2)
    {
        // A口插入根据电流判断
        SW3516H_set_ADC_Source(adc_vout); //读输出电压
        u8 AD_V = SW3516H_Read_AD_Value_H();

        SW3516H_set_ADC_Source(adc_iout2);
        u16 AD_I_W = SW3516H_Read_AD_Value(); //读输出电流

        u16 adv_const;
        if (AD_I_W >> 8 > 0)
        {
            AD_I_W >>= 4;
            adv_const = (u16)(2500000 / (6 * 16) / (2.5 * 16)); // 2.5W = 651
        }
        else
            adv_const = (u16)(2500000 / (6 * 16) / 2.5); // 2.5W = 10416

        AD_I_W *= AD_V;
        if (AD_I_W >= adv_const) // A口电流大于2.5W判断插入
            new_A2 = true;
        else
            new_A2 = false;
    }
    else
        new_A2 = SW3516H_Check_Port_A_1(); // 拔出用状态位判断

    bool new_C2 = SW3516H_Check_Port_C_1(); // C2口读状态位

    Switch_Device = SW_IC_1;
    // bool new_C1 = sw3516_Check_Port().PortC_state; // C1口读状态位 有BUG,经常读不到
    bool new_C1 = SW3516H_Check_Port_C_2(); // C1口读状态位

    static u8 C1_cnt = 0; //消抖计数 =20上电快速反应
    if (Port_state.C1 != new_C1)
    {
        if (++C1_cnt > 2)
        {
            C1_cnt = 0;
            Port_state.C1 = new_C1;

            if (Port_state.C1)
            { // C1插入
                if (Port_state.C2 || Port_state.A2)
                { // C2已插入,是否大于16V
                    Switch_Device = SW_IC_2;
                    SW3516H_set_ADC_Source(adc_vout); //读输出电压
                    u32 AD_V = SW3516H_Read_AD_Value_H();
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
                if (Port_state.C2 || Port_state.A2)
                    Work_mod = SINGLE_C2;
                else
                    Work_mod = normal;
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
                if (!Port_state.A2)
                { //如果A2没插入
                    if (Port_state.C1)
                    { // C1已插入,是否大于16V
                        Switch_Device = SW_IC_1;
                        SW3516H_set_ADC_Source(adc_vout); //读输出电压
                        u32 AD_V = SW3516H_Read_AD_Value_H();
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
                if (!Port_state.A2)
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
                if (!Port_state.C2)
                { //如果C2没插入
                    if (Port_state.C1)
                    { // C1已插入,是否大于16V
                        Switch_Device = SW_IC_1;
                        SW3516H_set_ADC_Source(adc_vout); //读输出电压
                        u32 AD_V = SW3516H_Read_AD_Value_H();
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
}

/*
  @brief    切换功率设置
  @tparam   loop    100ms-300ms
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
        SW3516_Setting.IC1.u0xB7.PD_20V_PDO = 1;
        SW3516_Setting.IC1.u0xB7.PD_15V_PDO = 1;
        // SW3516_Setting.IC1.u0xB7.w = 0B00111101;
        SW3516_Setting.IC1.u0xB4.Fixed_20V_current = 65000 / 20 / 50;
        SW3516_Setting.IC1.u0xA6.Power_Watt = OTHER_THAN_PD_60W;
        SW3516_Setting.IC1.u0xBA.EXCEPT_PD_MAX_Vol = EXCEPT_PD_MAX_20V;
        SW3516_Setting.IC1.u0xB9.SCP_EN = true;
        SW3516_Setting.IC1.u0xBD.Both_On_Current_Limiting = BOTH_CUR_LIM_2p2A;
        SW3516_Setting.IC1.u0xB5.PPS0_current = 5000 / 50; ////5A

        SW3516_Setting.IC2.u0xB7.PD_20V_PDO = 0;
        SW3516_Setting.IC2.u0xB7.PD_15V_PDO = 0;
        // SW3516_Setting.IC2.u0xB7.w = 0B00001101;
        SW3516_Setting.IC2.u0xB4.Fixed_20V_current = 20000 / 20 / 50;
        SW3516_Setting.IC2.u0xA6.Power_Watt = OTHER_THAN_PD_18W;
        SW3516_Setting.IC2.u0xBA.EXCEPT_PD_MAX_Vol = EXCEPT_PD_MAX_12V;
        SW3516_Setting.IC2.u0xB9.SCP_EN = true;
        SW3516_Setting.IC2.u0xBD.Both_On_Current_Limiting = BOTH_CUR_LIM_2p2A;
        SW3516_Setting.IC2.u0xB5.PPS0_current = 2250 / 50;
        break;

    case SINGLE_C2:
    case SINGLE_A2:

        SW3516_Setting.IC1.u0xB7.PD_20V_PDO = 0;
        SW3516_Setting.IC1.u0xB7.PD_15V_PDO = 0;
        // SW3516_Setting.IC1.u0xB7.w = 0B00001101;
        SW3516_Setting.IC1.u0xB4.Fixed_20V_current = 20000 / 20 / 50;
        SW3516_Setting.IC1.u0xA6.Power_Watt = OTHER_THAN_PD_18W;
        SW3516_Setting.IC1.u0xBA.EXCEPT_PD_MAX_Vol = EXCEPT_PD_MAX_20V;
        SW3516_Setting.IC1.u0xB9.SCP_EN = true;
        SW3516_Setting.IC1.u0xBD.Both_On_Current_Limiting = BOTH_CUR_LIM_2p2A;
        SW3516_Setting.IC1.u0xB5.PPS0_current = 2250 / 50;

        SW3516_Setting.IC2.u0xB7.PD_20V_PDO = 1;
        SW3516_Setting.IC2.u0xB7.PD_15V_PDO = 1;
        // SW3516_Setting.IC2.u0xB7.w = 0B00111101;
        SW3516_Setting.IC2.u0xB4.Fixed_20V_current = 65000 / 20 / 50;
        SW3516_Setting.IC2.u0xA6.Power_Watt = OTHER_THAN_PD_60W;
        SW3516_Setting.IC2.u0xBA.EXCEPT_PD_MAX_Vol = EXCEPT_PD_MAX_12V;
        SW3516_Setting.IC2.u0xB9.SCP_EN = true;
        SW3516_Setting.IC2.u0xBD.Both_On_Current_Limiting = BOTH_CUR_LIM_2p2A;
        SW3516_Setting.IC2.u0xB5.PPS0_current = 5000 / 50; /////5A
        break;

    case State_C1_45W__C2_20W:
        // Check_Curr.C1_not_break_cc = true;
        // Check_Curr.C1_check_Curr_2S = true;
        // Check_Curr.C1_cnt = 0;

        SW3516_Setting.IC1.u0xB7.PD_20V_PDO = 1;
        SW3516_Setting.IC1.u0xB7.PD_15V_PDO = 1;
        // SW3516_Setting.IC1.u0xB7.w = 0B00111101;
        SW3516_Setting.IC1.u0xB4.Fixed_20V_current = 45000 / 20 / 50;
        SW3516_Setting.IC1.u0xA6.Power_Watt = OTHER_THAN_PD_45W;
        SW3516_Setting.IC1.u0xBA.EXCEPT_PD_MAX_Vol = EXCEPT_PD_MAX_20V;
        SW3516_Setting.IC1.u0xB9.SCP_EN = true;
        SW3516_Setting.IC1.u0xBD.Both_On_Current_Limiting = BOTH_CUR_LIM_2p2A;
        SW3516_Setting.IC1.u0xB5.PPS0_current = 4000 / 50; /// 4A

        SW3516_Setting.IC2.u0xB7.PD_20V_PDO = 0;
        SW3516_Setting.IC2.u0xB7.PD_15V_PDO = 0;
        // SW3516_Setting.IC2.u0xB7.w = 0B00001101;
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

        SW3516_Setting.IC1.u0xB7.PD_20V_PDO = 0;
        SW3516_Setting.IC1.u0xB7.PD_15V_PDO = 0;
        // SW3516_Setting.IC1.u0xB7.w = 0B00001101;
        SW3516_Setting.IC1.u0xA6.Power_Watt = OTHER_THAN_PD_18W;
        SW3516_Setting.IC1.u0xB4.Fixed_20V_current = 20000 / 20 / 50;
        SW3516_Setting.IC1.u0xB9.SCP_EN = true;
        SW3516_Setting.IC1.u0xBA.EXCEPT_PD_MAX_Vol = EXCEPT_PD_MAX_20V;
        SW3516_Setting.IC1.u0xBD.Both_On_Current_Limiting = BOTH_CUR_LIM_2p2A;
        SW3516_Setting.IC1.u0xB5.PPS0_current = 2250 / 50;

        SW3516_Setting.IC2.u0xB7.PD_20V_PDO = 1;
        SW3516_Setting.IC2.u0xB7.PD_15V_PDO = 1;
        // SW3516_Setting.IC2.u0xB7.w = 0B00111101;
        SW3516_Setting.IC2.u0xA6.Power_Watt = OTHER_THAN_PD_45W;
        SW3516_Setting.IC2.u0xB4.Fixed_20V_current = 45000 / 20 / 50;
        SW3516_Setting.IC2.u0xB9.SCP_EN = true;
        SW3516_Setting.IC2.u0xBA.EXCEPT_PD_MAX_Vol = EXCEPT_PD_MAX_12V;
        SW3516_Setting.IC2.u0xBD.Both_On_Current_Limiting = BOTH_CUR_LIM_2p2A;
        SW3516_Setting.IC2.u0xB5.PPS0_current = 4000 / 50; /////4A

        break;

    case State_C1_30W__C2_30W:

        SW3516_Setting.IC1.u0xB7.PD_20V_PDO = 1;
        SW3516_Setting.IC1.u0xB7.PD_15V_PDO = 1;
        // SW3516_Setting.IC1.u0xB7.w = 0B00111101;
        SW3516_Setting.IC1.u0xA6.Power_Watt = OTHER_THAN_PD_24W;
        SW3516_Setting.IC1.u0xB4.Fixed_20V_current = 30000 / 20 / 50;
        SW3516_Setting.IC1.u0xB9.SCP_EN = true;
        SW3516_Setting.IC1.u0xBA.EXCEPT_PD_MAX_Vol = EXCEPT_PD_MAX_20V;
        SW3516_Setting.IC1.u0xBD.Both_On_Current_Limiting = BOTH_CUR_LIM_2p2A;
        SW3516_Setting.IC1.u0xB5.PPS0_current = 3000 / 50;

        SW3516_Setting.IC2.u0xB7.PD_20V_PDO = 1;
        SW3516_Setting.IC2.u0xB7.PD_15V_PDO = 1;
        // SW3516_Setting.IC2.u0xB7.w = 0B00111101;
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
        // if (!power_on_first)
        Check_Curr.C1_check_Curr_2S = true;
        // Check_Curr.C1_cnt = 0;
    }

    if (C2_POWER_BK != SW3516_Setting.IC2.u0xB4.Fixed_20V_current)
    {
        C2_POWER_BK = SW3516_Setting.IC2.u0xB4.Fixed_20V_current;
        // if (!power_on_first)
        Check_Curr.C2_check_Curr_2S = true;
        // Check_Curr.C2_cnt = 0;
    }
}

/*
  @brief    检查与写入设定值
  @tparam   loop    100ms-300ms
  @param
  @return
*/
void cycle_check()
{
    Switch_Device = SW_IC_1;
    SW3516H_Except_PD_W(false);
    SW3516H_Set_Mode(false);
    SW3516H_Change_PDO(false);

    SW3516H_Set_PPS(false);
    SW3516H_Set_DMDP(false);
    SW3516H_Except_PD_V(false);
    SW3516H_OpenPDO_20V(false);
    SW3516H_Enable__SCP(false);
    SW3516H_Both_CUR_LIM(false);
    SW3516H_set_PortC_Empty_Check(false);

    Switch_Device = SW_IC_2;
    SW3516H_Except_PD_W(false);
    SW3516H_Set_Mode(false);
    SW3516H_Change_PDO(false);

    SW3516H_Set_PPS(false);
    SW3516H_Set_DMDP(false);
    SW3516H_Except_PD_V(false);
    SW3516H_OpenPDO_20V(false);
    SW3516H_Enable__SCP(false);
    SW3516H_Both_CUR_LIM(false);
    SW3516H_set_PortC_Empty_Check(false);
}

/*
  @brief    三秒后查无电流断CC
  @tparam   loop    100ms-300ms
  @param
  @return
*/
void Check_Curr_After_3S()
{
    static u32 DB_C1 = 0;
    static u32 DB_C2 = 0;
    if (Check_Curr.C1_check_Curr_2S)
    {
        if (++DB_C1 > 10) // 300ms*10 秒
        {
            DB_C1 = 0;
            Check_Curr.C1_check_Curr_2S = false;

            Switch_Device = SW_IC_1;
            SW3516H_Enable_I2C_Write();
            SW3516H_set_ADC_Source(adc_iout1);
            u32 AD_I = SW3516H_Read_AD_Value(); //读输出电流
            AD_I *= 5;
            AD_I >>= 1;
            if (AD_I < 100)
                SW3516H_Disconnect_CC();
        }
    }
    else
        DB_C1 = 0;

    if (Check_Curr.C2_check_Curr_2S)
    {
        if (++DB_C2 > 10) // 300ms*10 秒
        {
            DB_C2 = 0;
            Check_Curr.C2_check_Curr_2S = false;

            Switch_Device = SW_IC_2;
            SW3516H_Enable_I2C_Write();
            SW3516H_set_ADC_Source(adc_iout1);
            u32 AD_I = SW3516H_Read_AD_Value(); //读输出电流
            AD_I *= 5;
            AD_I >>= 1;
            if (AD_I < 100)
                SW3516H_Disconnect_CC();
        }
    }
    else
        DB_C2 = 0;
}

/*
  @brief    AC同时插入关闭DMDP重启CC 适应三星快充 (A2读电流判读插拔太慢,这里另用读状态位)
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
    static u32 DB_cnt = 0;

    static u32 last_curr = 0; //上几次周期读到的电流值平均值,保存用于下一次校验
    static u32 adcv_sum = 0;  //低通累加值
    u16 new_adcv;             //读ADC

    // C2存在但A2不存在,检测大于20mA时插入A口,重启CC限流
    Switch_Device = SW_IC_2;

    // SW3516H_r0x07_st System_state0;
    // System_state0.w = CCC_I2C_ReadReg((u32) & (SW3516H->System_state0));///0x07寄存器反应慢,不能做为判断条件

    SW3516H_r0x08_st System_state1;
    System_state1.w = CCC_I2C_ReadReg((u32) & (SW3516H->System_state1));

    switch (Samsung_check)
    {
    case Samsung_none_port:
        //无插入
        if (System_state1.Device_Exists == AC_ONLY_C || System_state1.Device_Exists == AC_ONLY_A)
        {
            if (++DB_cnt >= 2)
            {
                DB_cnt = 0;
                Samsung_check = Samsung_single_port;
            }
        }
        else
            DB_cnt = 0;

        break;
    case Samsung_single_port:
        //单口检测变两口插入
        if (System_state1.Device_Exists == AC_BOTH)
        {
            if (++DB_cnt >= 2)
            {
                DB_cnt = 0;

                SW3516_Setting.IC2.u0xBD.DPDM_connfig = 0; // DP/DM无效，
                SW3516H_Set_DMDP(true);

                if (last_curr > (u32)(20 / 2.5)) //如果大于20mA，DMDP无效并断CC
                {
                    SW3516H_Disconnect_CC(); //重启过程中强制断开CC，；C口广播1.5A，DP/DM无效，
                }
                else
                {
                    //插入一根苹果线,会有十几MA电流,一直重启??
                }

                Samsung_check = Samsung_double_port;
                last_curr = 0;
                adcv_sum = 0;
            }
        }
        else
            DB_cnt = 0;

        if (System_state1.Device_Exists == AC_NONE)
        {
            Samsung_check = Samsung_none_port;
            last_curr = 0;
            adcv_sum = 0;

            DB_cnt = 0;
        }

        SW3516H_set_ADC_Source(adc_iout1);
        new_adcv = SW3516H_Read_AD_Value();

        last_curr = Slide_Average(new_adcv, &adcv_sum, last_curr);

        break;
    case Samsung_double_port:
        //双口检测拔出
        if (System_state1.Device_Exists != AC_BOTH)
        {
            if (++DB_cnt >= 2)
            {
                DB_cnt = 0;

                SW3516_Setting.IC2.u0xBD.DPDM_connfig = 1;
                SW3516H_Set_DMDP(true);
                Samsung_check = Samsung_none_port;
            }
        }
        else
            DB_cnt = 0;

        break;
    default:
        break;
    }
}
