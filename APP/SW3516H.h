/*================================================================================
  @Author: ChenJiaRan
  @Date: 2022-06-28 15:59:48
  @LastEditTime  2022-08-26 11:12
  @LastEditors  ChenJiaRan
  @Description: 智融SW3516
  @Version: V1.0
注意 :未定义的寄存器或 bit 不能被改写 ,先读后写
0xBC: 反的? 待测
================================================================================*/
#ifndef _SW3516H_H_
#define _SW3516H_H_

//#include "ft61f02x_IIC.h"

#define SW3516H_I2C_ADDR 0x3C                          // SW3516设备ID  前7位
#define SW3516H_WriteAddR (SW3516H_I2C_ADDR << 1)      // 0x78 写地址 8bit
#define SW3516H_ReadAddR ((SW3516H_I2C_ADDR << 1) | 1) // 0x79 读地址 8bit

typedef union
{
    struct
    {
        u8 b4 : 4;
    };
    u8 w;
} SW3516H_r0x3C_st;

//快充协议指示
typedef enum
{
    other = 0,
    QC2_0 = 1,
    QC3_0 = 2,
    FCP = 3,
    SCP = 4,
    PD_FIX = 5,
    PD_PPS = 6,
    PE1_1 = 7,
    PE2_0 = 8,
    SFCP = 0xa,
    AFC = 0xb
} PD_XY_enum;
typedef union
{
    struct
    {
        PD_XY_enum PD_XY : 4; //快充协议指示 @param PD_XY_enum
        u8 PD_ver : 2;        // PD 协议版本 @param 1: PD 2.0  @param 2: PD 3.0  @param other: Reserved
        u8 : 1;               //
        u8 LED : 1;           //快充指示灯的状态 @param 1: 打开
    };
    u8 w;
} SW3516H_r0x06_st;

typedef union
{
    struct
    {
        u8 PortC_state : 1; // C口的开关状态 @param 1: 打开
        u8 PortA_state : 1; // A口的开关状态 @param 1: 打开
        u8 Buck_state : 1;  // Buck的开关状态 @param 1: 打开
    };
    u8 w;
} SW3516H_r0x07_st;

//端口设备存在状态 //特别注意，这里 A 口从没有设备判断为有设备的电流门限为高于 80mA； A 口从有设备到没有设备的电流门限为低于 15mA。
typedef enum
{
    //对于 AA 模式
    AA_NONE = 1,    // 1: 表示 A1 和 A2 没有设备
    AA_ONLY_A1 = 2, // 2: 表示只有 A1 口有设备
    AA_ONLY_A2 = 3, // 3: 表示只有 A2 口有设备
    AA_BOTH = 4,    // 4: 表示 A1 口和 A2 口都有设备

    //对于 AC 模式来说
    AC_NONE = 5,   // 5: 表示 A 口和 C 口没有设备
    AC_ONLY_C = 6, // 6: 表示只有 C 口有设备
    AC_ONLY_A = 7, // 7: 表示只有 A 口有设备
    AC_BOTH = 8,   // 8: 表示 A 口和 C 口都有设备

    //对于单 C 模式
    SINGLE_C_NONE = 0xA, // A: 表示 C 口关闭
    SINGLE_C_EXIST = 0xB // B：表示 C 口打开，即有设备接入
} Device_Exists_enum;
typedef union
{
    struct
    {
        u8 : 4;
        Device_Exists_enum Device_Exists : 4; //端口设备存在状态 @param Device_Exists_enum
    };
    u8 w;
} SW3516H_r0x08_st;

// typedef union
// {
//     struct
//     {
//         u8 : 5;
//         u8 I2C_control_bit : 3; //写byte
//     };
//     u8 w; // I2C 写操作使能 如果要操作寄存器 reg0xB0~BF,需要先执行如下操作:写 0x20;0x40;0x80;
// } I2C_REG_Write_st;

