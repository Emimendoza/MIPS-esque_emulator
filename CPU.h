#ifndef MIPS_CPU_H
#define MIPS_CPU_H

#include <cstdint>

class CPU {
public:
    struct CPUInfo
    {
        uint8_t CPUId;
        uint8_t ROMType;
        char* ROMPath;
    };
    bool step();
    void run();
    explicit CPU(CPUInfo cpuInfo);
private:
    // Meta Info
    uint8_t CPUId;
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
    uint32_t** memory;
    uint32_t romEnd;
    uint32_t reservedStart;
    // ROM
    uint32_t* CPURom;
    // Functions
    uint32_t getRegister(uint8_t regNum);
    void setRegister(uint8_t regNum, uint32_t value);
    uint8_t getMemory(uint32_t memAddr);
    void setMemory(uint32_t memAddr, uint8_t value);
    void loadWord(uint32_t wordAddr, uint32_t* wordPtr);
    void setWord(uint32_t wordAddr, uint32_t word);
};

#endif //MIPS_CPU_H
