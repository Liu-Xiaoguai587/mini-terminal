#include "stm32f4xx.h"
#include "SPI.h"
#include "Utils.h"
#include "ST7735s.h"
#include <stdint.h>
// 128*160

#ifdef USE_DMA
#endif

/********************************************COMMAND***************************************************/
#define NOP     ( 0x00 ) // NOP
#define SWREST  ( 0x01 ) // 软件复位
#define SLPIN   ( 0x10 ) // 进入睡眠模式
#define SLPOUT  ( 0x11 ) // 退出睡眠模式
#define GAMSET  ( 0x26 ) // 设置伽马    parameter(gm)
#define DISPOFF ( 0x28 ) // 关闭显示
#define DISPON  ( 0x29 ) // 开启显示
#define RGBSET  ( 0x2D ) // RGB颜色设置 parameter(R,..., G, ..., B, ...) = 128 
#define CASET   ( 0x2A ) // 设置列地址  parameter(NOP, xStart, NOP, xEND)
#define RASET   ( 0x2B ) // 设置行地址  parameter(NOP, yStart, NOP, yEND)
#define RAMWR   ( 0x2C ) // 写入显存    parameter(...)
#define MADCTL  ( 0x36 ) /* MADCTL(Memory Data Access Control) 此命令定义了帧存储器的读写扫描方向
                          *     parameter ( bit )
                          *     bit: MY | MX | MV | ML | RGB | MH | 0 | 0
                          *         MY: 行地址顺序
                          *         MX: 列地址顺序
                          *         MV: 行/列交换
                          *         ML: 垂直刷新顺序    ;   0 = LCD从上向下刷新, 1 = LCD从下向上刷新
                          *         RGB: RGB or BGR     ;   0 = RGB, 1 = BGR
                          *         MH: 水平刷新顺序    ;   0 = LCD从左向右刷新, 1 = LCD从右向左刷新
                          */
#define COLMODE ( 0x3A ) // 设置颜色模式 parameter( mode )   mode: 011(12位像素) , 101(16位像素) , 110(18位像素)
#define FRMCTR1 ( 0xB1 ) /*   */
#define FRMCTR2 ( 0xB1 ) /*   */
#define FRMCTR3 ( 0xB1 ) /*   */
#define INVCTR  ( 0xB4 ) /* Display Inversion Control 显示反转
                          *     parameter ( bit ) , default (0x07)
                          *     bit: 0 | 0 | 0 | 0 | 0 | NLA | NLB | NLC
                          *         NLA: 全彩正常模式下     0 = 点反转, 1 = 列反转
                          *         NLB: 空闲模式下         0 = 点反转, 1 = 列反转
                          *         NLC: 全彩部分模式下     0 = 点反转, 1 = 列反转
                          */

#define RST_ON      GPIO_SetBits(GPIOA, GPIO_Pin_8)
#define RST_OFF     GPIO_ResetBits(GPIOA, GPIO_Pin_8)
#define SWITCH_DATA_MODE   GPIO_SetBits(GPIOA, GPIO_Pin_9)
#define SWITCH_CMD_MODE    GPIO_ResetBits(GPIOA, GPIO_Pin_9)

#define __CMD(cmd)     ST7735s_WriteCmd(cmd)
#define __DATA(byte)    ST7735s_WriteData(byte)
#define __MUTIDATA(byte, size) ST7735s_WriteMutiData(byte, size)


static void ST7735s_Delay(uint32_t ms) {
    volatile uint32_t i, j;
    for(i = 0; i < ms; i++) 
        for(j = 0; j < 20000; j++);
}

void ST7735s_WriteCmd(uint8_t cmd) {
    SWITCH_CMD_MODE;
    Abstract_SPI_SendByte(cmd);
}

void ST7735s_WriteData(uint8_t byte) {
    SWITCH_DATA_MODE;
    Abstract_SPI_SendByte(byte);
}

void ST7735s_WriteMutiData(uint8_t* byte, uint16_t size) {
    SWITCH_DATA_MODE; 
    Abstract_SPI_SendMutiByte(byte, size);
}

