#ifndef MIPS_CPU_H
#define MIPS_CPU_H


#include <cstdint>

class CPU {
public:
    bool step();
    void run();
private:
    // CPU Registers and Stuff
    uint32_t programCounter;
    uint32_t registers[30];
    uint32_t hi;
    uint32_t lo;
    // Buffers
    uint32_t instruction;
    uint8_t instructionBuffers[4];
    int32_t signedBuffer;
    uint32_t registerBuffers[3];
    int64_t multiplicationBuffer;
    int32_t divisionBuffer;
    // Memory
    uint32_t* memory;
    // Functions
    uint32_t getRegister(uint8_t regNum);
    void setRegister(uint8_t regNum, uint32_t value);
    uint32_t getMemory(uint32_t memAddr);
    void setMemory(uint32_t memAddr, uint32_t value);
};

#endif //MIPS_CPU_H