typedef union
{
    struct
    {
        u8 : 1;
        u8 ADC_Vin_bit : 1; //输入Vin的ADC工作使能，只有在使能时，Vin的数据才能读出 @param 1: 使能
        u8 : 4;
        u8 temp_up : 1; // PPS 和 SCP 协议是否上报NTC温度  @param 0: 上报 NTC 温度 ; @param 1: 上报 45°
    };
    u8 w;
} SW3516H_r0x13_st;

typedef union
{
    struct
    {
        u8 CLOSE_BUCK : 1; //强制关闭BUCK操作 @param 0: 无影响 @param 1: 强制关闭 Buck
        u8 OPEN_BUCK : 1;  //强制打开BUCK操作 @param 0: 无影响 @param 1: 强制打开 Buck
    };
    u8 w;
} SW3516H_r0x16_st;

typedef enum
{
    adc_vin = 1,   // 10mV/bit
    adc_vout = 2,  // 6mV/ bit
    adc_iout1 = 3, // C口 2.5mA/ bit
    adc_iout2 = 4, // A口 2.5mA/ bit
    adc_ntc = 6    // 0.5mv/ bit, RNTC= adc_ntc[11:0] *0.5mV / 100uA – 2kohm;
} ADC_SEL_enum;    // ADC 数据选择 12位[11:0]
typedef union
{
    struct
    {
        /*
          @brief    ADC 数据选择
          @tparam   注意 (写此寄存器之后，会将对应的 ADC 数据锁存到 Reg0x3A 和 Reg0x3B, 防止读到的数据高低位不对应)
          @param    ADC_SEL_enum
        */
        ADC_SEL_enum ADC_SEL : 3;
    };
    u8 w;
} SW3516H_r0x3A_st;

typedef union
{
    struct
    {
        u8 PD_Command_bit : 4; // PD 命令 @param 1: hardreset 命令 @param Other: reserved
        u8 : 3;
        u8 PD_Command_en : 1; //发送使能 @param 写1: 芯片将发送 PD_Command_bit 中所定义的 PD 命令。
    };
    u8 w;
} SW3516H_r0x70_st;

typedef union
{
    struct
    {
        u8 : 1;
        u8 DR_SWAP : 1;           // PD DR SWAP 使能 @param 0: 不使能 @param 1: 使能
        u8 VCONN_SWAP : 1;        // PD VCONN SWAP 使能 @param 0: 不使能 @param 1: 使能
        u8 Get_Status : 1;        // PD Get Status 使能 @param 0: 不使能 @param 1: 使能
        u8 Get_source_extend : 1; // PD Get source extend 使能 @param 0: 不使能 @param 1: 使能
    };
    u8 w;
} SW3516H_r0x71_st;

typedef union
{
    struct
    {
        u8 : 7;
        u8 SRC_Change : 1; //未确定,手册无 @param 写1: 不断CC更新PDO (或复位?)
    };
    u8 w;
} SW3516H_r0x73_st;

typedef union
{
    struct
    {
        /*
          @brief    强制CC不驱动
          @param    0: 正常 重新驱动 CC
          @param    1: 强制不驱动CC 使得CC连接断开
        */
        u8 Prohibit_CC : 1;
        /*
          @brief    强制BC1.2使能
          @tparam   注意 (此功能应用在MCU关闭/打开端口电压时，需要同步不使能/使能BC1.2功能)
          @param    0: 正常
          @param    1: 不使能 (DPDM将不被驱动。)
        */
        u8 Prohibit_BC : 1;
        /*
          @brief    单A口或单C口模式时，5V非快充时的限流档位配置使能
          @tparam   注意 配置流程为: 关闭端口快充， 设置此 bit 为 1， 在通过 reg0xBD[5:4]来设置限流。
          @param    0: 不使能 默认做法，根据功率和在线端口数自动设置
          @param    1: 使能
        */
        u8 Single_restrict : 1;
    };
    u8 w;
} SW3516H_r0x76_st;

