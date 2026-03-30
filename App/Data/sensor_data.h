#ifndef __SENSOR_DATA_H__
#define __SENSOR_DATA_H__

#include "FreeRTOS.h"
#include "Sensors.h"

typedef struct {
    float          temperature;   /* AHT10 */
    float          humidity;      /* AHT10 */
    uint8_t        aht10_ok;      /* 0 = read failed */
    DS3231_Time_t  time;          /* DS3231 */
    float          rtc_temp;      /* DS3231 on-chip temperature */
    uint8_t        ds3231_ok;
    TickType_t     last_update;
} SensorData_t;

void         sensor_data_init(void);
SensorData_t sensor_data_get(void);
void         sensor_data_set(const SensorData_t *d);

#endif /* __SENSOR_DATA_H__ */
