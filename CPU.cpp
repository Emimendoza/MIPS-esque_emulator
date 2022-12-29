#include <cstdio>
#include "CPU.h"

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
    printf("value of b: %X [%x]\n",instruction,instruction);

    // Do the instructions
    switch (instructionBuffers[0])
    {
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
            registerBuffers[0] = getRegister(instructionBuffers[2]);
            registerBuffers[1] = getRegister(instructionBuffers[3]);
            multiplicationBuffer = *(int32_t*)&registerBuffers[0];
            multiplicationBuffer *= *(int32_t*)&registerBuffers[1];
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
        case 0x0A:
            // and
            setRegister(instructionBuffers[0], getRegister(instructionBuffers[1]) & getRegister(instructionBuffers[2]));
            break;
        case 0x0B:
            // or
            setRegister(instructionBuffers[0], getRegister(instructionBuffers[1]) | getRegister(instructionBuffers[2]));
            break;
        case 0x0C:
            // andi
            registerBuffers[0] = getRegister(instructionBuffers[2]);
            setRegister(instructionBuffers[0], registerBuffers[0]&instructionBuffers[3]);
            break;
        case 0x0D:
            // ori
            registerBuffers[0] = getRegister(instructionBuffers[2]);
            setRegister(instructionBuffers[0], registerBuffers[0]|instructionBuffers[3]);
            break;
        case 0xFF:
            // end
            return true;
        default:
            // No OP or not supported OP Code
            break;
    }

    programCounter++;
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

uint8_t CPU::getMemory(uint32_t memAddr) {
    if (programCounter==0xFF)
    {
        return 0xFF;
    }
    return 0x00;
}

void CPU::setMemory(uint32_t memAddr, uint8_t value) {

}

void CPU::loadWord(uint32_t wordAddr, uint32_t* wordPtr) {
    *wordPtr = getMemory(wordAddr);
    *wordPtr = (*wordPtr << 8) | getMemory(wordAddr+1);
    *wordPtr = (*wordPtr << 8) | getMemory(wordAddr+2);
    *wordPtr = (*wordPtr << 8) | getMemory(wordAddr+3);
}

void CPU::setWord(uint32_t wordAddr, uint32_t word) {

}
