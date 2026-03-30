#ifndef __SENSORS_H__
#define __SENSORS_H__

#include "stm32f4xx.h"

/* ── AHT10 ─────────────────────────────────────────── */
void    AHT10_Init(void);
uint8_t AHT10_Read(float *temperature, float *humidity);

/* ── DS3231 ─────────────────────────────────────────── */
typedef struct {
    uint8_t sec;      // 0-59
    uint8_t min;      // 0-59
    uint8_t hour;     // 0-23
    uint8_t weekday;  // 1-7
    uint8_t date;     // 1-31
    uint8_t month;    // 1-12
    uint8_t year;     // 0-99 (2000-2099)
} DS3231_Time_t;

void    DS3231_Init(void);
uint8_t DS3231_Set_Time(DS3231_Time_t *t);
uint8_t DS3231_Get_Time(DS3231_Time_t *t);
float   DS3231_Get_Temp(void);

#endif // __SENSORS_H__