typedef enum
{
    OTHER_THAN_PD_18W = 0,
    OTHER_THAN_PD_24W = 1,
    OTHER_THAN_PD_45W = 2, //(36 or 45)
    OTHER_THAN_PD_60W = 3
} Power_Watt_enum;
typedef union
{
    struct
    {
        Power_Watt_enum Power_Watt : 2; //功率配置 (非 PD 和低压直充和双口在线以外的功率设置) @param Power_Watt_enum
        u8 : 5;
        u8 budin : 1; //未知补丁,写0
    };
    u8 w;
} SW3516H_r0xA6_st;

typedef union
{
    struct
    {
        u8 : 6;
        u8 QC3_0_EN : 1; // QC3.0使能 @param 1: 使能
    };
    u8 w;
} SW3516H_r0xAA_st;

typedef enum
{
    MODE_SINGLE_A = 0,  //单 A 口
    MODE_DOUBLE_AA = 1, //双 A 口
    MODE_SINGLE_C = 2,  //单 C 口
    MODE_COMBINE_AC = 3 // AC 口
} port_cfg_enum;
typedef union
{
    struct
    {
        u8 : 2;
        port_cfg_enum port_cfg : 2; //芯片端口设置 @param port_cfg_enum
    };
    u8 w;
} SW3516H_r0xAB_st;

typedef union
{
    struct
    {
        u8 : 2;
        u8 samsung1_2 : 1; // 三星1.2V 模式使能 @param 1: 使能
    };
    u8 w;
} SW3516H_r0xAD_st;

typedef union
{
    struct
    {
        /*
          @brief    Fixed 5V PDO 电流
          @tparam   注意 广播大于3A的电流时，需要是emarker线或reg0xB7[1]=0;修改电流后，需重插拔或写src_change命令生效
          @param    50mA/bit
        */
        u8 Fixed_5V_current : 7;
        /*
          @brief    Fixed 5V PDO 电流设置使能
          @param    0: 使能
          @param    1: 不使能!!! 根据最大功率自动配置(20V)
        */
        u8 Fixed_5V_EN : 1;
    };
    u8 w;
} SW3516H_r0xB0_st;

typedef union
{
    struct
    {
        /*
          @brief    Fixed 9V PDO 电流
          @param    50mA/bit
        */
        u8 Fixed_9V_current : 7;
        /*
          @brief    Fixed 9V PDO 电流设置使能
          @param    0: 使能
          @param    1: 不使能!!! 根据最大功率自动配置(20V)
        */
        u8 Fixed_9V_EN : 1;
    };
    u8 w;
} SW3516H_r0xB1_st;

typedef union
{
    struct
    {
        /*
          @brief    Fixed 12V PDO 电流
          @param    50mA/bit
        */
        u8 Fixed_12V_current : 7;
        /*
          @brief    Fixed 12V PDO 电流设置使能
          @param    0: 使能
          @param    1: 不使能!!! 根据最大功率自动配置(20V)
        */
        u8 Fixed_12V_EN : 1;
    };
    u8 w;
} SW3516H_r0xB2_st;

typedef union
{
    struct
    {
        /*
          @brief    Fixed 15V PDO 电流
          @param    50mA/bit
        */
        u8 Fixed_15V_current : 7;
        /*
          @brief    Fixed 15V PDO 电流设置使能
          @param    0: 使能
          @param    1: 不使能!!! 根据最大功率自动配置(20V)
        */
        u8 Fixed_15V_EN : 1;
    };
    u8 w;
} SW3516H_r0xB3_st;

typedef union
{
    struct
    {
        /*
          @brief    Fixed 20V PDO 电流
          @tparam   注意 当Read_Emarker使能(reg0xB7[1]=0)时，电流的变化都需要写src_change命令才会重新广播
          @param    50mA/bit
        */
        u8 Fixed_20V_current : 7;
        u8 : 1; //手册未开放，操作此寄存器需保留此位！！！
    };
    u8 w;
} SW3516H_r0xB4_st;

