#ifndef EMBEDDED_PROTOTYPE_SYSTEM_HPP
#define EMBEDDED_PROTOTYPE_SYSTEM_HPP

#include <cstdint>

namespace pj {


class System {

public:
    System() = default;
    virtual ~System() = default;

    System(const System&) = delete;
    System& operator=(const System&) = delete;
    System(System&&) = delete;
    System& operator=(System&&) = delete;

    virtual void delayMs(std::uint32_t ms) const {};
};


}
#endif //EMBEDDED_PROTOTYPE_SYSTEM_HPP
