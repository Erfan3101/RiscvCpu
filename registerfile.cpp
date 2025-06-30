#include "registerfile.h"

RegisterFile::RegisterFile() {
    regs.fill(0);
    next_regs.fill(0);
}

uint32_t RegisterFile::read(uint8_t index) const {
    return regs[index];
}

void RegisterFile::write(uint8_t index, uint32_t value) {
    if (index != 0) // x0 همیشه صفر می‌مونه
        next_regs[index] = value;
}
void RegisterFile::commit() {
    regs = next_regs;
}