typedef union
{
    struct
    {
        u8 PPS0_current : 7; // PPS0电流 @param 50mA/bit
        /*
          @brief    PPS0电流设置使能
          @param    0: 使能
          @param    1: 不使能!!! 根据最大功率自动配置(20V)
        */
        u8 PPS0_EN : 1;
    };
    u8 w;
} SW3516H_r0xB5_st;

typedef union
{
    struct
    {
        u8 PPS1_current : 7; // PPS1电流 @param 50mA/bit
        /*
          @brief    PPS1电流设置使能
          @param    0: 使能
          @param    1: 不使能!!! 根据最大功率自动配置(20V)
        */
        u8 PPS1_EN : 1;
    };
    u8 w;
} SW3516H_r0xB6_st;

typedef union
{
    struct
    {
        u8 PD3 : 1;          // PD3.0使能 @param 0: PD2.0 @param 1: PD3.0
        u8 Read_Emarker : 1; // PD读Emarker使能 @param 0: 使能 @param 1: 不使能!!!
        u8 PD_9V_PDO : 1;    // @param 0: 不使能 @param 1: 使能
        u8 PD_12V_PDO : 1;   // @param 0: 不使能 @param 1: 使能
        u8 PD_15V_PDO : 1;   // @param 0: 不使能 @param 1: 使能
        u8 PD_20V_PDO : 1;   // @param 0: 不使能 @param 1: 使能
        /*
          @brief    广播PPS0 (当PD3位设为0=PD2.0时,无法启用PPS...也许有BUG暂不处理)
          @tparam   注意 PPS0电压=3V~reg0xBE[1:0]V  修改此 bit 后，需要重新插拔或写 src_change 命令才会生效
          @param    0: 不使能
          @param    1: 使能
        */
        u8 PPS0_EN : 1;
        /*
          @brief    广播PPS1 (当PD3位设为0=PD2.0时,无法启用PPS...也许有BUG暂不处理)
          @tparam   注意 PPS1电压=3V~reg0xBE[5:4]V ,修改此 bit 后，需要重新插拔或些 src_change 命令才会生效
          @tparam   注意 PD 配置的最大功率大于 60W 时, PPS1 将不会广播
          @tparam   注意 PPS1的最高电压需要大于PPS0的最高电压，否则 PPS1 不会广播
          @param    0: 不使能
          @param    1: 使能
        */
        u8 PPS1_EN : 1;
    };
    u8 w;
} SW3516H_r0xB7_st;

typedef union
{
    struct
    {
        /*
          @brief    PDO 超载电流能力设置
          @tparam   注意 此设置只是影响 PDO 信息中的内容，与实际的 power 无关
          @param    见手册
        */
        u8 PDO_OVER_CURRENT : 2;
        /*
          @brief    PD Discovery Identity 响应
          @tparam   见手册
          @param    0: 不使能, PD2.0 时只回复 GoodCRC, PD3.0 时回复 not support
          @param    1: 使能， 响应命令， VID 由{Reg0xAF， Reg0xBF}决定， XID， PDI等信息均为0。
        */
        u8 PD_Discovery_Identity : 1;
        /*
          @brief    PPS 后出现 hardreset，是否自动禁止 PPS
          @param    0: 禁止 PPS，重新广播 PDO ;
          @param    1: 不禁止 PPS
        */
        u8 PPS_Auto_DIS : 1;
        u8 Need_emarker : 1; // PD 65W~70W 是否需检测到 emarker @param 0: 不检测 Emarker @param 1: 检测 Emarker
        /*
          @brief    PD 5V/2A PDO 广播
          @tparam   注意 在广播5V/3A, 设备请求5V PDO后, 将重新广播5V/2A PDO，兼容三星S8等使用
          @param    0: 使能
          @param    1: 不使能!!!
        */
        u8 PD_5V2A_PDO : 1;
        /*
          @brief    DR SWAP 使能 (角色变换)
          @param    0: 使能，回复accept，正确响应DR_SWAP ;
          @param    1: 不使能, 在 PD2.0 时回复Reject，在PD3.0时回复not support。
        */
        u8 DR_SWAP : 1;
    };
    u8 w;
} SW3516H_r0xB8_st;

