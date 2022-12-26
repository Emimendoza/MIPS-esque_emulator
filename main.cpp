#include <iostream>
#include "CPU.h"

int main() {
    CPU* cpu = new CPU;
    cpu->run();
    delete cpu;
    return 0;
}
