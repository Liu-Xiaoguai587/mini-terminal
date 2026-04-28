#include "FreeRTOS.h"
#include "task.h"

extern void task_lvgl(void *pvParameters);
extern void task_sensor(void *pvParameters);
extern void task_wifi(void *pvParameters);

TaskHandle_t task_sensor_handle;
TaskHandle_t task_wifi_handle;
TaskHandle_t task_lvgl_handle;

void task_init(void) {
    xTaskCreate(task_lvgl,   "lvgl",   768, NULL, 4, &task_lvgl_handle);
    xTaskCreate(task_sensor, "sensor", 256, NULL, 3, &task_sensor_handle);
    xTaskCreate(task_wifi,   "wifi",   512, NULL, 2, &task_wifi_handle);
}