typedef union
{
    struct
    {
        u8 PE_EN : 1;    // PE 协议 1: 使能
        u8 : 1;          //
        u8 SCP_EN : 1;   // SCP 协议 1: 使能
        u8 FCP_EN : 1;   // FCP 协议 1: 使能
        u8 QC_EN : 1;    // QC 协议 1: 使能
        u8 PD_EN : 1;    // PD 协议 1: 使能
        u8 PortA_EN : 1; // A口快充 1: 使能
        u8 PortC_EN : 1; // C口快充 1: 使能
    };
    u8 w;
} SW3516H_r0xB9_st;

typedef enum
{
    EXCEPT_PD_MAX_9V = 0,
    EXCEPT_PD_MAX_12V = 1,
    EXCEPT_PD_MAX_20V = 3
} EXCEPT_PD_MAX_Vol_enum;
typedef union
{
    struct
    {
        u8 : 2;
        EXCEPT_PD_MAX_Vol_enum EXCEPT_PD_MAX_Vol : 2; //最高输出电压(除 PD 以外的协议) @param EXCEPT_PD_MAX_Vol_enum
        u8 : 2;
        u8 AFC_EN : 1; // AFC 协议 1: 使能
    };
    u8 w;
} SW3516H_r0xBA_st;

typedef union
{
    struct
    {
        u8 : 3;
        u8 PortC_Empty_Check : 1; // C 口空载检测  @param 0: 使能  @param 1: 不使能!!!
        u8 : 2;
        /*
          @brief    Peak 超载功能
          @tparam   注意 Power超载使能，只是针对模拟有效
          @param    0: 使能
          @param    1: 不使能!!!
        */
        u8 Peak_Over_EN : 1;
        u8 FC_Ban_Check : 1; // 快充是否禁止空载检测 @param 0: 不禁止 @param 1: 禁止
    };
    u8 w;
} SW3516H_r0xBC_st;

typedef enum
{
    BOTH_CUR_LIM_2p6A = 0, // 2.6A
    BOTH_CUR_LIM_2p2A = 1, // 2.2A
    BOTH_CUR_LIM_1p7A = 2, // 1.7A
    BOTH_CUR_LIM_3p2A = 3  // 3.2A
} Both_Cur_Lim_enum;
typedef union
{
    struct
    {
        u8 : 4;
        Both_Cur_Lim_enum Both_On_Current_Limiting : 2; //双口同时打开时的每个端口限流值 @param Both_Cur_Lim_enum
        /*
          @brief    单口转双口时, DPDM 是否有效
          @tparam   注意 此 bit 设置为 0 时， TypeC Rp 将设置为 1.5A
          @param    0: 无效，即单口转双口时， DPDM 不支持苹果 2.7A 和三星 2A 模式
          @param    1: 有效，即单口转双口时， DPDM 支持苹果 2.7A 和三星 2A 模式
        */
        u8 DPDM_connfig : 1;
    };
    u8 w;
} SW3516H_r0xBD_st;

typedef union
{
    struct
    {
        u8 PPS0_VOL : 2;            // PPS0电压设置  @param 0: 5.9V  @param 1: 11V  @param 2: 16V  @param 3: 21V
        u8 PPS0_POWER_restrict : 1; // PPS0功率限制使能  @param 0: 使能 @param 1: 不使能!!!
        u8 PPS0_H_VOL_EN : 1;       // PPS0的最高点压设置使能  @param 0: 使能 @param 1: 不使能!!!
        u8 PPS1_VOL : 2;            // PPS1电压设置  @param 0: 5.9V  @param 1: 11V  @param 2: 16V  @param 3: 21V
        u8 PPS1_POWER_restrict : 1; // PPS1功率限制使能  @param 0: 使能 @param 1: 不使能!!!
        u8 PPS1_H_VOL_EN : 1;       // PPS1的最高点压设置使能  @param 0: 使能 @param 1: 不使能!!!
    };
    u8 w;
} SW3516H_r0xBE_st;

