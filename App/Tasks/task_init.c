#include "FreeRTOS.h"
#include "task.h"

extern void task_lvgl(void *pvParameters);
extern void task_sensor(void *pvParameters);

TaskHandle_t task_sensor_handle;

void task_init(void) {
    xTaskCreate(task_lvgl,   "lvgl",   768, NULL, 4, NULL);
    xTaskCreate(task_sensor, "sensor", 256, NULL, 3, &task_sensor_handle);
}
