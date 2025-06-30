#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "memory.h"
#include "registerfile.h"
#include <string>

class Simulator {
public:
    Simulator();
    void loadBinary(const std::string& filename);
    void step(); // اجرای یک پالس ساعت
    uint32_t getRegisterValue(int);
    Memory& memory1();
    uint32_t getRegisterValue(int index) const;
    uint32_t getPC() const;

private:

    Memory memory;
    RegisterFile regs;
    uint32_t PC;

    void execute(uint32_t instruction);

};

#endif
