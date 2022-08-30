

#ifndef __SW3516H_User_H
#define __SW3516H_User_H

typedef struct
{
    // SW3516H_r0x76_st u0x76; //连接设置
    SW3516H_r0xA6_st u0xA6; //功率配置
    SW3516H_r0xAB_st u0xAB; //端口配置

    SW3516H_r0xB4_st u0xB4; // PD配置4 （PDO 20V)
    SW3516H_r0xB5_st u0xB5; // PPS0
    SW3516H_r0xB6_st u0xB6; // PPS1
    SW3516H_r0xB7_st u0xB7; // PD配置7 各开关位
    SW3516H_r0xB8_st u0xB8; //不禁止pps
    SW3516H_r0xB9_st u0xB9; //快充配置1 (多个协议)
    SW3516H_r0xBA_st u0xBA; //快充配置2 (AFC和else最高电压)
    SW3516H_r0xBC_st u0xBC; //快充配置3 (空载超载)
    SW3516H_r0xBD_st u0xBD; //限流配置 Current_Limiting
    SW3516H_r0xBE_st u0xBE; // PPS

} Setting_st;
typedef struct
{
    Setting_st IC1;
    Setting_st IC2;
    bool Enable_IC1_Write; //全局写I2C解锁
    bool Enable_IC2_Write; //全局写I2C解锁
} SW3516_Setting_st;

typedef enum
{
    SW_IC_1 = 1, //单C口 PA7 PA6
    SW_IC_2 = 2, // AC口 PC3 PC2
} Device_enum;

extern SW3516_Setting_st SW3516_Setting;
extern Device_enum Switch_Device;

typedef struct
{
    bool C1_check_Curr_2S : 1; // 2秒后查电流
    bool C2_check_Curr_2S : 1; // 2秒后查电流
    // u32 C1_cnt;
    // u32 C2_cnt;
} Check_Curr_st;
extern Check_Curr_st Check_Curr;

void init_sw3516();

void check_Port();
void change_Power();
void cycle_check();

void Check_Curr_After_3S();

void check_Samsung();

#endif
