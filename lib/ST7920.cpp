#include "ST7920.hpp"

#include <stm32f411xe.h>

#include "FreeRTOS.h"
#include "task.h"


namespace pj {


ST7920::ST7920(System &system) : system(system)
{

}


void ST7920::init() const
{

    vTaskDelay(pdMS_TO_TICKS(100));

    clear();

    sendCmd(0x30); // Tryb podstawowy instrukcji (8-bit)
    vTaskDelay(pdMS_TO_TICKS(10));
    // sendCmd(0x30);
    vTaskDelay(pdMS_TO_TICKS(1));

    sendCmd(0x0C);
    vTaskDelay(pdMS_TO_TICKS(1));

    sendCmd(0x01);
    vTaskDelay(pdMS_TO_TICKS(30));

    sendCmd(0x06);
    vTaskDelay(pdMS_TO_TICKS(1));
}

void ST7920::clear() const
{
    sendCmd(0x01);
    vTaskDelay(pdMS_TO_TICKS(20));

}

void ST7920::sendCmd(std::uint8_t cmd) const
{
    activate();

    toSpi(0xF8, cmd);

    deactivate();
}

void ST7920::sendData(std::uint8_t data) const
{
    activate();

    toSpi(0xFA, data);

    deactivate();

}

void ST7920::setCursor(std::uint8_t row, std::uint8_t col) const
{
    uint8_t addr = 0x80;
    if (row == 1)      addr += 0x10 + col;
    else if (row == 2) addr += 0x08 + col;
    else if (row == 3) addr += 0x18 + col;
    else               addr += col;
    sendCmd(addr);
}

void ST7920::sendString(std::uint8_t row, std::uint8_t col, const char *str) const
{
    // 1. Ustaw kursor na wybranej pozycji
    setCursor(row, col); //ST7920_SetCursor(row, col);

    // 2. Wysy³aj znaki w trybie wymuszonego czystego ASCII (16-bit)
    while (*str) {
        auto b1 = (std::uint8_t)*str++;

        // Zabezpieczenie przed koñcem stringa
        if (b1 == '\0') break;

        // Jeœli mamy drugi znak w kolejce
        if (*str != '\0') {
            auto b2 = (std::uint8_t)*str++;

            // Wysy³amy parê znaków
            sendData(b1);
            sendData(b2);
        } else {
            // Jeœli zosta³ tylko jeden nieparzysty znak na koñcu stringa,
            // dopychamy go zerem logicznym. To zmusza ST7920 do wyœwietlenia
            // go jako czyste ASCII i zapobiega czekaniu w buforze na kolejny bajt.
            sendData(b1);
            // ST7920_SendData(' ');
        }
    }

}

void ST7920::transmitByte(std::uint8_t byte) const
{
    while (!(SPI1->SR & SPI_SR_TXE)) {}

    *(__IO uint8_t *)&SPI1->DR = byte;

    while (SPI1->SR & SPI_SR_BSY);
}

void ST7920::toSpi(std::uint8_t sync, std::uint8_t data) const
{
    transmitByte(sync);
    transmitByte(data & 0xF0);
    transmitByte((data << 4) & 0xF0);
}

void ST7920::activate() const
{
    GPIOB->BSRR = GPIO_BSRR_BS6;
    for(uint32_t i = 0; i < 20; ++i) {}

}

void ST7920::deactivate() const
{
    GPIOB->BSRR = GPIO_BSRR_BR6;
    for(uint32_t i = 0; i < 20; ++i) {}
}


}
