#include "sensor_data.h"
#include <string.h>

static SensorData_t  s_data;
static SemaphoreHandle_t s_mutex;

void sensor_data_init(void) {
    s_mutex = xSemaphoreCreateMutex();
    memset(&s_data, 0, sizeof(s_data));
}

SensorData_t sensor_data_get(void) {
    SensorData_t copy;
    xSemaphoreTake(s_mutex, portMAX_DELAY);
    copy = s_data;
    xSemaphoreGive(s_mutex);
    return copy;
}

void sensor_data_set(const SensorData_t *d) {
    xSemaphoreTake(s_mutex, portMAX_DELAY);
    s_data = *d;
    xSemaphoreGive(s_mutex);
}
