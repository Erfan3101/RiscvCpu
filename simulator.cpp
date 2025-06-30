#include "simulator.h"
#include <iostream>
#include <fstream>
#include <cstdint>
#include <QDebug>
Simulator::Simulator() : PC(0x1000) {
    std::fill(std::begin(prevRegs), std::end(prevRegs), 0);
}

void Simulator::step() {
    uint32_t instr = memory.load_word(PC);
    PC += 4;
    execute(instr);
    regs.commit(); // به‌روزرسانی نهایی ثبات‌ها
    clockCycle++;
    updateRegisterLog();

}

void Simulator::loadBinary(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Cannot open binary file.\n";
        return;
    }

    uint32_t addr = 0x1000; // آدرس شروع حافظه
    char byte;
    while (file.get(byte)) {
        memory.store_byte(addr++, static_cast<uint8_t>(byte));
    }
}


void Simulator::execute(uint32_t instruction) {
    uint32_t opcode = instruction & 0x7F;
    uint32_t rd = (instruction >> 7) & 0x1F;
    uint32_t funct3 = (instruction >> 12) & 0x7;
    uint32_t rs1 = (instruction >> 15) & 0x1F;
    uint32_t rs2 = (instruction >> 20) & 0x1F;
    uint32_t funct7 = (instruction >> 25) & 0x7F;

    int32_t imm_i = static_cast<int32_t>(instruction) >> 20;
    int32_t imm_s = ((int32_t)(instruction & 0xFE000000) >> 20) | ((instruction >> 7) & 0x1F);
    int32_t imm_b = ((instruction >> 31) << 12) |
                    (((instruction >> 7) & 0x1) << 11) |
                    (((instruction >> 25) & 0x3F) << 5) |
                    (((instruction >> 8) & 0xF) << 1);
    if (imm_b & 0x1000) imm_b |= 0xFFFFE000; // sign extend
    int32_t imm_u = instruction & 0xFFFFF000;
    int32_t imm_j = ((instruction >> 31) << 20) |
                    (((instruction >> 12) & 0xFF) << 12) |
                    (((instruction >> 20) & 0x1) << 11) |
                    (((instruction >> 21) & 0x3FF) << 1);
    if (imm_j & 0x100000) imm_j |= 0xFFE00000; // sign extend

    switch (opcode) {
        case 0x33: // R-type
            switch (funct3) {
                case 0x0:
                    if (funct7 == 0x00) regs.write(rd, regs.read(rs1) + regs.read(rs2)); // add
                    else if (funct7 == 0x20) regs.write(rd, regs.read(rs1) - regs.read(rs2)); // sub
                    else if (funct7 == 0x01) regs.write(rd, (int32_t)regs.read(rs1) * (int32_t)regs.read(rs2)); // mul
                    break;
                case 0x1:
                    if (funct7 == 0x00) regs.write(rd, regs.read(rs1) << (regs.read(rs2) & 0x1F)); // sll
                    else if (funct7 == 0x01) regs.write(rd, ((int64_t)(int32_t)regs.read(rs1) * (int64_t)(int32_t)regs.read(rs2)) >> 32); // mulh
                    break;
                case 0x2: regs.write(rd, ((int32_t)regs.read(rs1) < (int32_t)regs.read(rs2)) ? 1 : 0); break; // slt
                case 0x3: regs.write(rd, (regs.read(rs1) < regs.read(rs2)) ? 1 : 0); break; // sltu
                case 0x4:
                    if (funct7 == 0x00) regs.write(rd, regs.read(rs1) ^ regs.read(rs2)); // xor
                    else if (funct7 == 0x01) regs.write(rd, (int32_t)regs.read(rs1) / (int32_t)regs.read(rs2)); // div
                    break;
                case 0x5:
                    if (funct7 == 0x00) regs.write(rd, regs.read(rs1) >> (regs.read(rs2) & 0x1F)); // srl
                    else if (funct7 == 0x20) regs.write(rd, ((int32_t)regs.read(rs1)) >> (regs.read(rs2) & 0x1F)); // sra
                    break;
                case 0x6: regs.write(rd, regs.read(rs1) | regs.read(rs2)); break; // or
                case 0x7:
                    if (funct7 == 0x00) regs.write(rd, regs.read(rs1) & regs.read(rs2)); // and
                    else if (funct7 == 0x01) regs.write(rd, (int32_t)regs.read(rs1) % (int32_t)regs.read(rs2)); // rem
                    break;
            }
            break;

        case 0x13: // I-type (addi)
            switch (funct3) {
                case 0x0: regs.write(rd, regs.read(rs1) + imm_i); break; // addi
                // می‌توان دستورات ori, andi, etc را نیز اضافه کرد (اختیاری)
            }
            break;

        case 0x03: // Load
            switch (funct3) {
                case 0x2: regs.write(rd, memory.load_word(regs.read(rs1) + imm_i)); break; // lw
            }
            break;

        case 0x23: // Store
            switch (funct3) {
                case 0x2: memory.store_word(regs.read(rs1) + imm_s, regs.read(rs2)); break; // sw
            }
            break;

        case 0x63: // Branch
            switch (funct3) {
                case 0x0: if (regs.read(rs1) == regs.read(rs2)) PC += imm_b - 4; break; // beq
                case 0x1: if (regs.read(rs1) != regs.read(rs2)) PC += imm_b - 4; break; // bne
                case 0x4: if ((int32_t)regs.read(rs1) < (int32_t)regs.read(rs2)) PC += imm_b - 4; break; // blt
                case 0x5: if ((int32_t)regs.read(rs1) >= (int32_t)regs.read(rs2)) PC += imm_b - 4; break; // bge
                case 0x6: if (regs.read(rs1) < regs.read(rs2)) PC += imm_b - 4; break; // bltu
                case 0x7: if (regs.read(rs1) >= regs.read(rs2)) PC += imm_b - 4; break; // bgeu
            }
            break;

        case 0x6F: // jal
            regs.write(rd, PC);
            PC += imm_j - 4;
            break;

        case 0x67: // jalr
            regs.write(rd, PC);
            PC = (regs.read(rs1) + imm_i) & ~1;
            break;

        case 0x17: // auipc
            regs.write(rd, PC + imm_u);
            break;

        case 0x37: // lui
            regs.write(rd, imm_u);
            break;

        default:
            std::cerr << "Unknown instruction opcode: 0x" << std::hex << opcode << "\n";
            break;

    }
}
uint32_t Simulator::getRegisterValue(int idx) {
    return regs.read(idx);
}
Memory& Simulator::memory1() {
    return memory;
}
uint32_t Simulator::getRegisterValue(int index) const {
    return regs.read(index);
}

uint32_t Simulator::getPC() const {
    return PC;
}
void Simulator::setPC(uint32_t newPC) {
    PC = newPC;
}
const std::vector<QString>& Simulator::getClockLog() const {
    return clockLog;
}

void Simulator::updateRegisterLog() {
    for (int i = 0; i < 32; ++i) {
        uint32_t newVal = regs.read(i);
        if (newVal != prevRegs[i]) {
            clockLog.push_back(QString("T%1: x%2 ← %3")
                               .arg(clockCycle)
                               .arg(i)
                               .arg(newVal));
            prevRegs[i] = newVal;
        }
    }
}
QString Simulator::getOutput() const {
    return outputLog;
}

void Simulator::writeOutput(const QString &text) {
    outputLog += text;
}
void Simulator::provideInput(const QString &text) {
    inputBuffer = text;
}

QString Simulator::getInput() {
    QString temp = inputBuffer;
    inputBuffer.clear();
    return temp;
}
void Simulator::loadFromRawData(const QByteArray& data) {
    uint32_t addr = 0x1000;
    for (int i = 0; i < data.size(); ++i) {
        memory.store_byte(addr + i, static_cast<uint8_t>(data[i]));
    }
}
