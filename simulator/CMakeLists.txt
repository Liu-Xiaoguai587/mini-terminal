cmake_minimum_required(VERSION 3.16)
project(mini_terminal_sim C)
set(CMAKE_C_STANDARD 99)

find_package(SDL2 REQUIRED)

set(ROOT ${CMAKE_SOURCE_DIR}/..)

# --- Sources ---
file(GLOB_RECURSE LVGL_SOURCES ${ROOT}/Libs/LVGL/src/*.c)
file(GLOB         GUI_SOURCES  ${ROOT}/App/Gui/*.c)
# Exclude legacy files prefixed with '_'
list(FILTER GUI_SOURCES EXCLUDE REGEX ".*/_[^/]+\\.c$")

add_executable(simulator
    src/sim_main.c
    src/sim_disp.c
    src/sim_indev.c
    src/sim_sensor.c   # replaces App/Data/sensor_data.c
    ${GUI_SOURCES}
    ${LVGL_SOURCES}
)

# simulator/include MUST come first to shadow FreeRTOS.h / stm32f4xx.h
target_include_directories(simulator PRIVATE
    ${CMAKE_SOURCE_DIR}/include          # stub headers (highest priority)
    ${ROOT}/Libs/LVGL                    # lvgl.h + lv_conf.h
    ${ROOT}/Libs/LVGL/src               # LVGL internals
    ${ROOT}/App/Gui                      # gui_page.h
    ${ROOT}/App/Data                     # sensor_data.h
    ${ROOT}/Drivers/Peripherals          # EC11.h, Sensors.h
)

target_link_libraries(simulator PRIVATE SDL2::SDL2 m)

# Silence warnings from LVGL internals on a desktop compiler
target_compile_options(simulator PRIVATE
    -Wall -Wextra
    -Wno-unused-parameter
    -Wno-sign-compare
    -Wno-implicit-fallthrough
    -Wno-unused-function
)
