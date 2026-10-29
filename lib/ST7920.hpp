#pragma once

#include <cstdint>
#include <stm32f411xe.h>

#include "System.hpp"

namespace pj {

struct DefaultActivate { void operator()() const noexcept {} };
struct DefaultDeactivate { void operator()() const noexcept {} };

template <typename ActivateFun = DefaultActivate, typename DeactivateFun = DefaultDeactivate>
class ST7920 {

public:

    // NOTE AI
    enum Cmd : std::uint8_t {
        // 1. Maski bazowe instrukcji (bity początkowe poleceń)
        DisplayClear    = 0x01, // Czyszczenie ekranu (Wymaga dłuższego opóźnienia, ok. 1.6 - 4.6 ms!)
        DisplayHome     = 0x02, // Powrót kursora na pozycję początkową (adres 0)
        EntryModeSet    = 0x04, // Baza dla trybu wprowadzania danych
        DisplayCtrlBase = 0x08, // Baza dla sterowania wyświetlaczem (Display ON/OFF/Blink)
        CursorShift     = 0x10, // Kursor / przesunięcie wyświetlacza
        FunctionSet     = 0x20, // Baza konfiguracji funkcji (tryb 4/8 bit, zestaw instrukcji)
        SetCGRAMAddress = 0x40, // Ustawienie adresu RAM generatora znaków
        SetDDRAMAddress = 0x80, // Ustawienie adresu RAM wyświetlacza (tekstu)

        // 2. Gotowe kombinacje dla EntryModeSet (w tym komenda 0x06)
        EntryModeDefault = 0x06, // Kursor w prawo, ekran stoi w miejscu (Najczęstsza konfiguracja)
        EntryModeShift   = 0x07, // Kursor w prawo, ale cały ekran przesuwa się w lewo przy pisaniu

        // 3. Gotowe kombinacje dla DisplayCtrlBase
        DisplayOff      = 0x08, // Ekran wygaszony, kursor wyłączony
        DisplayOn       = 0x0C, // Wyświetlacz włączony, kursor wyłączony (Typowy stan pracy)
        DisplayOnCursor = 0x0E, // Wyświetlacz włączony + widoczny kursor podtekstowy
        DisplayOnBlink  = 0x0F, // Wyświetlacz włączony + kursor mruga jako pełny kwadrat

        // 4. Tryby specyficzne dla ST7920 (Przełączanie instrukcji Basic / Extended)
        BasicMode       = 0x30, // Tryb instrukcji podstawowych (używany do pisania tekstu)
        ExtendedMode    = 0x34, // Tryb instrukcji rozszerzonych (niezbędny do konfiguracji grafiki)
        GraphicOn       = 0x36, // Włączenie wyświetlania grafiki (wymaga wcześniejszego ExtendedMode)
        GraphicOff      = 0x34, // Wyłączenie wyświetlania grafiki (powrót do samej konfiguracji rozszerzonej)
    };


    ST7920(System &system,
        SPI_TypeDef *spi,
        ActivateFun activateFun = DefaultActivate{},
        DeactivateFun deactivateFun = DefaultDeactivate{}) noexcept;

    ST7920(const ST7920&) = delete;
    ST7920& operator=(const ST7920&) = delete;
    ST7920(ST7920&&) = delete;
    ST7920& operator=(ST7920&&) = delete;

    ~ST7920() = default;

    void init() const;

    void clear() const;

    void sendCmd(std::uint8_t cmd) const;

    void sendData(std::uint8_t data) const;

    void sendString(std::uint8_t row, std::uint8_t col, const char *str) const;

private:

    void setCursor(std::uint8_t row, std::uint8_t col) const;

    void toSpi(std::uint8_t sync, std::uint8_t data) const;

    void transmitByte(std::uint8_t byte) const;

    System &system;
    SPI_TypeDef *spi;
    ActivateFun activateFun;
    DeactivateFun deactivateFun;

};


// --------------------
// implementation


template <typename ActivateFun, typename DeactivateFun>
ST7920<ActivateFun, DeactivateFun>::ST7920(System &system,
    SPI_TypeDef *spi,
    ActivateFun activateFun,
    DeactivateFun deactivateFun) noexcept
    : system(system),
    spi(spi),
    activateFun(activateFun),
    deactivateFun(deactivateFun)
{
}

template <typename ActivateFun, typename DeactivateFun>
void ST7920<ActivateFun, DeactivateFun>::init() const
{
    system.delayMs(100);

    clear();

    sendCmd(Cmd::BasicMode);
    system.delayMs(10);
    system.delayMs(1);

    sendCmd(Cmd::DisplayOn);
    system.delayMs(1);

    sendCmd(Cmd::DisplayClear);
    system.delayMs(30);

    sendCmd(Cmd::EntryModeDefault);
    system.delayMs(1);
}

template <typename ActivateFun, typename DeactivateFun>
void ST7920<ActivateFun, DeactivateFun>::clear() const
{
    sendCmd(Cmd::DisplayClear);
    system.delayMs(20);
}

template <typename ActivateFun, typename DeactivateFun>
void ST7920<ActivateFun, DeactivateFun>::sendCmd(std::uint8_t cmd) const
{
    activateFun();

    toSpi(0xF8, cmd);

    deactivateFun();
}

template <typename ActivateFun, typename DeactivateFun>
void ST7920<ActivateFun, DeactivateFun>::sendData(std::uint8_t data) const
{
    activateFun();

    toSpi(0xFA, data);

    deactivateFun();
}

template <typename ActivateFun, typename DeactivateFun>
void ST7920<ActivateFun, DeactivateFun>::setCursor(std::uint8_t row, std::uint8_t col) const
{
    uint8_t addr = 0x80;
    if (row == 1)      addr += 0x10 + col;
    else if (row == 2) addr += 0x08 + col;
    else if (row == 3) addr += 0x18 + col;
    else               addr += col;
    sendCmd(addr);
}

template <typename ActivateFun, typename DeactivateFun>
void ST7920<ActivateFun, DeactivateFun>::sendString(std::uint8_t row, std::uint8_t col, const char *str) const
{
    // NOTE AI
    setCursor(row, col);

    while (*str) {
        auto b1 = (std::uint8_t)*str++;

        if (b1 == '\0') break;

        if (*str != '\0') {
            auto b2 = (std::uint8_t)*str++;
            sendData(b1);
            sendData(b2);
        } else {
            sendData(b1);
        }
    }
}

template <typename ActivateFun, typename DeactivateFun>
void ST7920<ActivateFun, DeactivateFun>::transmitByte(std::uint8_t byte) const
{
    while (!(spi->SR & SPI_SR_TXE)) {}

    *(__IO uint8_t *)&spi->DR = byte;

    while (spi->SR & SPI_SR_BSY);
}

template <typename ActivateFun, typename DeactivateFun>
void ST7920<ActivateFun, DeactivateFun>::toSpi(std::uint8_t sync, std::uint8_t data) const
{
    transmitByte(sync);
    transmitByte(data & 0xF0);
    transmitByte((data << 4) & 0xF0);
}

}