#pragma once

#include <cstdint>

#include "FreeRTOS.h"
#include "task.h"
#include "projdefs.h"

#include "System.hpp"

namespace pj {

class FreeRTOS : public System {
public:

    FreeRTOS():System() {};
    ~FreeRTOS() override = default;

    void delayMs(std::uint32_t ms) const final { vTaskDelay(pdMS_TO_TICKS(ms)); };
};

}
