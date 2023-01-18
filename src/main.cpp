#include <iostream>
#include <cstring>
#include "CPU.h"

int main(int argc, char** argv) {
    CPU::CPUInfo cpuInfo{};
    cpuInfo.ROMType = 255;
    for (int i = 0; i < argc; ++i)
    {
        if ((strcmp(argv[i],"-b") == 0 or 0 ==  strcmp(argv[i],"--bin")) and i+1<argc)
        {
            cpuInfo.ROMType = 0;
            cpuInfo.ROMPath = argv[i+1];
            i++;
        }
    }
    CPU* cpu = new CPU(cpuInfo);
    cpu->run();
    delete cpu;
    return 0;
}
