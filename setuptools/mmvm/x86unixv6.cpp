
#include "runtime.h"
#include <unistd.h>


X86UnixV6::X86UnixV6(X86Runtime *runtime): UnixV6(runtime) {
    runtime->assignRegister(&reg, &sreg);
}

X86UnixV6::X86UnixV6(X86Runtime *runtime, bool debug): UnixV6(runtime, debug) {
    runtime->assignRegister(&reg, &sreg);
    this->debug = debug;
}

X86UnixV6::~X86UnixV6() {    
}

void X86UnixV6::clearCarry() {
    *sreg &= ~C_BIT;
}

void X86UnixV6::v6indirect(uint16 addr) {
    uint16 opcode = memory[addr] << 8 | memory[addr+1];
    if(opcode == 0xcd07) {
        addr += 2;
        uchar systype = memory[addr++];
        switch(systype) {
            case 4: { // write 
                uint16 begin_addr = memory[addr] | (memory[addr+1] << 8);
                addr += 2;
                uint16 len = memory[addr] | (memory[addr+1] << 8);
                v6write(begin_addr, len);
                break;
            }
            default:
                fprintf(STDERR, "unknown sys type indirect = %x", systype);
                break;
        }        
    } else { // cannot understand syscall
        fprintf(STDERR, "can not understand syscall %04x\n", opcode);
        exit(1);
    }
    
}

void X86UnixV6::v6write(uint16 addr, uint16 len) {
    if(debug) fprintf(STDERR, "<write(%d, 0x%04x, %d)", AX, addr, len);    
    AX = write(AX, &memory[addr], len);
    if(debug) fprintf(STDERR, " => %d>\n", AX);
    
    clearCarry();
}

void X86UnixV6::v6exit() {
    if(debug) fprintf(STDERR, "<exit(%d)>\n", AX);
    exit(AX);
}



