#include "stm32f4xx.h"
#include "EC11.h"

#define EC11_TIM TIM3
#define EC11_Pin_B GPIO_Pin_6
#define EC11_Pin_A GPIO_Pin_7

#define BTN_SELECT_PIN  GPIO_Pin_0  // PB0
#define BTN_EXIT_PIN    GPIO_Pin_1  // PB1

void EC11_Init(void) {
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_TIM3);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_TIM3);

    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Pin = EC11_Pin_A | EC11_Pin_B;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;  // 上拉输入
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    TIM_TimeBaseInitTypeDef TIM_TimeBaseStruct;
    TIM_TimeBaseStruct.TIM_Prescaler = 0;
    TIM_TimeBaseStruct.TIM_Period = 0xFFFF;
    TIM_TimeBaseStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(EC11_TIM, &TIM_TimeBaseStruct);

    TIM_EncoderInterfaceConfig(EC11_TIM, TIM_EncoderMode_TI12, TIM_ICPolarity_Falling, TIM_ICPolarity_Falling);

    TIM_ICInitTypeDef TIM_ICInitStruct;
    TIM_ICInitStruct.TIM_ICPolarity  = TIM_ICPolarity_Falling;
    TIM_ICInitStruct.TIM_ICSelection = TIM_ICSelection_DirectTI;
    TIM_ICInitStruct.TIM_ICPrescaler = TIM_ICPSC_DIV1;
    TIM_ICInitStruct.TIM_ICFilter    = 10;

    TIM_ICInitStruct.TIM_Channel = TIM_Channel_1;
    TIM_ICInit(EC11_TIM, &TIM_ICInitStruct);

    TIM_ICInitStruct.TIM_Channel = TIM_Channel_2;
    TIM_ICInit(EC11_TIM, &TIM_ICInitStruct);

    TIM_SetCounter(EC11_TIM, 0);

    TIM_Cmd(EC11_TIM, ENABLE);
}

void Button_Init(void) {
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Pin   = BTN_SELECT_PIN | BTN_EXIT_PIN;
    GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOB, &GPIO_InitStruct);
}

static uint8_t sel_stable = 1;
static uint8_t ext_stable = 1;

// 返回当前消抖后的按键电平（BTN_SELECT / BTN_EXIT 置位表示正在按住）
uint8_t Button_Get_State(void) {
    uint8_t state = 0;
    if (sel_stable == 0) state |= BTN_SELECT;
    if (ext_stable == 0) state |= BTN_EXIT;
    return state;
}

// 在 LVGL read_cb 中调用，更新消抖状态
uint8_t Button_Scan(void) {
    static uint8_t sel_cnt = 0;
    static uint8_t ext_cnt = 0;
    uint8_t events = 0;

    uint8_t sel_raw = GPIO_ReadInputDataBit(GPIOB, BTN_SELECT_PIN);
    if (sel_raw != sel_stable) {
        if (++sel_cnt >= 3) { sel_stable = sel_raw; sel_cnt = 0; if (sel_raw == 0) events |= BTN_SELECT; }
    } else { sel_cnt = 0; }

    uint8_t ext_raw = GPIO_ReadInputDataBit(GPIOB, BTN_EXIT_PIN);
    if (ext_raw != ext_stable) {
        if (++ext_cnt >= 3) { ext_stable = ext_raw; ext_cnt = 0; if (ext_raw == 0) events |= BTN_EXIT; }
    } else { ext_cnt = 0; }

    return events;
}

int16_t EC11_Get_Count(void) {
    static int16_t remainder = 0;
    int16_t raw = (int16_t)TIM_GetCounter(EC11_TIM);
    TIM_SetCounter(EC11_TIM, 0);

    remainder += raw;
    int16_t count = remainder / 4;
    remainder %= 4;

    return count;
}
