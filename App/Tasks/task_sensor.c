#include "FreeRTOS.h"
#include "task.h"
#include "sensor_data.h"
#include "Sensors.h"

void task_sensor(void *pvParameters) {
    SensorData_t d = {0};
    TickType_t last_aht10 = 0;

    for (;;) {
        TickType_t now = xTaskGetTickCount();

        /* DS3231: read time every 1s */
        d.ds3231_ok = (DS3231_Get_Time(&d.time) == 0);
        d.rtc_temp  = DS3231_Get_Temp();

        /* AHT10: read temp/humidity every 2s (measurement takes ~80ms) */
        if (now - last_aht10 >= pdMS_TO_TICKS(2000)) {
            d.aht10_ok = (AHT10_Read(&d.temperature, &d.humidity) == 0);
            last_aht10 = now;
        }

        d.last_update = now;
        sensor_data_set(&d);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
