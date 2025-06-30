#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "memory.h"
#include "registerfile.h"
#include <string>
#include <QString>

class Simulator {
public:
    Simulator();
    void loadBinary(const std::string& filename);
    void step(); // اجرای یک پالس ساعت
    uint32_t getRegisterValue(int);
    Memory& memory1();
    uint32_t getRegisterValue(int index) const;
    uint32_t getPC() const;
    void setPC(uint32_t);
    void execute(uint32_t instruction);
    void updateRegisterLog();
    const std::vector<QString>& getClockLog() const;
    QString getOutput() const;
    void writeOutput(const QString &text);
    void provideInput(const QString &text);
    QString getInput();
    void loadFromRawData(const QByteArray& data);

private:

    Memory memory;
    RegisterFile regs;
    uint32_t PC;
    std::vector<QString> clockLog;
    int clockCycle = 0;
    uint32_t prevRegs[32];
    QString outputLog;
    QString inputBuffer;

};

#endif
