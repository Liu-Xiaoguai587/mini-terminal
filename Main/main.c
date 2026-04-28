#include "lvgl.h"
#include "FreeRTOS.h"
#include "task.h"

extern void lv_link2_st7735s(void);
extern void bsp_init(void);
extern void task_init(void);
extern void lv_port_indev_init(void);
extern void sensor_data_init(void);
extern void net_data_init(void);

int main() {
    bsp_init();
    sensor_data_init();
    net_data_init();
    lv_init();
    lv_link2_st7735s();
    lv_port_indev_init();

    task_init();
    vTaskStartScheduler();

    for(;;);
}
