#include <cstdio>
#include <iostream>
#include "CPU.h"
#include <filesystem>
#define TWO_TO_20 1048576
// Init static member
constinit uint8_t CPU::CPUIds = 0;
// Helper Functions
uint32_t roundUp(uint32_t numToRound, uint32_t multiple)
{
    if (multiple == 0)
        return numToRound;

    uint32_t remainder = numToRound % multiple;
    if (remainder == 0)
        return numToRound;

    return numToRound + multiple - remainder;
}

// Class Functions

CPU::CPU(CPU::CPUInfo cpuInfo)
{
    CPUId = CPUIds;
    CPUIds++;
    FILE* ROMPtr;
    uint32_t rawRomSize = 0;
    switch (cpuInfo.ROMType)
    {
        case 0:
            // BIN
            rawRomSize = std::filesystem::file_size({cpuInfo.ROMPath}); // Get file size
            romSize = roundUp(rawRomSize,TWO_TO_20); // need to round to the nearest Megabyte
            CPURom = (uint8_t*)malloc(sizeof(uint8_t)* romSize); // make array for rom
            ROMPtr = fopen(cpuInfo.ROMPath,"rb"); // Open BIN File
            fread(CPURom,rawRomSize,1,ROMPtr); // Copy rom to array
            fclose(ROMPtr);
            break;
        case 255:
            // Dry run
            rawRomSize = 1;
            romSize = roundUp(rawRomSize,TWO_TO_20);
            CPURom = (uint8_t*)malloc(sizeof(uint8_t)* romSize);
            CPURom[0] = 0xFF;
            break;
        default:
            fatalError("Invalid ROMType");
    }
    for(uint32_t i = rawRomSize; i< romSize; i++)
    {
        CPURom[i] = 0x00; // Fill the rest with 0x00
    }
    for(uint32_t i = 0; i < romSize>>20; i++)
    {
        setters[i] = &CPU::setRom;
        getters[i] = &CPU::getRom;
    }
    for(uint32_t i = romSize>>20; i <4096; i++) // TODO: Change this to support the shared mem and reserved mem
    {
        setters[i] = &CPU::setUninitializedRam;
        getters[i] = &CPU::getUninitializedRam;
    }
    for(bool & set : setOrNot)
    {
        set = false;
    }
    memory = (uint8_t**)malloc(sizeof(uint8_t*)*4096);
    programCounter = 0;
    hi = 0;
    lo = 0;
    for (unsigned int & i : registers)
    {
        i = 0;
    }
}

CPU::~CPU()
{
    free(CPURom);
    for(uint32_t  i = 0; i < 4096; i++)
    {
        if(setOrNot[i])
        {
            free(memory[i]);
        }
    }
    free(memory);
}

void CPU::run()
{
    while (!step()){}
}

