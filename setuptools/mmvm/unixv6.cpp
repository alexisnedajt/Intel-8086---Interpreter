#include <iostream>
#include "common.h"
#include "os.h"
#include "runtime.h"
#include <unistd.h>


UnixV6::UnixV6() {
}

UnixV6::UnixV6(Runtime *runtime) {
	this->debug = false;
	this->runtime = runtime;
        runtime->setMemory(&memory);
}

UnixV6::UnixV6(Runtime *runtime, bool debug) {
	this->debug = debug;
	this->runtime = runtime;
        runtime->setMemory(&memory);
}


UnixV6::~UnixV6() {
}

void UnixV6::processArgs(const vector<string>& args, const vector<string>& envp) {
    vector<uint16> addr;
    uint16 args_size = args.size();
    
    uint16 len = 0;    
    for (int i = 0; i < args.size(); ++i) {
        len += args[i].length();
        len++;
    }
    
    /* padding */    
    if (len % 2) memory[--SP] = 0;

    for (int i = args_size - 1; i >= 0; --i) {
        const char *arg = args[i].c_str();
        memory[--SP] = '\0';
        for (int j = args[i].length() - 1; j >= 0; --j) {
            memory[--SP] = arg[j];
        }
        addr.push_back(SP);        
    }
    
    for (int i = 0; i < addr.size(); ++i) {
        memory[--SP] = addr[i] >> 8;
        memory[--SP] = addr[i];
    }
    
    memory[--SP] = args_size >> 8;
    memory[--SP] = args_size;
    
    if (SP % 2) memory[--SP] = 0;
}


struct symbol {
    char name[8];
    uint16 type;
    uint16 addr;
};

map<uint16, string> UnixV6::createSymbolTable(const uchar *bin, uint16 sym_size) {
    struct symbol *sym;
    map<uint16, string> symbolTable;    
    char name[9] = {0};

    string sym_name;
    if (sym_size == 0) {
        return symbolTable;
    }

    for (uint16 i = 0; i < sym_size; i += 12) {
        sym = (struct symbol *) &bin[i];
        memcpy(name, sym, 8);
        name[8] = 0;
        sym_name = string(name);
        if (sym_name != "") {
            symbolTable[sym->addr] = sym_name;
        }
    }
    
    return symbolTable;
}

int UnixV6::execSysCall() {
    int ret = 0;
    uchar systype = runtime->fetch();
    
    switch(systype) {
        case 0: { // indirect sys call
            uint16 indir_pc = runtime->fetch2();
            v6indirect(indir_pc);
            break;
            
        }
        case 1: { /* exit system call */
            v6exit();
            break;
        }
        case 4: { /* write system call */
            
            uint16 addr = runtime->fetch2();
            uint16 len  = runtime->fetch2();
            v6write(addr, len);                        
            break;
        }
        default: {
            fprintf(STDERR, "unknown sys type = %x", systype);
            break;	
        }
    }
    
    return ret;
    
}
