#include "lvgl.h"
#include "FreeRTOS.h"
#include "task.h"

extern void lv_link2_st7735s(void);
extern void bsp_init(void);
extern void task_init(void);
extern void lv_port_indev_init(void);



int main() {
    bsp_init();
    lv_init();
    lv_link2_st7735s();
    lv_port_indev_init();

    
    task_init();
    /* 6. 启动调度 */
    vTaskStartScheduler();

    for(;;);
}