bool CPU::step()
{
    loadWord(programCounter, &instruction);
    // Op Code
    instructionBuffers[0] = (instruction >> 24) & 0xff;
    // Arg 1
    instructionBuffers[1] = (instruction >> 16) & 0xff;
    // Arg 2
    instructionBuffers[2] = (instruction >> 8) & 0xff;
    // Arg 3
    instructionBuffers[3] = instruction & 0xff;

    // Do the instructions
    switch (instructionBuffers[0])
    {
        case 0x00:
            // No OP
            break;
        // ARITHMETIC
        case 0x01:
            // add
            registerBuffers[0] = getRegister(instructionBuffers[2]);
            registerBuffers[1] = getRegister(instructionBuffers[3]);
            signedBuffer = *(int32_t*)&registerBuffers[0];
            signedBuffer += *(int32_t*)&registerBuffers[1];
            setRegister(instructionBuffers[1], *(uint32_t*)&signedBuffer);
            break;
        case 0x02:
            // sub
            registerBuffers[0] = getRegister(instructionBuffers[2]);
            registerBuffers[1] = getRegister(instructionBuffers[3]);
            signedBuffer = *(int32_t*)&registerBuffers[0];
            signedBuffer -= *(int32_t*)&registerBuffers[1];
            setRegister(instructionBuffers[1], *(uint32_t*)&signedBuffer);
            break;
        case 0x03:
            // addi
            registerBuffers[0] = getRegister(instructionBuffers[2]);
            signedBuffer = *(int32_t*) &registerBuffers[0];
            signedBuffer += *(int8_t*) &instructionBuffers[3];
            setRegister(instructionBuffers[1], *(uint32_t*)&signedBuffer);
            break;
        case 0x04:
            // addu
            setRegister(instructionBuffers[1], getRegister(instructionBuffers[2])+getRegister(instructionBuffers[3]));
            break;
        case 0x05:
            // subu
            setRegister(instructionBuffers[1], getRegister(instructionBuffers[2])-getRegister(instructionBuffers[3]));
            break;
        case 0x06:
            // addiu
            setRegister(instructionBuffers[1], getRegister(instructionBuffers[2])+instructionBuffers[3]);
            break;
        case 0x07:
            // mul
            setRegister(instructionBuffers[1], getRegister(instructionBuffers[2])* getRegister(instructionBuffers[3]));
            break;
        case 0x08:
            // mult
            multiplicationBuffer = getRegister(instructionBuffers[2]);
            multiplicationBuffer *= getRegister(instructionBuffers[3]);
            hi = ((*(uint64_t*)&multiplicationBuffer) >> 32) & 0xFFFFFFFF;
            lo = (*(uint64_t*)&multiplicationBuffer) & 0xFFFFFFFF;
            break;
        case 0x09:
            // div
            registerBuffers[0] = getRegister(instructionBuffers[2]);
            registerBuffers[1] = getRegister(instructionBuffers[3]);
            divisionBuffer = *(int32_t*)&registerBuffers[0];
            divisionBuffer %= *(int32_t*)&registerBuffers[1];
            hi = *(uint32_t*)&divisionBuffer;
            divisionBuffer = *(int32_t*)&registerBuffers[0];
            divisionBuffer /= *(int32_t*)&registerBuffers[1];
            lo = *(uint32_t*)&divisionBuffer;
            break;
        // LOGICAL
        case 0x10:
            // and
            setRegister(instructionBuffers[1], getRegister(instructionBuffers[1]) & getRegister(instructionBuffers[2]));
            break;
        case 0x11:
            // or
            setRegister(instructionBuffers[1], getRegister(instructionBuffers[1]) | getRegister(instructionBuffers[2]));
            break;
        case 0x12:
            // andi
            setRegister(instructionBuffers[1], getRegister(instructionBuffers[2])&instructionBuffers[3]);
            break;
        case 0x13:
            // ori
            setRegister(instructionBuffers[1], getRegister(instructionBuffers[2])|instructionBuffers[3]);
            break;
        case 0x14:
            // sll
            setRegister(instructionBuffers[1],getRegister(instructionBuffers[2]) << instructionBuffers[3]);
            break;
        case 0x15:
            // srl
            setRegister(instructionBuffers[1],getRegister(instructionBuffers[2]) >> instructionBuffers[3]);
            break;
        // DATA TRANSFER
        case 0x21:
            // lw
            loadWord(getRegister(instructionBuffers[3]) + instructionBuffers[2], &wordBuffer);
            setRegister(instructionBuffers[1], wordBuffer);
            break;
        case 0x22:
            // sw
            wordBuffer = getRegister(instructionBuffers[1]);
            setWord(getRegister(instructionBuffers[3]) + instructionBuffers[2],&wordBuffer);
            break;
        case 0x23:
            // lui
            wordBuffer = ((uint16_t*)instructionBuffers)[1];
            wordBuffer = wordBuffer << 16;
            setRegister(instructionBuffers[1], wordBuffer);
            break;
        case 0x24:
            // li
            setRegister(instructionBuffers[1],((uint16_t*)instructionBuffers)[1]);
            break;
        case 0x25:
            // mfhi
            setRegister(instructionBuffers[2], hi);
            break;
        case 0x26:
            // mflo
            setRegister(instructionBuffers[2], lo);
            break;
        case 0x27:
            // move
            setRegister(instructionBuffers[1], getRegister(instructionBuffers[2]));
            break;
        // Conditional Branching
        case 0x30:
            // beq
            if(getRegister(instructionBuffers[1])== getRegister(instructionBuffers[2]))
            {
                programCounter += instructionBuffers[3];
            }
            break;
        case 0x31:
            // bne
            if(getRegister(instructionBuffers[1]) != getRegister(instructionBuffers[2]))
            {
                programCounter += instructionBuffers[3];
            }
            break;
        case 0x32:
            // bgt
            if(getRegister(instructionBuffers[1]) > getRegister(instructionBuffers[2]))
            {
                programCounter += instructionBuffers[3];
            }
            break;
        case 0x33:
            // bge
            if(getRegister(instructionBuffers[1]) >= getRegister(instructionBuffers[2]))
            {
                programCounter += instructionBuffers[3];
            }
            break;
        case 0x34:
            // blt
            if(getRegister(instructionBuffers[1]) < getRegister(instructionBuffers[2]))
            {
                programCounter += instructionBuffers[3];
            }
            break;
        case 0x35:
            // ble
            if(getRegister(instructionBuffers[1]) <= getRegister(instructionBuffers[2]))
            {
                programCounter += instructionBuffers[3];
            }
            break;
        // Comparison
        case 0x40:
            // slt
            if (getRegister(instructionBuffers[2])< getRegister(instructionBuffers[3]))
            {
                setRegister(instructionBuffers[1],1);
            }
            else
            {
                setRegister(instructionBuffers[1],0);
            }
            break;
        case 0x41:
            // slti
            if (getRegister(instructionBuffers[2]) < instructionBuffers[3])
            {
                setRegister(instructionBuffers[1],1);
            }
            else
            {
                setRegister(instructionBuffers[1],0);
            }
            break;
        // Jumps
        case 0x50:
            // j
            programCounter = instruction & 0x00FFFFFF;
            return false;
        case 0x51:
            // jr
            programCounter = getRegister(instructionBuffers[1]);
            return false;
        case 0x52:
            // jal
            setRegister(31,programCounter+4);
            programCounter = instruction & 0x00FFFFFF;
            return false;
        case 0xFF:
            // hlt
            return true;
        default:
            // not supported OP Code
            break;
    }
    programCounter += 4;
    return false;
}

