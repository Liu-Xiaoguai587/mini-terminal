#include "Sensors.h"
#include "I2c.h"
#include "FreeRTOS.h"
#include "task.h"

/* ═══════════════════════════════════════════════════════
 *  AHT10  (温湿度传感器)
 *  地址: 0x38，无寄存器地址，命令驱动
 * ═══════════════════════════════════════════════════════ */
#define AHT10_ADDR  (0x38 << 1)

void AHT10_Init(void) {
    uint8_t cmd[3] = {0xE1, 0x08, 0x00};
    IIC_Write_Raw(AHT10_ADDR, cmd, 3);
}

uint8_t AHT10_Read(float *temperature, float *humidity) {
    // 1. 发送触发测量命令
    uint8_t cmd[3] = {0xAC, 0x33, 0x00};
    if (IIC_Write_Raw(AHT10_ADDR, cmd, 3)) return 1;

    // 2. 等待测量完成。避免密集轮询 I2C，防止总线偶发卡死。
    vTaskDelay(pdMS_TO_TICKS(80));

    // 3. 读取 6 字节数据：buf[0] 是状态字节，bit7=1 表示仍忙。
    uint8_t buf[6];
    if (IIC_Read_Raw(AHT10_ADDR, buf, 6)) return 1;
    if (buf[0] & 0x80) return 1;

    // 4. 解析（20bit 原始值）
    uint32_t raw_hum  = ((uint32_t)buf[1] << 12) | ((uint32_t)buf[2] << 4) | (buf[3] >> 4);
    uint32_t raw_temp = (((uint32_t)(buf[3] & 0x0F)) << 16) | ((uint32_t)buf[4] << 8) | buf[5];

    *humidity    = (float)raw_hum  / 1048576.0f * 100.0f;
    *temperature = (float)raw_temp / 1048576.0f * 200.0f - 50.0f;
    return 0;
}

/* ═══════════════════════════════════════════════════════
 *  DS3231  (RTC)
 *  地址: 0x68，标准寄存器读写，BCD 编码
 * ═══════════════════════════════════════════════════════ */
#define DS3231_ADDR  (0x68 << 1)
#define BCD2DEC(x)   (((x) >> 4) * 10 + ((x) & 0x0F))
#define DEC2BCD(x)   (((x) / 10 << 4) | ((x) % 10))

void DS3231_Init(void) {
    IIC_Write(DS3231_ADDR, 0x0E, 0x00);  // 关闭 SQW 输出，启用振荡器
    IIC_Write(DS3231_ADDR, 0x0F, 0x00);  // 清除 OSF 标志
}

uint8_t DS3231_Set_Time(DS3231_Time_t *t) {
    uint8_t buf[7] = {
        DEC2BCD(t->sec),
        DEC2BCD(t->min),
        DEC2BCD(t->hour),
        DEC2BCD(t->weekday),
        DEC2BCD(t->date),
        DEC2BCD(t->month),
        DEC2BCD(t->year)
    };
    return IIC_Write_Buf(DS3231_ADDR, 0x00, buf, 7);
}

uint8_t DS3231_Get_Time(DS3231_Time_t *t) {
    uint8_t buf[7];
    if (IIC_Read_Buf(DS3231_ADDR, 0x00, buf, 7)) return 1;
    t->sec     = BCD2DEC(buf[0]);
    t->min     = BCD2DEC(buf[1]);
    t->hour    = BCD2DEC(buf[2] & 0x3F);  // 屏蔽 12/24h 位
    t->weekday = BCD2DEC(buf[3]);
    t->date    = BCD2DEC(buf[4]);
    t->month   = BCD2DEC(buf[5] & 0x1F);  // 屏蔽世纪位
    t->year    = BCD2DEC(buf[6]);
    return 0;
}

float DS3231_Get_Temp(void) {
    uint8_t msb, lsb;
    IIC_Read(DS3231_ADDR, 0x11, &msb);
    IIC_Read(DS3231_ADDR, 0x12, &lsb);
    // MSB 为有符号整数部分，LSB bit7:6 为小数部分（0.25°C/bit）
    return (float)(int8_t)msb + ((lsb >> 6) & 0x03) * 0.25f;
}
