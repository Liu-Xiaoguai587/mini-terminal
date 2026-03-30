#include "I2c.h"
#include "stm32f4xx.h"

#define IIC_SCL_PIN     GPIO_Pin_8      // PB8
#define IIC_SDA_PIN     GPIO_Pin_9      // PB9
#define IIC_SCL_SRC     GPIO_PinSource8
#define IIC_SDA_SRC     GPIO_PinSource9
#define IIC_TIMEOUT     10000

static void IIC_Bus_Recovery(void) {
    GPIO_InitTypeDef gpio;

    /* SCL as open-drain output, SDA as input */
    gpio.GPIO_Pin   = IIC_SCL_PIN;
    gpio.GPIO_Mode  = GPIO_Mode_OUT;
    gpio.GPIO_OType = GPIO_OType_OD;
    gpio.GPIO_PuPd  = GPIO_PuPd_UP;
    gpio.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOB, &gpio);

    gpio.GPIO_Pin   = IIC_SDA_PIN;
    gpio.GPIO_Mode  = GPIO_Mode_IN;
    GPIO_Init(GPIOB, &gpio);

    GPIO_SetBits(GPIOB, IIC_SCL_PIN);

    /* 9 clock pulses to release any stuck slave */
    for (int i = 0; i < 9; i++) {
        GPIO_ResetBits(GPIOB, IIC_SCL_PIN);
        for (volatile int d = 0; d < 100; d++);
        GPIO_SetBits(GPIOB, IIC_SCL_PIN);
        for (volatile int d = 0; d < 100; d++);
        if (GPIO_ReadInputDataBit(GPIOB, IIC_SDA_PIN)) break;
    }

    /* Generate STOP: SDA low→high while SCL is high */
    gpio.GPIO_Pin   = IIC_SDA_PIN;
    gpio.GPIO_Mode  = GPIO_Mode_OUT;
    gpio.GPIO_OType = GPIO_OType_OD;
    GPIO_Init(GPIOB, &gpio);

    GPIO_ResetBits(GPIOB, IIC_SDA_PIN);
    for (volatile int d = 0; d < 100; d++);
    GPIO_SetBits(GPIOB, IIC_SCL_PIN);
    for (volatile int d = 0; d < 100; d++);
    GPIO_SetBits(GPIOB, IIC_SDA_PIN);
    for (volatile int d = 0; d < 100; d++);
}

void IIC_Init(void) {
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

    /* Recover bus before touching the I2C peripheral */
    IIC_Bus_Recovery();

    RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C1, ENABLE);
    RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C1, DISABLE);

    GPIO_PinAFConfig(GPIOB, IIC_SCL_SRC, GPIO_AF_I2C1);
    GPIO_PinAFConfig(GPIOB, IIC_SDA_SRC, GPIO_AF_I2C1);

    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Pin   = IIC_SCL_PIN | IIC_SDA_PIN;
    GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStruct.GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStruct);

    I2C_InitTypeDef I2C_InitStruct;
    I2C_InitStruct.I2C_Mode                = I2C_Mode_I2C;
    I2C_InitStruct.I2C_DutyCycle           = I2C_DutyCycle_2;
    I2C_InitStruct.I2C_OwnAddress1         = 0x00;
    I2C_InitStruct.I2C_Ack                 = I2C_Ack_Enable;
    I2C_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_InitStruct.I2C_ClockSpeed          = 400000;
    I2C_Init(I2C1, &I2C_InitStruct);

    I2C_Cmd(I2C1, ENABLE);
}

static uint8_t wait_flag(uint32_t flag, FlagStatus status) {
    uint32_t t = IIC_TIMEOUT;
    while (I2C_GetFlagStatus(I2C1, flag) != status)
        if (--t == 0) return 1;
    return 0;
}

static uint8_t wait_event(uint32_t event) {
    uint32_t t = IIC_TIMEOUT;
    while (!I2C_CheckEvent(I2C1, event))
        if (--t == 0) return 1;
    return 0;
}

uint8_t IIC_Write(uint8_t dev_addr, uint8_t reg_addr, uint8_t data) {
    if (wait_flag(I2C_FLAG_BUSY, RESET))                              return 1;
    I2C_GenerateSTART(I2C1, ENABLE);
    if (wait_event(I2C_EVENT_MASTER_MODE_SELECT))                     goto fail;
    I2C_Send7bitAddress(I2C1, dev_addr, I2C_Direction_Transmitter);
    if (wait_event(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))       goto fail;
    I2C_SendData(I2C1, reg_addr);
    if (wait_event(I2C_EVENT_MASTER_BYTE_TRANSMITTED))                goto fail;
    I2C_SendData(I2C1, data);
    if (wait_event(I2C_EVENT_MASTER_BYTE_TRANSMITTED))                goto fail;
    I2C_GenerateSTOP(I2C1, ENABLE);
    return 0;
fail: I2C_GenerateSTOP(I2C1, ENABLE); return 1;
}

