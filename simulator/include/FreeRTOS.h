/* Simulator stub — replaces real FreeRTOS.h (includes task/semphr stubs inline) */
#ifndef FREERTOS_H
#define FREERTOS_H
#include <stdint.h>
#include <stddef.h>

typedef uint32_t  TickType_t;
typedef long      BaseType_t;
typedef unsigned long UBaseType_t;
typedef void *TaskHandle_t;

#define portMAX_DELAY  ((TickType_t)0xffffffffUL)
#define pdTRUE  ((BaseType_t)1)
#define pdFALSE ((BaseType_t)0)
#define pdPASS  pdTRUE
#define pdFAIL  pdFALSE

/* task.h stubs */
typedef enum {
    eRunning = 0,
    eReady,
    eBlocked,
    eSuspended,
    eDeleted,
    eInvalid
} eTaskState;

static inline eTaskState eTaskStateGet(TaskHandle_t h) { (void)h; return eRunning; }
static inline void vTaskResume(TaskHandle_t h)         { (void)h; }
static inline void vTaskSuspend(TaskHandle_t h)        { (void)h; }
static inline void vTaskDelay(TickType_t t)            { (void)t; }

/* semphr.h stubs */
typedef void *SemaphoreHandle_t;
static inline SemaphoreHandle_t xSemaphoreCreateMutex(void) { return (void*)1; }
static inline BaseType_t xSemaphoreTake(SemaphoreHandle_t m, TickType_t t) { (void)m; (void)t; return pdTRUE; }
static inline BaseType_t xSemaphoreGive(SemaphoreHandle_t m) { (void)m; return pdTRUE; }

#endif /* FREERTOS_H */