void ST7735s_SetWindwos(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2) {
    __CMD(CASET);
    __DATA(NOP);
    __DATA(x1);
    __DATA(NOP);
    __DATA(x2);

    __CMD(RASET);
    __DATA(NOP);
    __DATA(y1);
    __DATA(NOP);
    __DATA(y2);

    // __CMD(RAMWR);
    // for (uint16_t i = 0; i <  ST7735S_MAX_HIGH * ST7735S_MAX_WIDTH; i++) {
    //     __DATA(0x00);
    //     __DATA(0x00);
    // }
}

void ST7735s_DrawScreen(uint16_t *buf, uint16_t buf_size) {
    ST7735s_SetWindwos(0, 0, ST7735S_MAX_WIDTH - 1, ST7735S_MAX_HIGH - 1);
    __CMD(RAMWR);
    // SWITCH_DATA_MODE;
    // Abstract_SPI_DMA_Send((uint8_t *)buf, buf_size * 2);
    ST7735s_WriteMutiData((uint8_t *)buf, buf_size * 2);
}

void ST7735s_SendColorsDate(uint16_t *buf, uint16_t buf_size) {
    __CMD(RAMWR);
    ST7735s_WriteMutiData((uint8_t *)buf, buf_size * 2);
}

void ST7735s_SendColorsDate_DMA(uint16_t *buf, uint16_t buf_size, spi_dma_done_cb_t done_cb) {
    __CMD(RAMWR);
    SWITCH_DATA_MODE;
    Abstract_SPI_DMA_Send_IT((uint8_t *)buf, buf_size * 2, done_cb);
}

void ST7735s_Init() {
    Abstract_SPI_Init();
    ABstract_SPI_DMA_Init();

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_11; // RST DC BLK
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_SetBits(GPIOA, GPIO_Pin_11); // BLK

    // LCD Reset
    RST_OFF;
    ST7735s_Delay(100);
    RST_ON;
    ST7735s_Delay(50);

    // software reset
    __CMD(SWREST);
    ST7735s_Delay(120);

    // Sleep mode exit
    __CMD(SLPOUT);
    ST7735s_Delay(120);

    //Set frame rate 60fps
    __CMD(0xB1); 
    __DATA(0x01); 
    __DATA(0x2C); 
    __DATA(0x2D); 

    __CMD(0xB2); 
    __DATA(0x01); 
    __DATA(0x2C); 
    __DATA(0x2D); 

    __CMD(0xB3); 
    __DATA(0x01); 
    __DATA(0x2C); 
    __DATA(0x2D); 
    __DATA(0x01); 
    __DATA(0x2C); 
    __DATA(0x2D); 

    // Set RGB565
    __CMD(COLMODE);
    __DATA(0x05);

    // Set My, Mx, RGB
    uint8_t screen[] = {0xA0, 0x60, 0xA8, 0x68, 0x20, 0xE0, 0xc0, 0x80, 0x40, 0};
    __CMD(MADCTL);
    __DATA(screen[0]);
    // __DATA(0xa8);

    // Display On
    __CMD(DISPON);

    ST7735s_Clear();
}


void ST7735s_Clear() {
    __CMD(CASET);
    __DATA(NOP);
    __DATA(0);
    __DATA(NOP);
    // __DATA(ST7735S_MAX_WIDTH - 1);
    __DATA(ST7735S_MAX_WIDTH- 1);

    __CMD(RASET);
    __DATA(NOP);
    __DATA(0);
    __DATA(NOP);
    // __DATA(ST7735S_MAX_HIGH - 1);
    __DATA(ST7735S_MAX_HIGH - 1);

    __CMD(RAMWR);
    for (uint16_t i = 0; i <  ST7735S_MAX_HIGH * ST7735S_MAX_WIDTH; i++) {
        __DATA(0x11);
        __DATA(0x11);
    }
}

void ST7735s_Test() {
    __CMD(CASET);
    __DATA(NOP);
    __DATA(0);
    __DATA(NOP);
    __DATA(10);

    __CMD(RASET);
    __DATA(NOP);
    __DATA(10);
    __DATA(NOP);
    __DATA(20);

    uint16_t data[100] = {0};
    for(uint16_t i = 0; i < 100; i++) {
        data[i] = 0xff;
    }

    __CMD(RAMWR);
    __MUTIDATA((uint8_t *)data, 200);
}