typedef u8 ______________u8; //仅为对齐变量名方便阅读
typedef union
{
    struct
    {
        __R_ ______________u8 Chip_ver;      //@0x01  芯片版本
        __R_ u8 _resv_02[4];                 //
        __R_ SW3516H_r0x06_st PD_state;      //@0x06  快充指示
        __R_ SW3516H_r0x07_st System_state0; //@0x07  系统状态0
        __R_ SW3516H_r0x08_st System_state1; //@0x08  系统状态1
        __R_ u8 _resv_09[9];                 //
        __RW ______________u8 I2C_REG_Write; //@0x12  I2C使能写操作使能 要操作寄存器reg0xB0~BF,顺序写入0x20;0x40;0x80解锁寄存器；完成后写其它值锁定
        __RW SW3516H_r0x13_st ADC_Vin_ctr;   //@0x13  ADC Vin 使能
        __R_ u8 _resv_14;
        __RW ______________u8 Power_REG_Write; //@0x15  PWR寄存器写使能 如果要操作寄存器reg0x16,顺序写入0x20;0x40;0x80解锁寄存器；
        __RW SW3516H_r0x16_st PWR_force;       //@0x16  PWR强制操作 (需先解锁)
        __R_ u8 _resv_17[25];
        __R_ ______________u8 ADC_Vin;    //@0x30  vin电压的高8bit,160mv/bit;(若取12bit 时分辨率为 10mv/bit) (需要Vin的ADC使能)
        __R_ ______________u8 ADC_Vout;   //@0x31  输出电压的高 8bit, 96mv/bit; (若取12bit 时分辨率为 6mv/bit)
        __R_ u8 _resv_32;                 //
        __R_ ______________u8 ADC_Iout1;  //@0x33  C口输出电流的高 8bit, 40mA/bit; (若取12bit 时分辨率为 2.5mA/bit)
        __R_ ______________u8 ADC_Iout2;  //@0x34  A口输出电流的高 8bit, 40mA/bit; (若取12bit 时分辨率为 2.5mA/bit)
        __R_ u8 _resv_35[2];              //
        __R_ ______________u8 ADC_NTC;    //@0x37  NTC 电阻上电压的高8bit, 8mV/bit; (若取12bit 时分辨率为 0.5mv/bit) NTC电阻的计算公式:RNTC=Reg0x37 *8mV /80uA;
        __R_ u8 _resv_38[2];              //
        __RW SW3516H_r0x3A_st ADC_ctr;    //@0x3A  ADC 配置 (8bit或12bit 每bit分辨率不同)
        __R_ ______________u8 ADC_Val_H;  //@0x3B  ADC数据高8位 (adc_data[11:04])
        __R_ SW3516H_r0x3C_st ADC_Val_L;  //@0x3C  ADC数据低4位 (adc_data[03:00])
        __R_ u8 _resv_3D[0x33];           //
        __RW SW3516H_r0x70_st PD_Command; //@0x70  PD命令请求
        __RW SW3516H_r0x71_st PD_CMD_ctr; //@0x71  PD命令使能
        __R_ u8 _resv_72;                 //
        __RW SW3516H_r0x73_st src_change; //未确定,手册无
        __R_ u8 _resv_74;
        __RW ______________u8 HardReset_count; //@0x75  HardReset 次数设置  寄存器写 hardreset 命令时的次数设置0= 3 次； 1= 1 次 设置次数后写reg0x70, 发送 hardreset.
        __RW SW3516H_r0x76_st Connect;         //@0x76  连接设置
        __R_ u8 _resv_77[0x2F];
        __RW SW3516H_r0xA6_st Power_Config; //@0xA6  功率配置 (非 PD 和低压直充和双口在线以外的功率设置) 见手册参考表格，用word
        __R_ u8 _resv_A7[3];                //
        __RW SW3516H_r0xAA_st HV_Config0;   //@0xAA  快充配置0 (QC3.0)
        __RW SW3516H_r0xAB_st Port_Config;  //@0xAB  端口配置
        __R_ u8 _resv_AC;                   //
        __RW SW3516H_r0xAD_st HV_Config1;   //@0xAD  快充配置1 (三星1.2V)
        __R_ u8 _resv_AE;                   //
        __RW ______________u8 VID_Config0;  //@0xAF  VID配置0  PD 认证里面的 vendor ID 配置 VID[15:8]高8位
        __RW SW3516H_r0xB0_st PD_Config0;   //@0xB0 PD配置0 （PDO 5V)
        __RW SW3516H_r0xB1_st PD_Config1;   //@0xB1 PD配置1 （PDO 9V)
        __RW SW3516H_r0xB2_st PD_Config2;   //@0xB2 PD配置2  (PDO 12V)
        __RW SW3516H_r0xB3_st PD_Config3;   //@0xB3 PD配置3 （PDO 15V)
        __RW SW3516H_r0xB4_st PD_Config4;   //@0xB4 PD配置4 （PDO 20V)
        __RW SW3516H_r0xB5_st PPS0_Config5; //@0xB5 PD配置5
        __RW SW3516H_r0xB6_st PPS1_Config6; //@0xB6 PD配置6
        __RW SW3516H_r0xB7_st PD_Config7;   //@0xB7 PD配置7 各开关位
        __RW SW3516H_r0xB8_st PD_Config8;   //@0xB8 PD配置8 各开关位
        __RW SW3516H_r0xB9_st FC_Config1;   //@0xB9  快充配置1 (多个协议)
        __RW SW3516H_r0xBA_st FC_Config2;   //@0xBA  快充配置2 (AFC和else最高电压)
        __R_ u8 _resv_BB;                   //
        __RW SW3516H_r0xBC_st FC_Config3;   //@0xBC  快充配置3 (空载超载)
        __RW SW3516H_r0xBD_st CL_Config;    //@0xBD  限流配置 Current_Limiting
        __RW SW3516H_r0xBE_st PD_Config9;   //@0xBE PD配置9 (PPS电压)
        __RW ______________u8 VID_Config1;  //@0xBF  VID配置1  PD 认证里面的 vendor ID 配置 VID[7:0]低8位
    };
} SW3516H_st;

#define SW3516H ((SW3516H_st *)0x1)

void sw3516_Set_Mode(bool force_edit);

void sw3516_Change_PD_1();
bool sw3516_Change_PD_2(bool force_edit);
void sw3516_Except_PD_W(bool force_edit);
void sw3516_Except_PD_V(bool force_edit);

void sw3516_Enable__SCP(bool force_edit);
void sw3516_Both_CUR_LIM(bool force_edit);

bool sw3516_Check_Port_C_1();
bool sw3516_Check_Port_C_2();
SW3516H_r0x07_st sw3516_Check_Port();
bool sw3516_Check_Port_A_1();

u8 sw3516_Read_AD_Value_H();
u16 sw3516_Read_AD_Value();
u16 sw3516_Read_Vin_VOL();
void sw3516_set_ADC_Source(ADC_SEL_enum ADC_SEL);
void sw3516_OpenPDO_20V(bool force_edit);

void SW3516H_Set_PPS(bool force_edit);
// void sw3516_Init_PDO();

void sw3516_OFF_CC();
void sw3516_Set_DMDP(bool force_edit);

void Enable_I2C_Write();
void Disable_I2C_Write();

void check_Samsung();
void set_src_change();
#endif
