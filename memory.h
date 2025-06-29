#ifndef MEMORY_H
#define MEMORY_H
#include <cstdint>
#include <vector>

class Memory
{
public:
    static const uint32_t MEM_SIZE = 65536; // 64KB

    Memory();
    uint32_t load_word(uint32_t addr);
    uint16_t load_half(uint32_t addr);
    uint8_t load_byte(uint32_t addr);

    void store_word(uint32_t addr, uint32_t value);
    void store_half(uint32_t addr, uint16_t value);
    void store_byte(uint32_t addr, uint8_t value);

private:
    std::vector<uint8_t> data;
};
#endif // MEMORY_H