uint8_t IIC_Read(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data) {
    if (wait_flag(I2C_FLAG_BUSY, RESET))                              return 1;
    I2C_GenerateSTART(I2C1, ENABLE);
    if (wait_event(I2C_EVENT_MASTER_MODE_SELECT))                     goto fail;
    I2C_Send7bitAddress(I2C1, dev_addr, I2C_Direction_Transmitter);
    if (wait_event(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))       goto fail;
    I2C_SendData(I2C1, reg_addr);
    if (wait_event(I2C_EVENT_MASTER_BYTE_TRANSMITTED))                goto fail;

    I2C_GenerateSTART(I2C1, ENABLE);
    if (wait_event(I2C_EVENT_MASTER_MODE_SELECT))                     goto fail;
    I2C_AcknowledgeConfig(I2C1, DISABLE);
    I2C_Send7bitAddress(I2C1, dev_addr, I2C_Direction_Receiver);
    if (wait_event(I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))          goto fail;
    I2C_GenerateSTOP(I2C1, ENABLE);
    if (wait_flag(I2C_FLAG_RXNE, SET))                                return 1;
    *data = I2C_ReceiveData(I2C1);
    I2C_AcknowledgeConfig(I2C1, ENABLE);
    return 0;
fail: I2C_GenerateSTOP(I2C1, ENABLE); return 1;
}

uint8_t IIC_Write_Buf(uint8_t dev_addr, uint8_t reg_addr, uint8_t *buf, uint16_t len) {
    if (wait_flag(I2C_FLAG_BUSY, RESET))                              return 1;
    I2C_GenerateSTART(I2C1, ENABLE);
    if (wait_event(I2C_EVENT_MASTER_MODE_SELECT))                     goto fail;
    I2C_Send7bitAddress(I2C1, dev_addr, I2C_Direction_Transmitter);
    if (wait_event(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))       goto fail;
    I2C_SendData(I2C1, reg_addr);
    if (wait_event(I2C_EVENT_MASTER_BYTE_TRANSMITTED))                goto fail;
    while (len--) {
        I2C_SendData(I2C1, *buf++);
        if (wait_event(I2C_EVENT_MASTER_BYTE_TRANSMITTED))            goto fail;
    }
    I2C_GenerateSTOP(I2C1, ENABLE);
    return 0;
fail: I2C_GenerateSTOP(I2C1, ENABLE); return 1;
}

uint8_t IIC_Read_Buf(uint8_t dev_addr, uint8_t reg_addr, uint8_t *buf, uint16_t len) {
    if (wait_flag(I2C_FLAG_BUSY, RESET))                              return 1;
    I2C_GenerateSTART(I2C1, ENABLE);
    if (wait_event(I2C_EVENT_MASTER_MODE_SELECT))                     goto fail;
    I2C_Send7bitAddress(I2C1, dev_addr, I2C_Direction_Transmitter);
    if (wait_event(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))       goto fail;
    I2C_SendData(I2C1, reg_addr);
    if (wait_event(I2C_EVENT_MASTER_BYTE_TRANSMITTED))                goto fail;

    I2C_GenerateSTART(I2C1, ENABLE);
    if (wait_event(I2C_EVENT_MASTER_MODE_SELECT))                     goto fail;
    I2C_Send7bitAddress(I2C1, dev_addr, I2C_Direction_Receiver);
    if (wait_event(I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))          goto fail;
    while (len) {
        if (len == 1) {
            I2C_AcknowledgeConfig(I2C1, DISABLE);
            I2C_GenerateSTOP(I2C1, ENABLE);
        }
        if (wait_flag(I2C_FLAG_RXNE, SET))                            goto fail;
        *buf++ = I2C_ReceiveData(I2C1);
        len--;
    }
    I2C_AcknowledgeConfig(I2C1, ENABLE);
    return 0;
fail: I2C_GenerateSTOP(I2C1, ENABLE); return 1;
}

uint8_t IIC_Write_Raw(uint8_t dev_addr, uint8_t *buf, uint16_t len) {
    if (wait_flag(I2C_FLAG_BUSY, RESET))                              return 1;
    I2C_GenerateSTART(I2C1, ENABLE);
    if (wait_event(I2C_EVENT_MASTER_MODE_SELECT))                     goto fail;
    I2C_Send7bitAddress(I2C1, dev_addr, I2C_Direction_Transmitter);
    if (wait_event(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))       goto fail;
    while (len--) {
        I2C_SendData(I2C1, *buf++);
        if (wait_event(I2C_EVENT_MASTER_BYTE_TRANSMITTED))            goto fail;
    }
    I2C_GenerateSTOP(I2C1, ENABLE);
    return 0;
fail: I2C_GenerateSTOP(I2C1, ENABLE); return 1;
}

uint8_t IIC_Read_Raw(uint8_t dev_addr, uint8_t *buf, uint16_t len) {
    if (wait_flag(I2C_FLAG_BUSY, RESET))                              return 1;
    I2C_GenerateSTART(I2C1, ENABLE);
    if (wait_event(I2C_EVENT_MASTER_MODE_SELECT))                     goto fail;
    I2C_Send7bitAddress(I2C1, dev_addr, I2C_Direction_Receiver);
    if (wait_event(I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))          goto fail;
    while (len) {
        if (len == 1) {
            I2C_AcknowledgeConfig(I2C1, DISABLE);
            I2C_GenerateSTOP(I2C1, ENABLE);
        }
        if (wait_flag(I2C_FLAG_RXNE, SET))                            goto fail;
        *buf++ = I2C_ReceiveData(I2C1);
        len--;
    }
    I2C_AcknowledgeConfig(I2C1, ENABLE);
    return 0;
fail: I2C_GenerateSTOP(I2C1, ENABLE); return 1;
}
