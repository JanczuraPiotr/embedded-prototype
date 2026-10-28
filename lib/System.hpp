#ifndef EMBEDDED_PROTOTYPE_SYSTEM_HPP
#define EMBEDDED_PROTOTYPE_SYSTEM_HPP

class System {

public:
    System() = default;
    ~System() = default;

    System(const System&) = delete;
    System& operator=(const System&) = delete;
    System(System&&) = delete;
    System& operator=(System&&) = delete;

    void delay();
};


#endif //EMBEDDED_PROTOTYPE_SYSTEM_HPP
