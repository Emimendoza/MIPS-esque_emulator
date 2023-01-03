#ifndef MIPS_CPU_H
#define MIPS_CPU_H

#include <cstdint>
#include <vector>
#include <string>

class CPU {
public:
    struct CPUInfo
    {
        uint8_t CPUId;
        uint8_t ROMType;
        char* ROMPath;
        uint32_t* sharedRamPtr;
    };
    bool step();
    void run();
    explicit CPU(CPUInfo cpuInfo);
    ~CPU();
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
    uint8_t** memory;
    uint32_t romSize;
    uint32_t reservedStart;
    uint8_t (CPU::*getters[4096])(uint32_t addr);
    void (CPU::*setters[4096])(uint32_t addr, const uint8_t& value);
    bool setOrNot[4096];
    // ROM
    uint8_t* CPURom;
    // Functions
    void assemble(std::vector<std::string>* assembly);
    void fatalError(const std::string& errorMsg) const;
    uint32_t getRegister(uint8_t regNum);
    void setRegister(uint8_t regNum, uint32_t value);
    void loadWord(uint32_t wordAddr, uint32_t* wordPtr);
    void setWord(uint32_t wordAddr, uint32_t word);
    // Memory Functions
    uint8_t getAddress(uint32_t memAddr);
    void setAddress(uint32_t memAddr,const uint8_t&  value);
    uint8_t getRom(uint32_t addr);
    void setRom(uint32_t addr, const uint8_t& value);
    uint8_t getRam(uint32_t addr);
    void setRam(uint32_t addr, const uint8_t& value);
    uint8_t getUninitializedRam(uint32_t addr);
    void setUninitializedRam(uint32_t addr, const uint8_t& value);
};

#endif //MIPS_CPU_H
