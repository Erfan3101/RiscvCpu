#ifndef REGISTERFILE_H
#define REGISTERFILE_H

#include <array>
#include <cstdint>

class RegisterFile {
public:
    RegisterFile();
    uint32_t read(uint8_t index) const;
    void write(uint8_t index, uint32_t value);

private:
    std::array<uint32_t, 32> regs;
};

#endif
