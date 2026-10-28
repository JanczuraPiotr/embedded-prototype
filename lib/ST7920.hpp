#ifndef EMBEDDED_PROTOTYPE_ST7920_HPP
#define EMBEDDED_PROTOTYPE_ST7920_HPP

#include <cstdint>

#include "System.hpp"

namespace pj {

class ST7920 {

public:

    ST7920(System &system);

    ST7920(const ST7920&) = delete;
    ST7920& operator=(const ST7920&) = delete;
    ST7920(ST7920&&) = delete;
    ST7920& operator=(ST7920&&) = delete;

    void init() const;

    void clear() const;

    void sendCmd(std::uint8_t cmd) const;

    void sendData(std::uint8_t data) const;

    void sendString(std::uint8_t row, std::uint8_t col, const char *str) const;

private:

    void setCursor(std::uint8_t row, std::uint8_t col) const;

    void toSpi(std::uint8_t sync, std::uint8_t data) const;

    void transmitByte(std::uint8_t byte) const;

    void activate() const;

    void deactivate() const;


    System &system;

};

}

#endif //EMBEDDED_PROTOTYPE_ST7920_HPP
