/*
 * Simulator stub for sensor_data — replaces App/Data/sensor_data.c
 * Returns static fake values so gui_sensor and gui_clock pages work.
 */
#include "sensor_data.h"
#include "task.h"

/* Referenced by gui_sensor.c as "extern TaskHandle_t task_sensor_handle" */
TaskHandle_t task_sensor_handle = NULL;

static SensorData_t s_data = {
    .temperature = 25.3f,
    .humidity    = 60.5f,
    .aht10_ok    = 1,
    .time = {
        .sec     = 0,
        .min     = 34,
        .hour    = 12,
        .weekday = 7,
        .date    = 5,
        .month   = 4,
        .year    = 26,   /* 2026 */
    },
    .rtc_temp  = 25.0f,
    .ds3231_ok = 1,
    .last_update = 0,
};

void sensor_data_init(void) { /* no-op */ }

SensorData_t sensor_data_get(void) {
    return s_data;
}

void sensor_data_set(const SensorData_t *d) {
    s_data = *d;
}
