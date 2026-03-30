# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Embedded firmware for a mini terminal device based on **STM32F411CEUx** (ARM Cortex-M4, 128KB RAM, 512KB Flash). The device has a 160×128 ST7735S TFT display, an EC11 rotary encoder, and 2 push buttons.

## Build System

This project uses **Keil MDK (µVision)**. There is no Makefile or CMake — build and flash via the Keil IDE by opening `mini_terminal.uvprojx`.

- Compiler: ARM Compiler 5.06 (ARMCC)
- Key defines: `STM32F411xE`, `USE_STDPERIPH_DRIVER`
- Standard: C99
- Build artifacts output to `Objects/` and `Listings/` (gitignored)

For IDE code intelligence (clangd/VS Code), `compile_commands.json` and `.clangd` are configured for `arm-none-eabi`.

## Architecture

### Layer Stack (top to bottom)

```
App/Gui/          — LVGL-based UI pages and page-stack navigation
App/Tasks/        — FreeRTOS task definitions
Port/             — LVGL porting layer (display flush, input device)
Drivers/Peripherals/ — Custom peripheral drivers (SPI, ST7735S, EC11, I2C)
Drivers/Libs/     — STM32F4xx Standard Peripheral Library
Drivers/CMSIS/    — ARM CMSIS + startup code
Libs/FreeRTOS/    — FreeRTOS v10.x (Cortex-M4F port)
Libs/LVGL/        — LVGL v8.3.11 (configured in lv_conf.h)
```

### Startup Sequence (`Main/main.c`)

```
bsp_init()           → initializes display, encoder, buttons
lv_init()            → initializes LVGL
lv_link2_st7735s()   → connects LVGL to ST7735S via Port/lvgl_prot_disp.c
lv_port_indev_init() → registers encoder + EXIT button as LVGL input devices
task_init()          → creates FreeRTOS tasks
vTaskStartScheduler()
```

### FreeRTOS Tasks (`App/Tasks/`)

Only one active task: `task_lvgl_base_timer()` (priority 3, 2KB stack), running every 5ms:
- Scans button inputs with debouncing
- Calls `lv_timer_handler()` for LVGL rendering
- Handles EXIT button for page-stack navigation

### GUI Page System (`App/Gui/gui_control.c`)

Stack-based page navigation (max 10 levels). Each "page" is a function that creates an LVGL screen. Pages push/pop on the stack; the EXIT button pops the current page.

Current pages: main menu (`gui_menu`) → test pages (`gui_test`, `gui_test2`).

### Key Hardware Details

| Peripheral | Interface | Key Pins |
|---|---|---|
| ST7735S display (160×128) | SPI2 + DMA1_Stream4 | CS=PA10, SCK=PB13, MOSI=PB15, RST=PA8, DC=PA9, BL=PA11 |
| EC11 rotary encoder | TIM3 encoder mode | A=PA6, B=PA7 |
| Buttons | GPIO | PB0, PB1 |

- SPI clock: ~42MHz (APB1 84MHz / prescaler 2)
- LVGL display buffer: 128×160 = 20.5KB (full-frame)
- LVGL memory pool: 20KB (`lv_conf.h`)
- EC11 count is divided by 4 (quadrature)
- Button debouncing: 3-sample filter at 5ms polling = stable after 15ms

### Adding a New GUI Page

1. Create `App/Gui/gui_yourpage.c` with a function `void gui_yourpage(void)`
2. Inside, call `lv_scr_act()` or create a new screen with LVGL widgets
3. Register it in the menu list in `App/Gui/gui_menu.c`
4. Add the file to the Keil project in `mini_terminal.uvprojx`
