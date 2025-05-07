# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================



cmake_minimum_required(VERSION 3.13)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(PICO_BOARD pico_w CACHE STRING "Board type")
include(pico_sdk_import.cmake)
set(FREERTOS_KERNEL_PATH "C:/Users/malub/FreeRTOS-Kernel")
include(C:/Users/malub/FreeRTOS-Kernel/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)

project(Tarefa3_EmbarcaTech2_project C CXX ASM)
pico_sdk_init()


include_directories(${CMAKE_SOURCE_DIR}/lib)


add_executable(Tarefa3_EmbarcaTech2_project  
Tarefa3_EmbarcaTech2.c 
        lib/ssd1306.c # Biblioteca para o display OLED
        )

target_include_directories(Tarefa3_EmbarcaTech2_project PRIVATE ${CMAKE_CURRENT_LIST_DIR}/generated)

# Gerar cabeçalho PIO
file(MAKE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/generated)
pico_generate_pio_header(Tarefa3_EmbarcaTech2_project ${CMAKE_CURRENT_LIST_DIR}/Tarefa3_EmbarcaTech2.pio OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/generated)

# Definir nome e versão do programa
pico_set_program_name(Tarefa3_EmbarcaTech2_project "Tarefa3_EmbarcaTech2_project")
pico_set_program_version(Tarefa3_EmbarcaTech2_project "0.1")

target_link_libraries(Tarefa3_EmbarcaTech2_project 
        pico_stdlib 
        hardware_pio
        hardware_gpio
        hardware_i2c
        hardware_pwm
        FreeRTOS-Kernel 
        FreeRTOS-Kernel-Heap4
        )

pico_enable_stdio_usb(Tarefa3_EmbarcaTech2_project 1)
pico_enable_stdio_uart(Tarefa3_EmbarcaTech2_project 1)

pico_add_extra_outputs(Tarefa3_EmbarcaTech2_project)




