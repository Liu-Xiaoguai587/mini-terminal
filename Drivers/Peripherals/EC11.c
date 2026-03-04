#include "stm32f4xx.h"

#define EC11_TIM TIM3
#define EC11_Pin_SW GPIO_Pin_5
#define EC11_Pin_B GPIO_Pin_6
#define EC11_Pin_A GPIO_Pin_7

void EC11_Init(void) {
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_TIM3);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_TIM3);

    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Pin = EC11_Pin_SW;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;  // 上拉输入
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(GPIOA, &GPIO_InitStruct);


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
    TIM_ICInitStruct.TIM_ICFilter = 10;
    TIM_ICInit(EC11_TIM, &TIM_ICInitStruct);

    TIM_SetCounter(EC11_TIM, 0);

    TIM_Cmd(EC11_TIM, ENABLE);
}

int16_t EC11_Get_Count(void) {
    int16_t count = (int16_t)TIM_GetCounter(EC11_TIM);
    TIM_SetCounter(EC11_TIM, 0);

    return count / 4;
}

uint8_t EC11_Get_SW_State(void) {
    return GPIO_ReadInputDataBit(GPIOA, EC11_Pin_SW);
}
