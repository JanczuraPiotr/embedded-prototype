cmake_minimum_required(VERSION 3.30)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

project(EmbGenerator1 C CXX ASM)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(MCU_FLAGS "-mcpu=cortex-m7 -mthumb -mfpu=fpv5-sp-d16 -mfloat-abi=hard")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${MCU_FLAGS} -ggdb3 -O0 -ffunction-sections -fdata-sections -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MCU_FLAGS} -ggdb3 -O0 -ffunction-sections -fdata-sections -Wall -fno-exceptions -fno-rtti")
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} ${MCU_FLAGS} -x assembler-with-cpp")

set(LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/STM32F767ZITx_FLASH.ld")
if(NOT EXISTS "${LINKER_SCRIPT}")
    message(FATAL_ERROR "Missing linker script: ${LINKER_SCRIPT}")
endif()

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${MCU_FLAGS} -T${LINKER_SCRIPT} --specs=nano.specs -Wl,--gc-sections -Wl,-Map=${PROJECT_NAME}.map")

add_compile_definitions(STM32F767xx)

get_filename_component(SHARED_DEPS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/.." ABSOLUTE)
set(FREERTOS_DIR "${SHARED_DEPS_DIR}/FreeRTOS/FreeRTOS")
set(STM32CUBE_DIR "${SHARED_DEPS_DIR}/Drivers/STM32CubeF7")

if(NOT EXISTS "${FREERTOS_DIR}/Source/tasks.c" OR
   NOT EXISTS "${STM32CUBE_DIR}/Drivers/CMSIS/Device/ST/STM32F7xx/Source/Templates/system_stm32f7xx.c")
    message(FATAL_ERROR "Missing shared dependencies. Run git submodule update --init --recursive from ${SHARED_DEPS_DIR}")
endif()


add_compile_definitions(projCOVERAGE_TEST=0)

file(GLOB FREERTOS_CORE_SOURCES "${FREERTOS_DIR}/Source/*.c")

set(FREERTOS_SOURCES
        ${FREERTOS_CORE_SOURCES}
        "${FREERTOS_DIR}/Source/portable/GCC/ARM_CM7/r0p1/port.c"
        "${FREERTOS_DIR}/Source/portable/MemMang/heap_4.c" # heap_4 jest lepszy dla mikrokontrolerów niż heap_3
)

set(CMSIS_SOURCES
        "${STM32CUBE_DIR}/Drivers/CMSIS/Device/ST/STM32F7xx/Source/Templates/gcc/startup_stm32f767xx.s"
        "${STM32CUBE_DIR}/Drivers/CMSIS/Device/ST/STM32F7xx/Source/Templates/system_stm32f7xx.c"
)

add_library(freertos STATIC ${FREERTOS_SOURCES})
add_library(cmsis STATIC ${CMSIS_SOURCES})

add_executable(${PROJECT_NAME}
        main.cpp
        console.h
        console.c
)

target_compile_definitions(${PROJECT_NAME} PRIVATE STM32F767xx)
target_compile_definitions(freertos PRIVATE STM32F767xx)
target_compile_definitions(cmsis PRIVATE STM32F767xx)

target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR} # Dla FreeRTOSConfig.h
        ${FREERTOS_DIR}/Source/include
        ${FREERTOS_DIR}/Source/portable/GCC/ARM_CM7/r0p1
        ${STM32CUBE_DIR}/Drivers/CMSIS/Device/ST/STM32F7xx/Include
        ${STM32CUBE_DIR}/Drivers/CMSIS/Include
)

target_include_directories(freertos PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${FREERTOS_DIR}/Source/include
        ${FREERTOS_DIR}/Source/portable/GCC/ARM_CM7/r0p1
)

target_include_directories(cmsis PRIVATE
        ${STM32CUBE_DIR}/Drivers/CMSIS/Device/ST/STM32F7xx/Include
        ${STM32CUBE_DIR}/Drivers/CMSIS/Include
)

target_link_libraries(${PROJECT_NAME} PRIVATE freertos cmsis)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O ihex ${PROJECT_NAME} ${PROJECT_NAME}.hex
        COMMAND ${CMAKE_OBJCOPY} -O binary ${PROJECT_NAME} ${PROJECT_NAME}.bin
        COMMENT "Generowanie plików Flash (.hex i .bin)..."
)

message("CMAKE_CURRENT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}")
message("STM32CUBE_DIR ${STM32CUBE_DIR}")
message("FREERTOS_DIR ${FREERTOS_DIR}")
message("CMSIS_SOURCES ${CMSIS_SOURCES}")
