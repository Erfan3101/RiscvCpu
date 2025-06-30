#ifndef REGISTERFILE_H
#define REGISTERFILE_H

#include <array>
#include <cstdint>

class RegisterFile {
public:
    RegisterFile();
    uint32_t read(uint8_t index) const;
    void write(uint8_t index, uint32_t value);
    void commit(); // ثبت نهایی در پایان هر دستور
private:
    std::array<uint32_t, 32> regs;
    std::array<uint32_t, 32> next_regs;
};

#endif