uint32_t CPU::getRegister(uint8_t regNum)
{
    if(regNum == 0)
    {
        return 0;
    }
    return registers[regNum-1];
}

void CPU::setRegister(uint8_t regNum, uint32_t value)
{
    if(regNum == 0)
    {
        return;
    }
    registers[regNum-1] = value;
}

void CPU::loadWord(uint32_t wordAddr, uint32_t* wordPtr)
{
    *wordPtr = getAddress(wordAddr);
    *wordPtr = (*wordPtr << 8) | getAddress(wordAddr+1);
    *wordPtr = (*wordPtr << 8) | getAddress(wordAddr+2);
    *wordPtr = (*wordPtr << 8) | getAddress(wordAddr+3);
}

void CPU::setWord(uint32_t wordAddr, const uint32_t* wordPtr)
{
    setAddress(wordAddr,*wordPtr&0xFF);
    setAddress(wordAddr+1,(*wordPtr>>8)&0xFF);
    setAddress(wordAddr+2,(*wordPtr>>16)&0xFF);
    setAddress(wordAddr+3,(*wordPtr>>24)&0xFF);
}

void CPU::assemble(std::vector<std::string> *assembly)
{
    // TODO: Finish assembler
}

void CPU::fatalError(const std::string& errorMsg) const
{
    std::cerr << "CPU " << unsigned(CPUId) << ": " << errorMsg << '\n';
    throw std::exception();
}

uint8_t CPU::getAddress(uint32_t memAddr)
{
    return (this->*getters[memAddr>>20])(memAddr);
}

void CPU::setAddress(uint32_t memAddr, const uint8_t& value)
{
    (this->*setters[memAddr>>20])(memAddr,value);
}

uint8_t CPU::getRom(uint32_t addr)
{
    return CPURom[addr];
}

void CPU::setRom(uint32_t addr, const uint8_t& value){}

uint8_t CPU::getRam(uint32_t addr)
{
    return memory[addr>>20][(addr & (TWO_TO_20 -1))];
}

void CPU::setRam(uint32_t addr, const uint8_t& value)
{
    memory[addr>>20][(addr & (TWO_TO_20 -1))] = value;
}

uint8_t CPU::getUninitializedRam(uint32_t addr)
{
    return 0;
}

void CPU::setUninitializedRam(uint32_t addr, const uint8_t& value) {
    memory[addr >> 20 ] = (uint8_t *) malloc(sizeof(uint8_t) << 20);
    setOrNot[addr >> 20] = true;
    setters[addr >> 20] = &CPU::setRam;
    getters[addr >> 20] = &CPU::getRam;
    memory[addr >> 20][(addr & (TWO_TO_20 -1))] = value;
}
