#include "memory.h"
#include <stdexcept>

Memory::Memory() : data(MEM_SIZE, 0) {}

uint32_t Memory::load_word(uint32_t addr) {
    if (addr + 3 >= MEM_SIZE) throw std::out_of_range("Address out of range");
    return data[addr] | (data[addr+1] << 8) | (data[addr+2] << 16) | (data[addr+3] << 24);
}
uint8_t Memory::load_byte(uint32_t addr) {
    if (addr >= MEM_SIZE) throw std::out_of_range("Address out of range in load_byte");
    return data[addr];
}
uint16_t Memory::load_half(uint32_t addr) {
    if (addr + 1 >= MEM_SIZE) throw std::out_of_range("Address out of range in load_half");

    return data[addr] | (data[addr + 1] << 8);
}

void Memory::store_word(uint32_t addr, uint32_t value) {
    if (addr + 3 >= MEM_SIZE) throw std::out_of_range("Address out of range");
    data[addr] = value & 0xFF;
    data[addr+1] = (value >> 8) & 0xFF;
    data[addr+2] = (value >> 16) & 0xFF;
    data[addr+3] = (value >> 24) & 0xFF;
}
void Memory::store_byte(uint32_t addr, uint8_t value) {
    if (addr >= MEM_SIZE) throw std::out_of_range("Address out of range in store_byte");
    data[addr] = value;
}
void Memory::store_half(uint32_t addr, uint16_t value) {
    if (addr + 1 >= MEM_SIZE) throw std::out_of_range("Address out of range in store_half");

    data[addr]     = value & 0xFF;
    data[addr + 1] = (value >> 8) & 0xFF;
}


