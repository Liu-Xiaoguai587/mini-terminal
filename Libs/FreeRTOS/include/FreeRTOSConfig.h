#ifndef __FREERTOSCONFIG_H__
#define __FREERTOSCONFIG_H__

/* 引入 STM32 头文件，确保能读取到 SystemCoreClock 变量 */
#include "stm32f4xx.h"

/*-----------------------------------------------------------
 * Application specific definitions.
 *
 * These definitions should be adjusted for your particular hardware and
 * application requirements.
 *----------------------------------------------------------*/

/* 基础配置 */
#define configUSE_PREEMPTION            1   // 1=抢占式内核，0=协程
#define configUSE_IDLE_HOOK             0   // 是否使用空闲钩子函数
#define configUSE_TICK_HOOK             1   // 是否使用时间片钩子函数
#define configCPU_CLOCK_HZ              ( SystemCoreClock ) // 关键：直接引用 STM32 的主频变量
#define configTICK_RATE_HZ              ( ( TickType_t ) 1000 ) // 系统心跳 1000Hz (1ms)
#define configMAX_PRIORITIES            ( 5 )  // 最大优先级数
#define configMINIMAL_STACK_SIZE        ( ( unsigned short ) 128 ) // 空闲任务的栈大小
#define configTOTAL_HEAP_SIZE           ( ( size_t ) ( 30 * 1024 ) ) // 堆大小：给 FreeRTOS 分配 20KB RAM
#define configMAX_TASK_NAME_LEN         ( 16 ) // 任务名最大长度

/* 互斥量与信号量 */
#define configUSE_TRACE_FACILITY        1
#define configUSE_16_BIT_TICKS          0
#define configIDLE_SHOULD_YIELD         1
#define configUSE_MUTEXES               1

/* 协程配置 (一般不用，置0) */
#define configUSE_CO_ROUTINES           0
#define configMAX_CO_ROUTINE_PRIORITIES ( 2 )

/* API 功能配置 (1=开启，0=关闭) */
#define INCLUDE_vTaskPrioritySet        1
#define INCLUDE_uxTaskPriorityGet       1
#define INCLUDE_vTaskDelete             1
#define INCLUDE_vTaskCleanUpResources   0
#define INCLUDE_vTaskSuspend            1
#define INCLUDE_vTaskDelayUntil         1
#define INCLUDE_vTaskDelay              1

/* Cortex-M 优先级配置 (STM32F4 必须配置这里！) */
#ifdef __NVIC_PRIO_BITS
    #define configPRIO_BITS       __NVIC_PRIO_BITS
#else
    #define configPRIO_BITS       4
#endif

/* 这里的优先级设置非常重要，设置不当会导致中断锁死 */
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY         15
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY    5
#define configKERNEL_INTERRUPT_PRIORITY     ( configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    ( configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )

/*-----------------------------------------------------------
 * 中断映射 (最重要的部分)
 * 将 FreeRTOS 的底层函数映射到 STM32 的中断向量表中
 *----------------------------------------------------------*/
#define vPortSVCHandler    SVC_Handler
#define xPortPendSVHandler PendSV_Handler
#define xPortSysTickHandler SysTick_Handler



#endif // __FREERTOSCONFIG_H__
