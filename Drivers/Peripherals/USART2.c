#include "USART2.h"
#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include <string.h>

/* ── Ring buffer ────────────────────────────────────────────── */
#define RX_BUF_SIZE  512

static volatile uint8_t  rx_buf[RX_BUF_SIZE];
static volatile uint16_t rx_head;   /* ISR writes here  */
static volatile uint16_t rx_tail;   /* task reads here   */

static SemaphoreHandle_t rx_sem;    /* binary: ISR signals new data */

/* ── Helpers ────────────────────────────────────────────────── */
static uint16_t ring_count(void) {
    return (uint16_t)((rx_head - rx_tail) & (RX_BUF_SIZE - 1));
}

static uint8_t ring_get(void) {
    uint8_t c = rx_buf[rx_tail];
    rx_tail = (rx_tail + 1) & (RX_BUF_SIZE - 1);
    return c;
}

/* ── Init ───────────────────────────────────────────────────── */
void USART2_Init(uint32_t baudrate) {
    rx_head = 0;
    rx_tail = 0;
    rx_sem  = xSemaphoreCreateBinary();

    /* Clock enable */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

    /* PA2 = TX (AF7), PA3 = RX (AF7) */
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);

    GPIO_InitTypeDef gpio;
    gpio.GPIO_Pin   = GPIO_Pin_2 | GPIO_Pin_3;
    gpio.GPIO_Mode  = GPIO_Mode_AF;
    gpio.GPIO_OType = GPIO_OType_PP;
    gpio.GPIO_PuPd  = GPIO_PuPd_UP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpio);

    /* USART2 config */
    USART_InitTypeDef uart;
    uart.USART_BaudRate            = baudrate;
    uart.USART_WordLength          = USART_WordLength_8b;
    uart.USART_StopBits            = USART_StopBits_1;
    uart.USART_Parity              = USART_Parity_No;
    uart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    uart.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART2, &uart);

    /* Enable RXNE interrupt */
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);

    /* NVIC: priority 6 (safe for FreeRTOS API from ISR) */
    NVIC_InitTypeDef nvic;
    nvic.NVIC_IRQChannel                   = USART2_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = 6;
    nvic.NVIC_IRQChannelSubPriority        = 0;
    nvic.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&nvic);

    USART_Cmd(USART2, ENABLE);
}

/* ── ISR ────────────────────────────────────────────────────── */
void USART2_IRQHandler(void) {
    BaseType_t wake = pdFALSE;

    if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) {
        uint8_t c = (uint8_t)USART_ReceiveData(USART2);
        uint16_t next = (rx_head + 1) & (RX_BUF_SIZE - 1);
        if (next != rx_tail) {          /* drop byte on overflow */
            rx_buf[rx_head] = c;
            rx_head = next;
        }
        xSemaphoreGiveFromISR(rx_sem, &wake);
    }

    /* Clear overrun if it happens */
    if (USART_GetFlagStatus(USART2, USART_FLAG_ORE) != RESET) {
        (void)USART_ReceiveData(USART2);
    }

    portYIELD_FROM_ISR(wake);
}

/* ── Send (blocking / polled) ───────────────────────────────── */
void USART2_SendByte(uint8_t byte) {
    while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
    USART_SendData(USART2, byte);
    while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);
}

void USART2_SendStr(const char *str) {
    while (*str) {
        USART2_SendByte((uint8_t)*str++);
    }
}

void USART2_SendBuf(const uint8_t *buf, uint16_t len) {
    while (len--) {
        USART2_SendByte(*buf++);
    }
}

/* ── Receive: read one line (up to \n or max_len) ───────────── */
uint16_t USART2_ReadLine(char *buf, uint16_t max_len, uint32_t timeout_ms) {
    uint16_t idx = 0;
    TickType_t deadline = xTaskGetTickCount() + pdMS_TO_TICKS(timeout_ms);

    while (idx < max_len - 1) {
        /* Wait for data if buffer empty */
        while (ring_count() == 0) {
            TickType_t now = xTaskGetTickCount();
            if ((int32_t)(deadline - now) <= 0) {
                buf[idx] = '\0';
                return idx;
            }
            xSemaphoreTake(rx_sem, deadline - now);
        }
        char c = (char)ring_get();
        buf[idx++] = c;
        if (c == '\n') break;
    }
    buf[idx] = '\0';
    return idx;
}

/* ── Receive: read exact byte count ─────────────────────────── */
uint16_t USART2_ReadBytes(uint8_t *buf, uint16_t count, uint32_t timeout_ms) {
    uint16_t idx = 0;
    TickType_t deadline = xTaskGetTickCount() + pdMS_TO_TICKS(timeout_ms);

    while (idx < count) {
        while (ring_count() == 0) {
            TickType_t now = xTaskGetTickCount();
            if ((int32_t)(deadline - now) <= 0) {
                return idx;
            }
            xSemaphoreTake(rx_sem, deadline - now);
        }
        buf[idx++] = ring_get();
    }
    return idx;
}

/* ── Flush RX buffer ────────────────────────────────────────── */
void USART2_FlushRX(void) {
    taskENTER_CRITICAL();
    rx_head = 0;
    rx_tail = 0;
    taskEXIT_CRITICAL();
    xSemaphoreTake(rx_sem, 0);   /* clear pending signal */
}
