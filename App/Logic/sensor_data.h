#ifndef __SENSOR_DATA_H__
#define __SENSOR_DATA_H__

#include "FreeRTOS.h"
#include "semphr.h"
#include "Sensors.h"

typedef struct {
    /* AHT10 */
    float          temperature;
    float          humidity;
    uint8_t        aht10_ok;

    /* DS3231 */
    DS3231_Time_t  time;
    float          rtc_temp;
    uint8_t        ds3231_ok;

    /* FreeRTOS tick of last successful update */
    TickType_t     last_update;
} SensorData_t;

void         sensor_data_init(void);
SensorData_t sensor_data_get(void);
void         sensor_data_set(const SensorData_t *d);

#endif /* __SENSOR_DATA_H__ */
