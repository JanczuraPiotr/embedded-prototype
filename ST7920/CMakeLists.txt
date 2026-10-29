cmake_minimum_required(VERSION 3.15)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

project(ST7920 C CXX ASM)

get_filename_component(PROJECT_ROOT "${PROJECT_SOURCE_DIR}/.." ABSOLUTE)
set(PROJECT_ROOT "${PROJECT_ROOT}" CACHE PATH "Project root directory" FORCE)
message("PROJECT_ROOT ${PROJECT_ROOT}")


set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(MCU_FLAGS "-mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${MCU_FLAGS} -ggdb3 -O0 -ffunction-sections -fdata-sections -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MCU_FLAGS} -ggdb3 -O0 -ffunction-sections -fdata-sections -Wall -fno-exceptions -fno-rtti")
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} ${MCU_FLAGS} -x assembler-with-cpp")

# STM32F411RE: 512 KiB Flash, 128 KiB RAM.
set(LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/STM32F411RETx_FLASH.ld"
        CACHE FILEPATH "Linker script for STM32F411RE")
if(NOT EXISTS "${LINKER_SCRIPT}")
    message(FATAL_ERROR "Missing linker script: ${LINKER_SCRIPT}")
endif()

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${MCU_FLAGS} -T${LINKER_SCRIPT} --specs=nano.specs -Wl,--gc-sections -Wl,-Map=${PROJECT_NAME}.map")

add_compile_definitions(STM32F411xE)

get_filename_component(SHARED_DEPS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/.." ABSOLUTE)
set(FREERTOS_DIR "${SHARED_DEPS_DIR}/FreeRTOS/FreeRTOS")
set(STM32CUBE_DIR "${SHARED_DEPS_DIR}/Drivers/STM32CubeF4"
        CACHE PATH "STM32CubeF4 package directory")

if(NOT EXISTS "${FREERTOS_DIR}/Source/tasks.c" OR
        NOT EXISTS "${STM32CUBE_DIR}/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/system_stm32f4xx.c")
    message(FATAL_ERROR "Missing dependencies: provide FreeRTOS at ${FREERTOS_DIR} and STM32CubeF4 (including CMSIS submodules) at ${STM32CUBE_DIR}. Override STM32CUBE_DIR if needed.")
endif()


add_compile_definitions(projCOVERAGE_TEST=0)

file(GLOB FREERTOS_CORE_SOURCES "${FREERTOS_DIR}/Source/*.c")

set(FREERTOS_SOURCES
        ${FREERTOS_CORE_SOURCES}
        "${FREERTOS_DIR}/Source/portable/GCC/ARM_CM4F/port.c"
        "${FREERTOS_DIR}/Source/portable/MemMang/heap_4.c"
)

set(CMSIS_SOURCES
        "${STM32CUBE_DIR}/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/startup_stm32f411xe.s"
        "${STM32CUBE_DIR}/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/system_stm32f4xx.c"
        FreeRTOSConfig.h
)

add_library(freertos STATIC ${FREERTOS_SOURCES})
# Keep the vector table and Reset_Handler in the final executable.
add_library(cmsis OBJECT ${CMSIS_SOURCES})

add_executable(${PROJECT_NAME}
        main.cpp
        ${PROJECT_ROOT}/lib/System.hpp
        ${PROJECT_ROOT}/lib/FreeRTOS.hpp
        ${PROJECT_ROOT}/lib/ST7920.hpp
)

# Relink whenever the memory layout changes.
set_property(TARGET ${PROJECT_NAME} APPEND PROPERTY LINK_DEPENDS "${LINKER_SCRIPT}")

target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${FREERTOS_DIR}/Source/include
        ${FREERTOS_DIR}/Source/portable/GCC/ARM_CM4F
        ${STM32CUBE_DIR}/Drivers/CMSIS/Device/ST/STM32F4xx/Include
        ${STM32CUBE_DIR}/Drivers/CMSIS/Include
)

target_include_directories(freertos PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${FREERTOS_DIR}/Source/include
        ${FREERTOS_DIR}/Source/portable/GCC/ARM_CM4F
)

target_include_directories(cmsis PRIVATE
        ${STM32CUBE_DIR}/Drivers/CMSIS/Device/ST/STM32F4xx/Include
        ${STM32CUBE_DIR}/Drivers/CMSIS/Include
)

target_link_libraries(${PROJECT_NAME} PRIVATE freertos cmsis)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${PROJECT_NAME}> $<TARGET_FILE:${PROJECT_NAME}>.hex
        COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${PROJECT_NAME}> $<TARGET_FILE:${PROJECT_NAME}>.bin
        COMMENT "Generating Flash images (.hex and .bin)..."
        VERBATIM
)

message("CMAKE_CURRENT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}")
message("STM32CUBE_DIR ${STM32CUBE_DIR}")
message("FREERTOS_DIR ${FREERTOS_DIR}")
message("CMSIS_SOURCES ${CMSIS_SOURCES}")
