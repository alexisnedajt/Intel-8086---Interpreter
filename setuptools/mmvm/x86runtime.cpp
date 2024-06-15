#include <string.h>
#include "opcode.h"
#include "runtime.h"
#include "x86dump.h"

#define CF (((sreg & C_BIT) >> 0) & 1)
#define ZF (((sreg & Z_BIT) >> 6) & 1)
#define SF (((sreg & S_BIT) >> 7) & 1)
#define OF (((sreg & O_BIT) >> 11) & 1)
#define DF (((sreg & D_BIT) >> 10) & 1)

inline static uint16 reverse2(const uchar *buf) {
    return buf[0] | buf[1] << 8;
}


X86Runtime::X86Runtime(OSType os_type, string rootPath) : addr(0) {
    this->debug = false;
    this->os_type = os_type;
    this->rootPath = rootPath;
    this->isFork = false;
    init(os_type);

}

X86Runtime::X86Runtime(OSType os_type, bool debug, string rootPath) : addr(0) {    
    this->debug = debug;
    this->os_type = os_type;
    this->rootPath = rootPath;
    this->isFork = false;
    
    init(os_type);

}

X86Runtime::~X86Runtime() {
    delete os;
}

void X86Runtime::setIsFork(bool isFork) {
    this->isFork = isFork;
}

bool X86Runtime::getIsFork() {
    return this->isFork;
}

int X86Runtime::getPid() {
    if (os) {
        return os->getPid();
    }
    return -1;
}

void X86Runtime::copyText(const uchar *text, size_t tsize) {
    memcpy(this->text, text, tsize);
}
void X86Runtime::copyData(const uchar *data, size_t dsize) {
    memcpy(this->data, data, dsize);
}

void X86Runtime::copyRegister(const uint16* reg) {
    for (int i = 0; i < 12; ++i) {
        this->reg[i] = reg[i];
    }
}

void X86Runtime::copySregister(const uint16 sreg) {
    this->sreg = sreg;
}

void X86Runtime::copyPcInfo(const uint16 offset, const uint16 addr) {
    this->pc = &text[offset];
    this->addr = addr;
}

uint16 X86Runtime::getAddr() {
    return this->addr;
}

void X86Runtime::copyEachSize(const uint32 brksize, const uint32 datasize, const uint32 bsssize) {
    os->setBrkSize(brksize);
    os->setDataSize(datasize);
    os->setBssSize(bsssize);
}

void X86Runtime::init(OSType os_type) {
    AX = CX = DX = BX = SP = BP = SI = DI = 0;
    ES = CS = SS = DS = 0;

    switch(os_type) {
        case UNIXV6: {
            os = new X86UnixV6(this, this->debug);
            text = data = memory;
            break;
        }
        case MINIX: {
            os = new X86Minix(this, this->debug);
            text = memory;
            data = d_memory;
            break;
        }
        default: {
            fprintf(STDERR, "unsupported operation system\n");
            exit(1);
            break;
        }
    }
    os->setRootPath(rootPath);

    
    sreg = 0;
    memset(memory, 0, 65536);
    memset(d_memory, 0, 65536);
}

uchar X86Runtime::fetch() {
    addr++;
    return *pc++;
}

uint16 X86Runtime::fetch2() {
    addr += 2;
    uchar v1 = *pc++;
    return v1 | (*pc++ << 8);
}

void X86Runtime::jump(uint16 n_addr) {
    addr = n_addr;
    pc = text + n_addr;
}

uint16 X86Runtime::nextPC() {
    return pc - text;
}

void X86Runtime::push(uint16 val) {
    data[--SP] = val >> 8;
    data[--SP] = val;
}

uint16 X86Runtime::pop() {
    return (data[SP++] | data[SP++] << 8);    
}

void X86Runtime::setMemory(uchar **text_memref, uchar **data_memref) {
    *text_memref = memory;
    if (data_memref) {
        *data_memref = d_memory;
    }
}


void X86Runtime::assignRegister(uint16 **preg, uint16 **psreg) {
    *preg = reg;
    *psreg = &sreg;
}

void X86Runtime::setData(X86OpCode *opcode) {
    if (opcode->w) {
        opcode->data = reverse2(&opcode->raws[opcode->data_p]);
    } else {
        opcode->data = opcode->raws[opcode->data_p];
    }
}
void X86Runtime::setDataSW(X86OpCode *opcode) {
    if ((!opcode->s) && opcode->w) {
        opcode->data = reverse2(&opcode->raws[opcode->data_p]);
    } else if (opcode->s && opcode->w) {
        opcode->data = (char) opcode->raws[opcode->data_p];
    } else {
        opcode->data = opcode->raws[opcode->data_p];
    }
}

inline static string trimstring(const string& s) {
    if (s.length() == 0) return s;
    int b = s.find_first_not_of(" ¥t¥r¥n");
    //    int e = s.find_last_not_of(" ¥t¥r¥n");
    int e = s.find_last_not_of(" ");
    if (b == -1) // 左右両端に、スペース、タブ、改行がない。
        return "";
    return string(s, b, e - b + 1);
}

inline static uchar c_lshift(uint16 value, uint16 count) {
    return count ? ((value << (count - 1)) >> 15) & 1 : 0;
}

inline static uchar c_lshift(uchar value, uchar count) {
    return count ? ((value << (count - 1)) >> 7) & 1 : 0;
}

inline static uchar c_rshift(uint16 value, uint16 count) {
    return count ? (value >> (count - 1)) & 1 : 0;
}


void X86Runtime::writeRegister(X86OpCode *opcode, uchar regid, uint16 value) {
    if (opcode->w) {
        switch (regid) {
            case 0:
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
            {
                reg[regid] = value;
                break;
            }
            default:
                fprintf(stdout, "wrong reg number\n");
                break;
        }
    } else {
        switch(regid) {
            case 0: { // al
                AL = value;
                break;
            }
            case 1: { // cl
                CL = value;
                break;
            }
            case 2: { // dl
                DL = value;
                break;
            }
            case 3: { // bl
                BL = value;
                break;
            }
            case 4: { // ah
                AH = value;
                break;
            }
            case 5: { // ch
//                ((uchar*)&CX)[1] = value;
                CH = value;
                break;
            }
            case 6: { // dh
                DH = value;
                break;
            }
            case 7: { // bh
                BH = value;
                break;
            }
            default: {
                break;
            }
        }
    }
}


uint16 X86Runtime::readRegister(X86OpCode *opcode, uchar regid) {
    uint16 value = 0;
    
    if(opcode->w) {    
        switch(regid) {
            case 0:
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
                value = reg[regid];
                break;
            default:
                fprintf(stdout, "wrong reg number\n");
                break;
        }        
        return value;
    } else {
        switch(regid) {
            case 0: { // al
                value = AL;
                break;
            }
            case 1: { // cl
                value = CL;
                break;
            }
            case 2: { // dl
                value = DL;
                break;
            }
            case 3: { // bl
                value = BL;
                break;
            }
            case 4: { // ah
                value = AH;
                break;
            }
            case 5: { // ch
                value = CH;
                break;                
            }
            case 6: { // dh
                value = DH;
                break;                
            }
            case 7: { // bh
                value = BH;
                break;
            }
            default: {
                fprintf(stdout, "wrong reg number\n");
                break;
            }
            
        }        
        return value & 0xff;
    }
}

uint16 X86Runtime::readMemory(X86OpCode *opcode, uint16 addr) {
    if (debug) {
        addMemInfo(dump_memory(addr, opcode->w));
    }
    
    if (opcode->w) {
        return (data[addr] | data[addr+1] << 8);
    } else {
        return data[addr];
    }
}

uint16 X86Runtime::readMemoryWithSreg(X86OpCode *opcode, uint16 srg, uint16 addr) {
    if (opcode->w) {
        return data[(srg<<4) + addr] | data[(srg<<4) + addr+1] << 8;
    } else {
        return data[(srg<<4) + addr];
    }
}

uint16 X86Runtime::readMemoryWithES(X86OpCode *opcode, uint16 addr) {
    return readMemoryWithSreg(opcode, ES, addr);
}

uint16 X86Runtime::readMemoryWithDS(X86OpCode *opcode, uint16 addr) {
    return readMemoryWithSreg(opcode, DS, addr);
}

void X86Runtime::writeMemory(X86OpCode* opcode, uint16 addr, uint16 value) {
    if (debug) {
        addMemInfo(dump_memory(addr, opcode->w));
    }
    
    if (opcode->w) {
        data[addr] = value;
        data[addr+1] = value >> 8;
    } else {
        data[addr] = value & 0xff;
    }
}


void X86Runtime::writeMemoryWithSreg(X86OpCode *opcode, uint16 srg, uint16 addr, uint16 value) {
    if (opcode->w) {
        data[(srg<<4) + addr] = value;
        data[(srg<<4) + addr+1] = value >> 8;
    } else {
        data[(srg<<4) + addr] = value & 0xff;
    }
}


void X86Runtime::writeMemoryWithES(X86OpCode *opcode, uint16 addr, uint16 value) {
    writeMemoryWithSreg(opcode, ES, addr, value);
}

void X86Runtime::writeMemoryWithDS(X86OpCode *opcode, uint16 addr, uint16 value) {
    writeMemoryWithSreg(opcode, DS, addr, value);
}

void X86Runtime::setBinary(const uchar *bin, uint32 t_size, uint32 d_size, uint32 bss_size, uint32 sym_size) {

    switch(os_type) {
        case UNIXV6: {
            if (sym_size) {
                this->symbolTable = os->createSymbolTable(&bin[t_size + d_size], sym_size);
            }
            // pending
            memcpy(text, bin, t_size + d_size);
            pc = &text[0];
            addr = 0;
        }
        case MINIX: {
            memcpy(text, bin, t_size);
            memcpy(data, &bin[t_size], d_size);
            pc = &text[0];
            addr = 0;
            os->setDataSize(d_size);
            os->setBssSize(bss_size);
            os->setBrkSize(d_size + bss_size);
            break;
        }
        case END_OF_OSType: {
            fprintf(STDERR, "unsupported operation system\n");
            exit(1);
            break;
        }
        default: {
            fprintf(STDERR, "unsupported operation system\n");        
            exit(1);
            break;
        }
    }
}

int X86Runtime::systemcall() {
    return os->execSysCall();
}

void X86Runtime::showHeader() {
    fprintf(stderr, " AX   BX   CX   DX   SP   BP   SI   DI  FLAGS IP\n");
}

string X86Runtime::getRunStatus() {
    char buffer[64] = {0};
    sprintf(buffer, "%04x %04x %04x %04x %04x %04x %04x %04x %c%c%c%c %04x:",
            AX, BX, CX, DX, SP, BP, SI, DI,
            ((OF) ? 'O' : '-'), ((SF) ? 'S' : '-'), ((ZF) ? 'Z' : '-'), ((CF) ? 'C' : '-'),
            addr);
    return string(buffer);
}

void X86Runtime::setParameter(X86OpCode* opcode, uchar data) {
    opcode->mod = (data >> 6) & 3;
    opcode->reg = opcode->ext = (data >> 3) & 7;
    opcode->rom = data & 7;
    opcode->data_p++;
    resolveDisp(opcode);
}

void X86Runtime::setParameter(X86OpCode* opcode) {
    setParameter(opcode, opcode->raws[1]);
}

void X86Runtime::setZSOC(bool zf, bool sf, bool of, bool cf) {
    zf ? sreg |= Z_BIT : sreg &= ~Z_BIT;
    sf ? sreg |= S_BIT : sreg &= ~S_BIT;
    of ? sreg |= O_BIT : sreg &= ~O_BIT;
    cf ? sreg |= C_BIT : sreg &= ~C_BIT;
}

void X86Runtime::setDF(bool df) {
    df ? sreg |= D_BIT : sreg &~D_BIT;
}

void X86Runtime::resolveDisp(X86OpCode* opcode) {
    switch (opcode->mod) {
        case 0: { // rom==6 then EA = disp-high disp-low
            if (opcode->rom == 6)  {
                opcode->raws[opcode->len++] = fetch();
                opcode->raws[opcode->len++] = fetch();        
                opcode->disp = reverse2(&opcode->raws[2]);
                opcode->data_p += 2;
            } else {
                opcode->disp = 0;                
            }
            break;
        }
        case 1: { // DISP = disp-low sign-extended
            opcode->raws[opcode->len++] = fetch();
            opcode->disp = (char) opcode->raws[opcode->data_p++];
            break;
        }
        case 2: { // DISP = disp-high; disp-low
            opcode->raws[opcode->len++] = fetch();
            opcode->raws[opcode->len++] = fetch();
            opcode->disp = (int16) ((opcode->raws[opcode->data_p] | opcode->raws[opcode->data_p + 1] << 8));
            opcode->data_p += 2;
            break;
        }
        case 3: { // r/m is treated as register
            opcode->rom_reg = opcode->rom;
            break;
        }
        default:
        {
            fprintf(stderr, "wrong mod");
            break;
        }
    }
}

string X86Runtime::dump_memory(uint16 addr, uchar w) {
    char buf[32] = { 0 };

    if (debug_addr == addr) {
        return "";
    }
    
    debug_addr = addr;
    if (w) {
        sprintf(buf, "[%04x]%04x", addr, (data[addr] | data[addr+1] << 8));
    } else {
        sprintf(buf, "[%04x]%02x", addr, data[addr]);
    }
    
    return string(buf);
}

void X86Runtime::addMemInfo(string info) {
    if(memInfo.empty()) {
        if(!info.empty())
            memInfo = " ;" + info;
    } else {
        if(!info.empty())
            memInfo = memInfo + " " + info;
    }
}

void X86Runtime::showMemory(uint16 addr) {
    addMemInfo(dump_memory(addr, 1));
}

uint16 X86Runtime::getEAddress(X86OpCode* opcode) {
    /* for memory dump */
    if (debug) {
        if ((opcode->mod == 0) && (opcode->rom == 6)) {
            addMemInfo(dump_memory(opcode->disp, opcode->w));
        } else if (opcode->mod != 3) {
            switch(opcode->rom) {
                case 0: {
                    break;
                }
                case 1: {
                    break;
                }
                case 2: {
                    addMemInfo(dump_memory(BP + SI + opcode->disp, opcode->w));
                    break;
                }
                case 3: {
                    addMemInfo(dump_memory(BP + DI + opcode->disp, opcode->w));
                    break;
                }
                case 4: {
                    addMemInfo(dump_memory(SI + opcode->disp, opcode->w));
                    break;
                }
                case 5: {
                    addMemInfo(dump_memory(DI + opcode->disp, opcode->w));
                    break;
                }
                case 6: {
                    addMemInfo(dump_memory(BP + opcode->disp, opcode->w));
                    break;
                }
                case 7: {
                    addMemInfo(dump_memory(BX + opcode->disp, opcode->w));
                    break;
                }
                default: {
                    break;
                }                
            }
        }        
    }
    // mod == 00 and r/m == 110 then EA = disp-high; disp-low  
    if ((opcode->mod == 0) && (opcode->rom == 6)) {
        return opcode->disp;
    }
    // r/m represents memory
    if (opcode->mod == 3) {
        return 0;
    }
    
    uint16 addr = 0;
    switch(opcode->rom) {
        case 0: { // bx+si
            cerr << "not support rom=0 in writeEA" << endl;
            exit(1);
            break;
        }
        case 1: { // bx+di
            cerr << "not support rom=1 in writeEA" << endl;            
            exit(1);
            break;
        }
        case 2: { // bp+si
            addr = BP + SI + opcode->disp;
            break;
        }
        case 3: { // bp_di
            addr = BP + DI + opcode->disp;
            break;
        }
        case 4: { // si
            addr = SI + opcode->disp;
            break;
        }
        case 5: { // di
            addr = DI + opcode->disp;
            break;
        }
        case 6: { // bp
            addr = BP + opcode->disp;
            break;
        }
        case 7: { // bx
            addr = BX + opcode->disp;
            break;
        }
        default: {
            cerr << "unsupported rom in writeEA" << endl;
            exit(1);
            break;
        }            
    }
    
    return addr;    
}

void X86Runtime::writeEA(X86OpCode *opcode, uint16 value) {
        
    uint16 addr;
// mod == 00 and r/m == 110 then EA = disp-high; disp-low    
    if ((opcode->mod == 0) && (opcode->rom == 6)) {
        addr = getEAddress(opcode);
        if (opcode->w) {
            data[addr  ] = value;
            data[addr+1] = value >> 8;
        } else {
            data[addr]   = value;
        }
        return;
    }    
    if (opcode->mod == 3) {
        writeRegister(opcode, opcode->rom_reg, value);
        return;
    }    
    addr = getEAddress(opcode);    
    if(opcode->w) {
        data[addr] = value;
        data[addr+1] = value >> 8;
    } else {
        data[addr] = value;
    }    
}

uint16 X86Runtime::readEA(X86OpCode *opcode) {
    uint16 value;
    uint16 addr;
    if ((opcode->mod == 0) && (opcode->rom == 6)) {
        addr = getEAddress(opcode);
        if (opcode->w) {
            value = reverse2(&data[addr]);
        } else {
            value = data[addr] & 0xff;
        }
        return value;
    }
    
    if (opcode->mod == 3) {
        return readRegister(opcode, opcode->rom_reg);
    }
    
    addr = getEAddress(opcode);
    if (opcode->w) {        
        value = reverse2(&data[addr]);
    } else {
        value = data[addr] & 0xff;
    }
    
    return value;
}

void X86Runtime::writeSymbol() {
    auto it = symbolTable.find(addr);
    if (it != symbolTable.end()) {
        cerr << it->second << ":" << endl;
    }
}


int X86Runtime::run(vector<string> args, vector<string> envp) {
    os->processArgs(args, envp);
    if (debug) showHeader();
    return run();
}

int X86Runtime::run() {
    X86OpCode opcode;
    bool running = true;
    bool nextstop = false;
    string(X86Dump::*trace_f)(X86OpCode*);
    int val;
    int result = 0;
    int16 val16;
    char val8;
    uint32 loopcount = 0;
    X86Dump dump(13);
    
    while (running) {
        if (debug) {
            memInfo = "";
            debug_addr = -1;
            writeSymbol();
            if(!opcode.option.active) cerr << getRunStatus();
        }
        
        if (fixedloop) {
            if (!opcode.option.active) ++loopcount;
            if (loopcount >= numOfLoop) {
                fprintf(STDERR, "exit because of the fixed loop\n");
                exit(1);
            }
        }
        
        // for debug
        (nextstop) ? running = false : running = true;


        opcode.flush();
        trace_f = NULL;
        switch (opcode.raws[0] = fetch()) { // or
            case 0x00:
            case 0x01:
            case 0x02:
            case 0x03: { // add
                opcode.raws[1] = fetch();
                opcode.len = 2;
                opcode.opname = "add";
                opcode.w = opcode.raws[0] & 1;
                opcode.d = (opcode.raws[0] >> 1) & 1;
                setParameter(&opcode);
                uint16 rval, eaval;
                rval = readRegister(&opcode, opcode.reg);
                eaval = readEA(&opcode);
                switch(opcode.raws[0]) {
                    case 0x00: { // r/m8 + r8
                        val8 = val = (char)eaval + (char)rval;
                        writeEA(&opcode, val8);
                        setZSOC((val8 == 0), (val8 < 0), (val8 != val), rval+eaval >= 0x100);
                        break;
                    }
                    case 0x01: { // r/m16 + r16
                        val16 = val = (int16)eaval + (int16)rval;
                        writeEA(&opcode, val16);
                        setZSOC((val16 == 0), (val16 < 0), (val16 != val), rval+eaval >= 0x10000);
                        break;
                    }
                    case 0x02: { // r8 + r/m8
                        val8 = val = (char)rval + (char)eaval;
                        writeRegister(&opcode, opcode.reg, val8);
                        setZSOC((val8 == 0), (val8 < 0), (val8 != val), rval+eaval >= 0x100);
                        break;
                    }
                    case 0x03: { // r16 + r/m16
                        val16 = val = (int16)rval + (int16)eaval;
                        writeRegister(&opcode, opcode.reg, val16);
                        setZSOC((val16 == 0), (val16 < 0), (val16 != val), rval+eaval >= 0x10000);
                        break;
                    }
                    default: {
                        break;
                    }
                }
                trace_f = &X86Dump::dump_add1;
                
                break;
            }
            case 0x04:
            case 0x05: { // add (immediate to accumulator)
                opcode.raws[1] = fetch();
                opcode.len = 2;
                opcode.opname = "add";
                if ((opcode.w = opcode.raws[0] & 1)) opcode.raws[opcode.len++] = fetch();
                setData(&opcode);
                if (opcode.w) {
                    uint16 dst = readRegister(&opcode, 0); // AX
                    uint16 src = opcode.data;
                    val16 = val = (int16)dst + (int16)src;
                    writeRegister(&opcode, 0, val16); // AX
                    setZSOC((val16 == 0), (val16 < 0), (val16 != val), dst+src >= 0x10000);
                } else {
                    uchar dst = readRegister(&opcode, 0); // AL
                    uchar src = opcode.data;
                    val8 = val = (char)dst + (char)src;
                    writeRegister(&opcode, 0, val8); // AL
                    setZSOC((val8 == 0), (val8 < 0), (val8 != val), dst+src >= 0x100);
                }
                trace_f = &X86Dump::dump_add3;
             //   nextstop = true; // debug
                break;
            }
            case 0x08:
            case 0x09:
            case 0x0a:
            case 0x0b: {
                opcode.raws[1] = fetch();
                opcode.len = 2;
                opcode.w = opcode.raws[0] & 1;
                opcode.d = (opcode.raws[0] >> 1) & 1;
                opcode.opname = "or";
                setParameter(&opcode);
                
                uint16 rval, eaval;
                rval = readRegister(&opcode, opcode.reg);
                eaval = readEA(&opcode);
                if (opcode.w) {
                    val16 = val = (int16)rval | (int16)eaval;
                } else {
                    val16 = val = (char)rval | (char)eaval;
                }                
                if (opcode.d) { // to register
                    writeRegister(&opcode, opcode.reg, val16);
                } else {
                    writeEA(&opcode, val16);
                }
                if (opcode.w) {
                    setZSOC((val16 == 0), (val16 < 0), false, false);
                } else {
                    setZSOC(((val16 & 0xff) == 0), (((char)val16) < 0), false, false);
                }
                trace_f = &X86Dump::dump_or1;
                break;
            }
            case 0x0c:
            case 0x0d: { // or (immediate to accumulator)
                opcode.raws[1] = fetch();
                opcode.len = 2;
                if ((opcode.w = opcode.raws[0] & 1)) opcode.raws[opcode.len++] = fetch();
                opcode.opname = "or";
                setData(&opcode);
                if (opcode.w) {
                    uint16 src = readRegister(&opcode, 0); // AX
                    uint16 dst = opcode.data;
                    val16 = val = (int16)src | (int16)dst;
                    writeRegister(&opcode, 0, val16);
                    setZSOC((val16 == 0), (val16 < 0), false, false);
                } else {
                    uchar src = readRegister(&opcode, 0);
                    uchar dst = opcode.data;
                    val8 = val = (char)src | (char)dst;
                    writeRegister(&opcode, 0, val8);
                    setZSOC((val8 == 0), (val8 < 0), false, false);
                }
                trace_f = &X86Dump::dump_or3;                
                break;
            }
            case 0x10:
            case 0x11:
            case 0x12:
            case 0x13: { // adc (Reg/Memory with Register Either)
                opcode.raws[1] = fetch();
                opcode.len = 2;
                opcode.opname = "adc";
                opcode.w = opcode.raws[0] & 1;
                opcode.d = (opcode.raws[0] >> 1) & 1;
                setParameter(&opcode);
                uint16 rval, eaval;
                rval = readRegister(&opcode, opcode.reg);
                eaval = readEA(&opcode);
                switch(opcode.raws[0]) {
                    case 0x10: {
                        val8 = val = (char)eaval + (char)rval + ((CF) ? 1 : 0);
                        writeEA(&opcode, val8);
                        setZSOC((val8 == 0), (val8 < 0), (val8 != val), rval+eaval + ((CF) ? 1 : 0) >= 0x100);
                        break;
                    }
                    case 0x11: {
                        val16 = val = (int16)eaval + (int16)rval + (CF ? 1 : 0);
                        writeEA(&opcode, val16);
                        setZSOC((val16 == 0), (val16 < 0), (val16 != val), rval+eaval+(CF ? 1 : 0) >= 0x10000);
                        break;
                    }
                    case 0x12: {
                        val8 = val = (char)rval + (char)eaval + (CF ? 1 : 0);
                        writeRegister(&opcode, opcode.reg, val8);
                        setZSOC((val8 == 0), (val8 < 0), (val8 != val), rval+eaval+(CF ? 1 : 0) >= 0x100);
                        break;
                    }
                    case 0x13: {
                        val16 = val = (int16)rval + (int16)eaval + (CF ? 1 : 0);
                        writeRegister(&opcode, opcode.reg, val16);
                        setZSOC((val16 == 0), (val16 < 0), (val16 != val), rval+eaval+(CF ? 1 : 0) >= 0x10000);
                        break;
                    }
                    default: {
                        break;
                    }
                }
                trace_f = &X86Dump::dump_adc1;                
                break;
            }
            case 0x14:
            case 0x15: { // adc (Immediate to Accumulator)
                opcode.raws[1] = fetch();
                opcode.len = 2;
                if ((opcode.w = opcode.raws[0] & 1)) opcode.raws[opcode.len++] = fetch();                
                opcode.opname = "adc";
                setData(&opcode);                
                if (opcode.w) {
                    uint16 dst = readRegister(&opcode, 0); // AX
                    uint16 src = opcode.data;
                    val16 = val = (int16)dst + (int16)src + ((CF) ? 1 : 0);
                    writeRegister(&opcode, 0, val16);
                    setZSOC((val16 == 0), (val16 < 0), (val16 != val), dst+src+ ((CF) ? 1 : 0) >= 0x10000);
                } else {
                    uchar dst = readRegister(&opcode, 0); // AL
                    uchar src = opcode.data;
                    val8 = val = (char)dst + (char)src + ((CF) ? 1 : 0);
                    writeRegister(&opcode, 0, val8);
                    setZSOC((val8 == 0), (val8 < 0), (val8 != val), dst+src+ ((CF) ? 1 : 0) >= 0x100);
                }
                trace_f = &X86Dump::dump_adc3;
                break;
            }
            case 0x18:
            case 0x19:
            case 0x1a:
            case 0x1b: { // sbb (Reg./Memory and Register Either)
                opcode.raws[1] = fetch();
                opcode.len = 2;
                opcode.w = opcode.raws[0] & 1;
                opcode.d = (opcode.raws[0] >> 1) & 1;
                opcode.opname = "sbb";
                setParameter(&opcode);
                uchar cv = (CF ? 1 : 0);
                uint16 rval, eaval;
                rval = readRegister(&opcode, opcode.reg);
                eaval = readEA(&opcode);
                switch(opcode.raws[0]) {
                    case 0x18: { // d = 0, w = 0;
                        val8 = val = (char)eaval - (char)(rval + cv);
                        writeEA(&opcode, val8);
                        setZSOC((val8 == 0), (val8 < 0), (val8 != val), eaval < rval+cv);
                        break;
                    }
                    case 0x19: { // d = 0, w = 1
                        val16 = val = (int16)eaval - (int16)(rval + cv);
                        writeEA(&opcode, val16);
                        setZSOC((val16 == 0), (val16 < 0), (val16 != val), eaval < rval+cv);
                        break;
                    }
                    case 0x1a: { // d = 1, w = 0
                        val8 = val = (char)rval - (char)(eaval + cv);
                        writeRegister(&opcode, opcode.reg, val8);
                        setZSOC((val8 == 0), (val8 < 0), (val8 != val), rval < eaval+cv);
                        break;
                    }
                    case 0x1b: { // d = 1, w = 1
                        val16 = val = (int16)rval - (int16)(eaval + cv);
                        writeRegister(&opcode, opcode.reg, val16);                        
                        setZSOC((val16 == 0), (val16 < 0), (val16 != val), rval < eaval+cv);
                        break;
                    }
                }
                
                trace_f = &X86Dump::dump_sbb1;
                break;
            }
            case 0x1c:
            case 0x1d: { // sbb (immediate from accumulator)
                opcode.raws[1] = fetch();
                opcode.len = 2;
                if ((opcode.w = opcode.raws[0] & 1)) opcode.raws[opcode.len++] = fetch();
                opcode.opname = "sbb";
                setData(&opcode);
                uchar cv = (CF ? 1 : 0);
                if (opcode.w) {
                    uint16 dst = readRegister(&opcode, 0); // AX;
                    uint16 src = opcode.data;
                    val16 = val = (int16)dst - (int16)(src + cv);
                    writeRegister(&opcode, 0, val16);
                    setZSOC((val16 == 0), (val16 < 0), (val16 != val), dst < src+cv);
                } else {
                    uchar dst = readRegister(&opcode, 0); // AL
                    uchar src = opcode.data;
                    val8 = val = (char)dst - (char)(src + cv);
                    writeRegister(&opcode, 0, val8);
                    setZSOC((val8 == 0), (val8 < 0), (val8 != val), dst < src+cv);
                }
                trace_f = &X86Dump::dump_sbb3;
                break;
            }
            case 0x20:
            case 0x21:
            case 0x22:
            case 0x23: { // and (Reg./Memory and Register to Either)
                opcode.raws[1] = fetch();
                opcode.len = 2;
                opcode.w = opcode.raws[0] & 1;
                opcode.d = (opcode.raws[0] >> 1) & 1;
                opcode.opname = "and";
                setParameter(&opcode);
                uint16 rval, eaval;
                rval = readRegister(&opcode, opcode.reg);
                eaval = readEA(&opcode);
                switch(opcode.raws[0]) {
                    case 0x20: {
                        val8 = val = (char)rval & (char)eaval;
                        writeEA(&opcode, val8);
                        setZSOC((val8 == 0), (val8 < 0), false, false);
                        break;
                    }
                    case 0x21: {
                        val16 = val = (int16)rval & (int16)eaval;
                        writeEA(&opcode, val16);
                        setZSOC((val16 == 0), (val16 < 0), false, false);
                        break;
                    }
                    case 0x22: {
                        val8 = val = (char)rval & (char)eaval;
                        writeRegister(&opcode, opcode.reg, val8);
                        setZSOC((val8 == 0), (val8 < 0), false, false);
                        break;
                    }
                    case 0x23: {
                        val16 = val = (int16)rval & (int16)eaval;
                        writeRegister(&opcode, opcode.reg, val16);     
                        setZSOC((val16 == 0), (val16 < 0), false, false);
                        break;
                    }
                    default: {
                        break;
                    }                        
                }
                
                trace_f = &X86Dump::dump_and1;
                break;
            }
            case 0x24:
            case 0x25: { // and (Immediate to Accumulator)
                opcode.raws[1] = fetch();
                opcode.len = 2;
                if ((opcode.w = opcode.raws[0] & 1)) opcode.raws[opcode.len++] = fetch();
                opcode.opname = "and";
                setData(&opcode);
                if (opcode.w) {
                    uint16 dst = readRegister(&opcode, 0); // AX
                    uint16 src = opcode.data;
                    val16 = val = (int16)dst & (int16)src;
                    writeRegister(&opcode, 0, val16);
                    setZSOC((val16 == 0), (val16 < 0), false, false);
                } else {
                    uchar dst = readRegister(&opcode, 0); // AL
                    uchar src = opcode.data;
                    val8 = val = (char)dst & (char)src;
                    writeRegister(&opcode, 0, val8);
                    setZSOC((val8 == 0), (val8 < 0), false, false);                    
                }
                trace_f = &X86Dump::dump_and3;
                break;
            }
            case 0x28:
            case 0x29:
            case 0x2a:
            case 0x2b: { // sub(Reg/Memory and Register to Either)
                opcode.raws[1] = fetch();
                opcode.len = 2;
                opcode.w = opcode.raws[0] & 1;
                opcode.d = (opcode.raws[0] >> 1) & 1;
                opcode.opname = "sub";
                setParameter(&opcode);
                uint16 rval, eaval;
                rval = readRegister(&opcode, opcode.reg);
                eaval = readEA(&opcode);
                switch(opcode.raws[0]) {
                    case 0x28: { // r/m8 - r8
                        val8 = val = (char)eaval - (char)rval;
                        writeEA(&opcode, val8);
                        setZSOC((val8 == 0), (val8 < 0), (val8 != val), (eaval & 0xff) < (rval & 0xff));
                        break;
                    }
                    case 0x29: { // r/m16 - r16
                        val16 = val = (int16)eaval - (int16)rval;
                        writeEA(&opcode, val16);
                        setZSOC((val16 == 0), (val16 < 0), (val16 != val), eaval < rval);
                        break;
                    }
                    case 0x2a: { // r8 - r/m8
                        val8 = val = (char)rval - (char)eaval;
                        writeRegister(&opcode, opcode.reg, val8);
                        setZSOC((val8 == 0), (val8 < 0), (val8 != val), (rval & 0xff) < (eaval & 0xff));
                        break;
                    }
                    case 0x2b: { // r16 - r/m16
                        val16 = val = (int16)rval - (int16)eaval;
                        writeRegister(&opcode, opcode.reg, val16);
                        setZSOC((val16 == 0), (val16 < 0), (val16 != val), rval < eaval);
                        break;
                    }
                    default: {
                        break;
                    }
                    break;
                }
                trace_f = &X86Dump::dump_sub1;
                break;
            }
            case 0x2c:
            case 0x2d: { // sub (immediate from accumulator)
                opcode.raws[1] = fetch();
                opcode.opname = "sub";
                opcode.len = 2;
                if ((opcode.w = (opcode.raws[0] & 1))) opcode.raws[opcode.len++] = fetch();
                setData(&opcode);
                if (opcode.w) {
                    uint16 ax = AX;
                    uint16 rm = opcode.data;
                    val16 = val = (int16)ax - (int16)rm;
                    AX = val16;
                    setZSOC((val16 == 0), (val16 < 0), (val16 != val), ax < rm);
                } else {
                    uchar al = AL;
                    uchar rm = opcode.data;                    
                    val8 = val = (char)al - (char)rm;
                    AL = val8;
                    setZSOC((val8 == 0), (val8 < 0), (val8 != val), al < rm);
                }
                trace_f = &X86Dump::dump_sub3;
                break;
            }
            case 0x30:
            case 0x31:
            case 0x32:
            case 0x33: { // xor (Reg/Memory and Register to Either)
                opcode.raws[1] = fetch();
                opcode.len = 2;
                opcode.w = opcode.raws[0] & 1;
                opcode.d = (opcode.raws[0] >> 1) & 1;
                opcode.opname = "xor";
                setParameter(&opcode);
                
                uint16 rval, eaval;
                rval  = readRegister(&opcode, opcode.reg);
                eaval = readEA(&opcode);
                if (opcode.w) {
                    val16 = val = (int16)rval ^ (int16)eaval;
                } else {
                    val16 = val = (char)rval ^ (char)eaval;                    
                }
                
                if (opcode.d) { // to register
                    writeRegister(&opcode, opcode.reg, val16);
                } else {
                    writeEA(&opcode, val16);                            
                }
              
                if (opcode.w) {
                    setZSOC((val16 == 0), (val16 < 0), false, false);
                } else {
                    setZSOC(((val16 & 0xff) == 0), (((char)val16) < 0), false, false);
                }
                trace_f = &X86Dump::dump_xor1;
                break;
            }
            case 0x38:
            case 0x39:
            case 0x3a:
            case 0x3b: { // cmp (Register/Memory and Register)
                opcode.raws[1] = fetch();
                opcode.len = 2;
                opcode.w = opcode.raws[0] & 1;
                opcode.d = (opcode.raws[0] >> 1) & 1;
                opcode.opname = "cmp";
                setParameter(&opcode);
                
                uint16 rval, eaval;
                rval  = readRegister(&opcode, opcode.reg);
                eaval = readEA(&opcode);
                
                switch(opcode.raws[0]) {
                    case 0x38: { //cmp r/m8 r8
                        val8 = val = (char)eaval - (char)rval;
                        setZSOC((val8 == 0), (val8 < 0), (val8 != val), eaval < rval);
                        break;
                    }
                    case 0x39: { //cmp r/m16, r16
                        val16 = val = (int16)eaval - (int16)rval;
                        setZSOC((val16 == 0), (val16 < 0), (val16 != val), eaval < rval);
                        break;
                    }
                    case 0x3a: { //cmp r8, r/m8
                        val8 = val = (char)rval - (char)eaval;
                        setZSOC((val8 == 0), (val8 < 0), (val8 != val), rval < eaval);
                        break;
                    }
                    case 0x3b: { //cmp r16, r/m16
                        val16 = val = (int16)rval - (int16)eaval;
                        setZSOC((val16 == 0), (val16 < 0), (val16 != val), rval < eaval);
                        break;
                    }
                    default: {
                        break;
                    }
                }
                
                trace_f = &X86Dump::dump_cmp1;
                
                break;
            }
            case 0x3c:
            case 0x3d:{ // cmp (immediate with accumulator)
                opcode.raws[1] = fetch();
                opcode.len = 2;
                opcode.opname = "cmp";
                if ((opcode.w = opcode.raws[0] & 1)) opcode.raws[opcode.len++] = fetch();
                setData(&opcode);
                if (opcode.w) {
                    val16 = val = (int16)AX - (int16)opcode.data;
                    setZSOC((val16 == 0), (val16 < 0), (val16 != val), AX < opcode.data);
                } else {
                    val8 = val = (char)AL - (char)opcode.data;
                    setZSOC((val8 == 0), (val8 < 0), (val8 != val), AL < (opcode.data & 0xff));
                }
                trace_f = &X86Dump::dump_cmp3;
                break;
            }
            case 0x40:
            case 0x41:
            case 0x42:
            case 0x43:
            case 0x44:
            case 0x45:
            case 0x46:
            case 0x47: { // inc (Register)
                opcode.len = 1;
                opcode.reg = opcode.raws[0] & 7;
                opcode.w = 1;
                opcode.opname = "inc";
                uint16 dst = readRegister(&opcode, opcode.reg);
                val16 = val = (int16)dst + 1;
                writeRegister(&opcode, opcode.reg, val16);
                setZSOC((val16 == 0), (val16 < 0), (val16 != val), CF);
                trace_f = &X86Dump::dump_inc2;
                break;
            }
            case 0x48:
            case 0x49:
            case 0x4a:
            case 0x4b:
            case 0x4c:
            case 0x4d:
            case 0x4e:
            case 0x4f: { // dec (Register)
                opcode.len = 1;
                opcode.reg = opcode.raws[0] & 7;
                opcode.w = 1;
                opcode.opname = "dec";
                uint16 dst = readRegister(&opcode, opcode.reg);
                val16 = val = (int16)dst - 1;
                writeRegister(&opcode, opcode.reg, val16);
                setZSOC((val16 == 0), (val16 < 0), (val16 != val), CF);
                trace_f = &X86Dump::dump_dec2;
                break;
            }
            case 0x50:
            case 0x51:
            case 0x52:
            case 0x53:
            case 0x54:
            case 0x55:
            case 0x56:
            case 0x57: { // push (register)
                opcode.len = 1;
                opcode.w = 1;
                opcode.reg = opcode.raws[0] & 7;
                opcode.opname = "push";
                push(readRegister(&opcode, opcode.reg));
                trace_f = &X86Dump::dump_push2;
                break;
            }
            case 0x58:
            case 0x59:
            case 0x5a:
            case 0x5b:
            case 0x5c:
            case 0x5d:
            case 0x5e:
            case 0x5f: { // pop (register)
                opcode.len = 1;
                opcode.w = 1;
                opcode.reg = opcode.raws[0] & 7;
                opcode.opname = "pop";
                uint16 v = pop();
                writeRegister(&opcode, opcode.reg, v);
                trace_f = &X86Dump::dump_pop2;
                break;
            }
            case 0x72: { // jb/jnae
                opcode.raws[1] = fetch();
                opcode.len = 2;
                opcode.disp = (char) opcode.raws[1] + addr;
                opcode.opname = "jb";
                if (CF) jump(opcode.disp);
                trace_f = &X86Dump::dump_jb;
                break;
            }
            case 0x73: { // jnb/jnc
                opcode.raws[1] = fetch();
                opcode.len = 2;
                opcode.disp = (char) opcode.raws[1] + addr;
                opcode.opname = "jnb";
                if (!CF) jump(opcode.disp);
                trace_f = &X86Dump::dump_jnb;                
                break;
            }
            case 0x74: { // je/jz
                opcode.raws[1] = fetch();
                opcode.len = 2;
                opcode.disp = (char) opcode.raws[1] + addr;
                opcode.opname = "je";
                if (ZF) {
                    jump(opcode.disp);
                }
                trace_f = &X86Dump::dump_je;
                break;
            }
            case 0x75: { // jne/jnz
                opcode.raws[1] = fetch();
                opcode.len = 2;
                opcode.disp = (char) opcode.raws[1] + addr;
                opcode.opname = "jne";
                if (!ZF) {
                    jump(opcode.disp);
                }
                trace_f = &X86Dump::dump_jne;
                
                break;
            }
            case 0x76: { // jbe
                opcode.raws[1] = fetch();
                opcode.len = 2;
                opcode.disp = (char) opcode.raws[1] + addr;
                opcode.opname = "jbe";
                if (CF ||  ZF) {
                    jump(opcode.disp);
                }
                trace_f = &X86Dump::dump_jbe;
                break;
            }
            case 0x77: { // jnbe
                opcode.raws[1] = fetch();
                opcode.len = 2;
                opcode.disp = (char) opcode.raws[1] + addr;
                opcode.opname = "jnbe";                
                if(!CF && !ZF) {
                    jump(opcode.disp);
                }
                trace_f = &X86Dump::dump_jnbe;
                break;
            }
            case 0x7c: { // jl/jnge
                opcode.raws[1] = fetch();
                opcode.len = 2;
                opcode.disp = (char) opcode.raws[1] + addr;
                opcode.opname = "jl";
                if (SF != OF) {
                    jump(opcode.disp);
                }
                trace_f = &X86Dump::dump_jl;
                break;
            }
            case 0x7d: { // jnl/jge
                opcode.raws[1] = fetch();
                opcode.len = 2;
                opcode.disp = (char) opcode.raws[1] + addr;
                opcode.opname = "jnl";
                if (SF == OF) {
                    jump(opcode.disp);
                }
                trace_f = &X86Dump::dump_jnl;
                break;
            }
            case 0x7e: { // jle/jng (Junp on less or equal/ Not greater)
                opcode.raws[1] = fetch();
                opcode.len = 2;
                opcode.disp = (char) opcode.raws[1] + addr;
                opcode.opname = "jle";
                if (ZF || (SF != OF)) {
                    jump(opcode.disp);
                }
                trace_f = &X86Dump::dump_jle;
                break;
            }
            case 0x7f: { // jnle/jg (Jump on not less or equal/Greater)
                opcode.raws[1] = fetch();
                opcode.len = 2;
                opcode.disp = (char) opcode.raws[1] + addr;
                opcode.opname = "jnle";
                if (!ZF && (SF == OF)) {
                    jump(opcode.disp);
                }
                trace_f = &X86Dump::dump_jnle;
                break;
            }
            case 0x80:
            case 0x81:
            case 0x83: { // add/or/adc/sub/sbb/cmp
                opcode.raws[1] = fetch();
                opcode.raws[2] = fetch();
                opcode.len = 3;
                opcode.w = opcode.raws[0] & 1;
                opcode.s = (opcode.raws[0] >> 1) & 1;
                if (opcode.s == 0 && opcode.w == 1) { // 0x81
                    opcode.raws[opcode.len++] = fetch();
                }
                setParameter(&opcode);
                setDataSW(&opcode);
                
                switch (opcode.ext) {
                    case 0: { // add
                        opcode.opname = "add";
                        uint16 dst = readEA(&opcode);
                        uint16 src = opcode.data;  
                        uint16 tmp;
                        switch(opcode.raws[0]) {
                            case 0x80: { // 8bit
                                val8  = val = (char)dst + (char)src;
                                writeEA(&opcode, val8);
                                setZSOC((val8 == 0), (val8 < 0), (val8 != val), dst+src >= 0x100);
                                break;
                            }
                            case 0x81: { // 16bit
                                val16 = val = (int16)dst + (int16)src;
                                writeEA(&opcode, val16);
                                setZSOC((val16 == 0), (val16 < 0), (val16 != val), dst+src >= 0x10000);
                                break;
                            }
                            case 0x83: { // sign extended
                                val16 = val = (int16)dst + (char)src;
                                writeEA(&opcode, val16);
                                setZSOC((val16 == 0), (val16 < 0), (val16 != val), (dst+(tmp = (char)src)) >= 0x10000);
                                break;
                            }
                            default:
                                break;
                        }
                                                                                                
                        trace_f = &X86Dump::dump_add2;
                        break;
                    }
                    case 1: { // or
                        opcode.opname = "or";
                        uint16 dst = readEA(&opcode);
                        uint16 src = opcode.data;
                        switch(opcode.raws[0]) {
                            case 0x80: { // 8bit
                                val8 = (char)dst | (char)src;
                                writeEA(&opcode, val8);
                                setZSOC((val8 == 0), (val8 < 0), false, false);
                                break;
                            }
                            case 0x81: { // 16bit
                                val16 = (int16)dst | (int16)src;
                                writeEA(&opcode, val16);
                                setZSOC((val16 == 0), (val16 < 0), false, false);
                                break;
                            }
                            default: {
                                fprintf(STDERR, " unknown OR 0x8x\n");
                                exit(1);
                                running = false;
                            }
                        }
                        trace_f = &X86Dump::dump_or2;
                        break;
                    }
                    case 2: { // adc
                        opcode.opname = "adc";
                        uint16 dst = readEA(&opcode);
                        uint16 src = opcode.data;
                        uchar cv = (CF) ? 1 : 0;
                        switch(opcode.raws[0]) {
                            case 0x80: { // 8bit
                                val8 = val = (char)dst + (char)src + cv;
                                writeEA(&opcode, val8);
                                setZSOC((val8 == 0), (val8 < 0), (val8 != val), dst+src+cv >= 0x100);
                                break;
                            }
                            case 0x81: { // 16bit
                                val16 = val = (int16)dst + (int16)src + cv;
                                writeEA(&opcode, val16);
                                setZSOC((val16 == 0), (val16 < 0), (val16 != val), dst+src+cv >= 0x10000);
                                break;
                            }
                            case 0x83: { // sign extended
                                val16 = val = (int16)dst + (char)src + cv;
                                writeEA(&opcode, val16);
                                uint16 tmp;
                                setZSOC((val16 == 0), (val16 < 0), (val16 != val), dst+(tmp = (char)src)+cv >= 0x10000);
                                break;
                            }
                        } 
                        
                        
                        trace_f = &X86Dump::dump_adc2;
                        break;
                    }
                    case 3: { // sbb
                        opcode.opname = "sbb";
                        uint16 dst = readEA(&opcode);
                        uint16 src = opcode.data;
                        uchar cv = (CF) ? 1 : 0;
                        switch(opcode.raws[0]) {
                            case 0x80: { // 8bit
                                val8 = val = (char)dst - (char)(src+cv);
                                writeEA(&opcode, val8);
                                setZSOC((val8 == 0), (val8 < 0), (val8 != val), dst < src+cv);
                                break;
                            }
                            case 0x81: { // 16bit
                                val16 = val = (int16)dst - (int16)(src + cv);
                                writeEA(&opcode, val16);
                                setZSOC((val16 == 0), (val16 < 0), (val16 != val), dst < src+cv);
                                break;
                            }
                            case 0x83: { // sign extended
                                val16 = val = (int16)dst - (int16)((char)src + cv);
                                writeEA(&opcode, val16);
                                uint16 tmp;
                                setZSOC((val16 == 0), (val16 < 0), (val16 != val), dst < (tmp = (char)src) + cv);                                        
                                break;
                            }
                        }
                        
                        
                        trace_f = &X86Dump::dump_sbb2;
                        break;
                    }
                    case 4: { // AND (Immediate to Register/Memory)
                        opcode.opname = "and";
                        uint16 dst = readEA(&opcode);
                        uint16 src = opcode.data; 
                        switch(opcode.raws[0]) {
                            case 0x80: {
                                val8 = val = (char)dst & (char)dst;
                                writeEA(&opcode, val8);
                                setZSOC((val8 == 0), (val8 < 0), false, false);
                                break;
                            }
                            case 0x81: {
                                val16 = val = (int16)dst & (int16)src;
                                writeEA(&opcode, val16);
                                setZSOC((val16 == 0), (val16 < 0), false, false);
                                break;
                            }
                            case 0x83: {
                                val16 = val = (int16)dst & (char)src;
                                writeEA(&opcode, val16);
                                setZSOC((val16 == 0), (val16 < 0), false, false);
                                break;
                            }
                            default:
                                break;
                        }
                        trace_f = &X86Dump::dump_and2;
                        break;
                    }
                    case 5: { // sub
                        opcode.opname = "sub";                        
                        uint16 dst = readEA(&opcode);
                        uint16 src = opcode.data;
                        uint16 tmp;
                        switch(opcode.raws[0]) {
                            case 0x80: {  // 8bit
                                val8 = val = (char)dst - (char)src;
                                writeEA(&opcode, val8);
                                setZSOC((val8 == 0), (val8 < 0), (val8 != val), dst < src);
                                break;                                
                            }
                            case 0x81: { // 16bit;
                                val16 = val = (int16)dst - (int16)src;
                                writeEA(&opcode, val16);
                                setZSOC((val16 == 0), (val16 < 0), (val16 != val), dst < src);
                                break;
                            }
                            case 0x83: { // sign extended
                                val16 = val = (int16)dst - (char)src;
                                writeEA(&opcode, val16);
                                setZSOC((val16 == 0), (val16 < 0), (val16 != val), dst < (tmp = (char)src));
                                break;
                            }
                            default:
                                break;
                        }
                        trace_f = &X86Dump::dump_sub2;
                        break;
                    }
                    case 7: { // cmp
                        opcode.opname = "cmp";
                        uint16 dst = readEA(&opcode);
                        uint16 src = opcode.data;
                        uint16 tmp;
                        switch(opcode.raws[0]) {
                            case 0x80: { // 8bit
                                val8 = val = (char)dst - (char)src;
                                setZSOC((val8 == 0), (val8 < 0), (val8 != val), dst < src);
                                break;
                            }
                            case 0x81: { // 16bit
                                val16 = val = (int16)dst - (int16)src;
                                setZSOC((val16 == 0), (val16 < 0), (val16 != val), dst < src);
                                break;
                            }
                            case 0x83: { // sign extended
                                val16 = val = (int16)dst - (char)src;
                                setZSOC((val16 == 0), (val16 < 0), (val16 != val), dst < (tmp = (char)src));
                                break;
                            }
                            default: {
                                break;
                            }                                
                        }
                        trace_f = &X86Dump::dump_cmp2;
                        break;
                    }
                    default: {
                        fprintf(STDERR, " 0x8x not implemented");
                        exit(1);
                        break;
                    }
                }
                break;
            }
            case 0x84:
            case 0x85: { // test1
                opcode.raws[1] = fetch();
                opcode.len = 2;
                opcode.w = opcode.raws[0] & 1;
                opcode.opname = "test";
                setParameter(&opcode);
                
                if (opcode.w) {
                    int16 src = readEA(&opcode);
                    int16 dst = readRegister(&opcode, opcode.reg);
                    val16 = val = src & dst;
                    setZSOC((val16 == 0), (val16 < 0), false, false);
                } else {
                    char src = readEA(&opcode);
                    char dst = readRegister(&opcode, opcode.reg);
                    val8 = val = src & dst;
                    setZSOC((val8 == 0), (val8 < 0), false, false);
                }
                trace_f = &X86Dump::dump_test1;
                break;
            }
            case 0x86:
            case 0x87: { // xchg (Register/Memory with Register)
                opcode.raws[1] = fetch();
                opcode.len = 2;
                opcode.w = opcode.raws[0] & 1;
                opcode.opname = "xchg";
                setParameter(&opcode);
                
                uint16 tmp = readRegister(&opcode, opcode.reg);
                writeRegister(&opcode, opcode.reg, readEA(&opcode));
                writeEA(&opcode, tmp);
                trace_f = &X86Dump::dump_xchg1;                
                break;
            }
            case 0x88:
            case 0x89:
            case 0x8a:
            case 0x8b: { // mov (Register/Memory to/from Register)
                opcode.raws[1] = fetch();
                opcode.len = 2;
                opcode.w = opcode.raws[0] & 1;
                opcode.d = (opcode.raws[0] >> 1) & 1;
                opcode.opname = "mov";
                setParameter(&opcode);
                
                if (opcode.d) { // to register 
                    uint16 v = readEA(&opcode);
                    writeRegister(&opcode, opcode.reg, v);
                } else {         // from register 
                    uint16 v = readRegister(&opcode, opcode.reg);
                    writeEA(&opcode, v);
                }
                trace_f = &X86Dump::dump_mov1;
                break;
            }
            case 0x8d: { // lea (Load EA to Register)
                opcode.raws[1] = fetch();
                opcode.len = 2;
                opcode.w = 1;
                opcode.opname = "lea";
                setParameter(&opcode);                
                uint16 src = getEAddress(&opcode);
                writeRegister(&opcode, opcode.reg, src);                
                trace_f = &X86Dump::dump_lea;                
                break;
            }
            case 0x8f: { // pop
                opcode.raws[1] = fetch();
                opcode.len = 2;
                opcode.w = 1;
                setParameter(&opcode);
                switch (opcode.ext) {
                    case 0: {
                        opcode.opname = "pop";
                        uint16 v = pop();
                        writeEA(&opcode, v);
                        trace_f = &X86Dump::dump_pop1;
                        break;
                    }
                    default: {
                        fprintf(STDERR, "unknown in 0x8f\n");
                        exit(1);
                        break;
                    }
                }

                break;
            }
            case 0x90:
            case 0x91:
            case 0x92:
            case 0x93:
            case 0x94:
            case 0x95:                
            case 0x96:
            case 0x97: { // xchg (Register with Accumulator)
                opcode.len = 1;
                opcode.w = 1;
                opcode.opname = "xchg";
                opcode.reg = opcode.raws[0] & 7;
                uint16 tmp = AX;
                AX = readRegister(&opcode, opcode.reg);
                writeRegister(&opcode, opcode.reg, tmp);                
                trace_f = &X86Dump::dump_xchg2;
                break;
            }
            case 0x98: { // cbw (Convert Byte to Word)
                opcode.len = 1;
                opcode.opname = "cbw";
                uchar dst = readRegister(&opcode, 0);
                uint16 v = (char)dst;
                opcode.w = 1;   // w should be 0 when reading, but be 1 when writing
                writeRegister(&opcode, 0, v);
                trace_f = &X86Dump::dump_cbw;
                break;
            }
            case 0x99: { // cwd (Convert Word to Double Word)
                opcode.len = 1;
                opcode.opname = "cwd";
                val = (int16)AX;
                DX = (val >> 16);
                trace_f = &X86Dump::dump_cwd;
                break;
            }
            case 0xa0:
            case 0xa1: { // mov (Memory to Accumulator)
                opcode.raws[1] = fetch();
                opcode.raws[2] = fetch();
                opcode.len = 3;
                opcode.w = opcode.raws[0] & 1;
                opcode.opname = "mov";
                opcode.addr = reverse2(&opcode.raws[1]);
                writeRegister(&opcode, 0, readMemory(&opcode, opcode.addr));
                trace_f = &X86Dump::dump_mov4;
                
               // nextstop = true; // for debug
                break;
            }
            case 0xa2:
            case 0xa3: { // mov (Accumulator to Memory)
                opcode.raws[1] = fetch();
                opcode.raws[2] = fetch();
                opcode.len = 3;
                opcode.w = opcode.raws[0] & 1;
                opcode.addr = reverse2(&opcode.raws[1]);
                opcode.opname = "mov";
                uint16 val = readRegister(&opcode, 0);
                writeMemory(&opcode, opcode.addr, val);
                trace_f = &X86Dump::dump_mov5;
                break;
            }
            case 0xa4: { // movsb
                opcode.len = 1;
                opcode.opname = "movsb";
                uchar src;
                                
                if (opcode.option.optype == REPEAT && !CX) {
                    trace_f = &X86Dump::dump_movs;
                    break;
                }
                
                while (true) {
                    src = readMemoryWithDS(&opcode, SI);
                    writeMemoryWithES(&opcode, DI, src);                    
                    
                    if (!DF) {
                        ++SI; ++DI;
                    } else {
                        --SI; --DI;
                    }
                    // for repeat
                    if (!opcode.option.active) break;
                    if (opcode.option.optype != REPEAT) break;
                    if (!(--CX)) break;
                }
                
                trace_f = &X86Dump::dump_movs;
                break;
            }  
            case 0xa5: { // movsw
                opcode.len = 1;
                opcode.opname = "movsw";
                opcode.w = 1;
                uint16 src;
                if (opcode.option.optype == REPEAT && !CX) {
                    trace_f = &X86Dump::dump_movs;
                    break;
                }                
                while (true) {
                    src = readMemoryWithDS(&opcode, SI);
                    writeMemoryWithES(&opcode, DI, src);
                    if (!DF) {
                        SI += 2; DI += 2;
                    } else {
                        SI -= 2; DI -= 2;
                    }
                    // for repeat
                    if (!opcode.option.active) break;
                    if (opcode.option.optype != REPEAT) break;
                    if (!(--CX)) break;                    
                }
                trace_f = &X86Dump::dump_movs;
                break;
            }
            case 0xa6: { // cmpsb
                opcode.len = 1;
                opcode.w = 0;
                opcode.opname = "cmpsb";
                uchar src1, src2;
                
                if (opcode.option.optype == REPEAT && !CX) {
                    trace_f = &X86Dump::dump_cmps;
                    break;
                }
                
                while (true) {
                    src1 = readMemoryWithDS(&opcode, SI);
                    src2 = readMemoryWithES(&opcode, DI);
                    val8 = val = (char)src1 - (char)src2;                    
                    setZSOC((val8 == 0), (val8 < 0), (val8 != val), src1 < src2);
                    if (!DF) {
                        ++SI; ++DI;
                    } else {
                        --SI; --DI;
                    }
                    // for repeat
                    if (!opcode.option.active) break;
                    if (opcode.option.optype != REPEAT) break;
                    if (!(--CX)) break;
                    
                    if (opcode.option.z) {
                        if (!ZF) break;
                    } else {
                        if (ZF) break;
                    }
                    
                }
                    
                trace_f = &X86Dump::dump_cmps;
                break;
            }
            case 0xa8:
            case 0xa9: { // test (Immediate Data and Accumulator)
                opcode.raws[1] = fetch();
                opcode.len = 2;
                if ((opcode.w = opcode.raws[0] & 1)) opcode.raws[opcode.len++] = fetch();
                opcode.opname = "test";
                setData(&opcode);
                if (opcode.w) {
                    char al = AL;
                    char imdata = opcode.data;
                    val8 = val = al & imdata;
                    setZSOC((val8 == 0), (val8 < 0), false, false);
                } else {
                    int16 ax = AX;
                    int16 imdata = opcode.data;
                    val16 = val = ax & imdata;
                    setZSOC((val16 == 0), (val16 < 0), false, false);
                }
                trace_f = &X86Dump::dump_test3;               
                break;
            }
            case 0xaa: { // stosb
                opcode.len = 1;
                opcode.w = 0;
                opcode.opname = "stosb";
                uchar src;
                
                
                if (opcode.option.optype == REPEAT && !CX) {
                    trace_f = &X86Dump::dump_stos;
                    break;
                }
                
                src = readRegister(&opcode, 0); // AL                
                while (true) {
                    writeMemoryWithES(&opcode, DI, src);
                    if (!DF) {
                        ++DI;
                    } else {
                        --DI;
                    }
                    if (!opcode.option.active) break;
                    if (opcode.option.optype != REPEAT) break;
                    // for repeat
                    if (!(--CX)) break;
                    
                }
                
                trace_f = &X86Dump::dump_stos;
                break;
            }
            case 0xae: { // scasb
                opcode.len = 1;
                opcode.w = 0;
                opcode.opname = "scasb";
                uchar dst, src;
                
                if (opcode.option.optype == REPEAT && !CX) {
                    trace_f = &X86Dump::dump_scas;
                    break;
                }
                
                dst = AL;                
                while (true) {
                    src = readMemoryWithES(&opcode, DI);
                    val8 = val = (char)dst - (char)src;
                    setZSOC((val8 == 0), (val8 < 0), (val8 != val), dst < src);
                    if (!DF) {
                        ++DI;
                    } else {
                        --DI;
                    }
                    if (!opcode.option.active) break;
                    if (opcode.option.optype != REPEAT) break;
                    // for repeat
                    if (!(--CX)) break;
                    if (opcode.option.z) { // REPE/REPZ
                        if (!ZF) break;
                    } else {
                        if (ZF) break;
                    }
                }               
                trace_f = &X86Dump::dump_scas;
				break;
            }
            case 0xb0:
            case 0xb1:
            case 0xb2:
            case 0xb3:
            case 0xb4:
            case 0xb5:
            case 0xb6:
            case 0xb7:
            case 0xb8:
            case 0xb9:
            case 0xba:
            case 0xbb:
            case 0xbc:
            case 0xbd:                
            case 0xbe:
            case 0xbf: { // mov (Immediate to Register)
                opcode.raws[1] = fetch();
                opcode.len = 2;
                if ((opcode.w = (opcode.raws[0] >> 3) & 1)) {
                    opcode.raws[opcode.len++] = fetch();
                }
                opcode.opname = "mov";
                opcode.reg = opcode.raws[0] & 7;
                setData(&opcode);   // read immediate data
                writeRegister(&opcode, opcode.reg, opcode.data);
                trace_f = &X86Dump::dump_mov3;
                break;
            }
            case 0xc2: { // ret (within seg adding immed to sp)
                opcode.raws[1] = fetch();
                opcode.raws[2] = fetch();
                opcode.len = 3;
                opcode.opname = "ret";
                setData(&opcode);
                opcode.disp = opcode.data;
                uint16 v = pop();
                SP += opcode.disp;
                jump(v);
                
                trace_f = &X86Dump::dump_ret2;                
                break;
            }
            case 0xc3: { // ret (Within Segment)
                opcode.len = 1;
                opcode.opname = "ret";
                uint16 v = pop();
                jump(v);
                trace_f = &X86Dump::dump_ret1;
                break;
            }
            case 0xc6:
            case 0xc7: { // mov (Immediate to Register/Memory)
                opcode.raws[1] = fetch();
                opcode.raws[2] = fetch();
                opcode.len = 3;
                
                if ((opcode.w = opcode.raws[0] & 1)) {
                    opcode.raws[opcode.len++] = fetch();
                }
                opcode.opname = "mov";
                setParameter(&opcode);
                setData(&opcode);
                writeEA(&opcode, opcode.data);
                trace_f = &X86Dump::dump_mov2;
                break;
            }
            case 0xcd: { // Interupt                
                opcode.raws[1] = fetch();
                opcode.len = 2;
                opcode.opname = "int";
                if (debug) {
                    cerr << dump.dump_int(opcode.serialize()) << endl;
                }

                if ((result = systemcall()) != -1) {
                    running = false;
                }
                break;
            }
            case 0xd0:
            case 0xd1:
            case 0xd2:
            case 0xd3: { // shl/sal/shr/sar/rol/ror/rcl/rcr/call
                opcode.raws[1] = fetch();
                opcode.len = 2;
                opcode.w = opcode.raws[0] & 1;
                opcode.v = (opcode.raws[0] >> 1) & 1;
                setParameter(&opcode);
                
                switch(opcode.ext) {
                    case 2: { // rcl
                        opcode.opname = "rcl";
                        if (opcode.w) {
                            uint16 dst = readEA(&opcode);
                            if (opcode.v) {
                                if (CL) {
                                    val16 = val = (dst << (CL & 0x1f));
                                    uchar carry = c_lshift(dst, (CL & 0x1f));
                                    if (CF) {
                                        val16 |= 0x0001;
                                    } else {
                                        val16 &= 0xfffe;
                                    }
                                    writeEA(&opcode, val16);
                                    setZSOC(ZF, SF, OF, ((carry) ? true : false));
                                }
                            } else {
                                val16 = val = (dst << 1);
                                uchar carry = c_lshift(dst, 1);
                                if (CF) {
                                    val16 |= 0x0001;
                                } else {
                                    val16 &= 0xfffe;
                                }
                                writeEA(&opcode, val16);
                                
                                uchar lb2 = (val16 >> 15) & 1;
                                
                                setZSOC(ZF, SF, (carry ^ lb2), ((carry) ? true : false));
                            }
                        } else {
                            uchar dst = readEA(&opcode);
                            if (opcode.v) {
                                if (CF) {
                                    val8 = val = (dst << (CL & 0x1f));
                                    uchar carry = c_lshift(dst, (CL & 0x1f));
                                    if (carry) {
                                        val8 |= 0x01;
                                    } else {
                                        val8 &= 0xfe;
                                    }
                                    writeEA(&opcode, val8);
                                    setZSOC(ZF, SF, OF, ((carry) ? true : false));
                                }                                
                            } else {
                                val8 = val = (dst << 1);
                                uchar carry = c_lshift(dst, 1);
                                if (CF) {
                                    val8 |= 0x01;
                                } else {
                                    val8 &= 0xfe;
                                }
                                writeEA(&opcode, val8);
                                uchar lb2 = (val8 >> 7) & 1;
                                setZSOC(ZF, SF, (carry ^ lb2), ((carry) ? true : false));
                            }
                        }       
                        
                        trace_f = &X86Dump::dump_rcl;
                        break;
                    }
                    case 3: { // rcr
                        opcode.opname = "rcr";
                        if (opcode.w) { // 
                            uint16 dst = readEA(&opcode);
                            if (opcode.v) {
                                if (CL) {
                                    val16 = val = dst >> (CL & 0x1f);
                                    uchar carry = c_rshift(dst, CL & 0x1f);
                                    if (CF) {
                                        val16 |= 0x8000;
                                    } else {
                                        val16 &= 0x7fff;
                                    }
                                    writeEA(&opcode, val16);
                                    setZSOC(ZF, SF, OF, ((carry) ? true : false));
                                }
                            } else {
                                val16 = val = dst >> 1;
                                uchar carry = c_rshift(dst, 1);
                                if (CF) {
                                    val16 |= 0x8000;
                                } else {
                                    val16 &= 0x7fff;
                                }
                                writeEA(&opcode, val16);
                                uchar lb = (val16 >> 15) & 1;
                                uchar lb2 = (val16 >> 14) & 1;
                                setZSOC(ZF, SF, (lb ^ lb2), ((carry) ? true : false));                                
                            }
                        } else {
                            uchar dst = readEA(&opcode);
                            if (opcode.v) {
                                if (CL) {
                                    val8 = val = dst >> (CL & 0x1f);
                                    uchar carry = c_rshift(dst, CL & 0x1f);
                                    if (CF) {
                                        val8 |= 0x80;
                                    } else {
                                        val8 &= 0x7f;
                                    }
                                    writeEA(&opcode, val8);
                                    setZSOC(ZF, SF, OF, ((carry) ? true : false));
                                }
                            } else {
                                val8 = val = dst >> 1;
                                uchar carry = c_rshift(dst, 1);
                                if (CF) {
                                    val8 |= 0x80;
                                } else {
                                    val8 &= 0x7f;
                                }
                                writeEA(&opcode, val8);
                                uchar lb = (val8 >> 7) & 1;
                                uchar lb2 = (val8 >> 6) & 1;
                                setZSOC(ZF, SF, (lb ^ lb2), ((carry) ? true : false));
                            }
                        }
                        trace_f = &X86Dump::dump_rcr;
                        break;
                    }
                    case 4: { // shl/sal (Shift Logical/Arithmetic Left)
                        opcode.opname = "shl";
                        if (opcode.w) {

                            uint16 dst = readEA(&opcode);
                            if (opcode.v) {
                                if (CL) { // keep OF flag
                                    val16 = val = (dst << (CL & 0x1f));
                                    writeEA(&opcode, val16);
                                    uchar carry = c_lshift(dst, (CL & 0x1f));
                                    setZSOC((val16 == 0), (val16 < 0), ((val16 >> 15) & 1) != carry, carry);
                                }
                            } else {
                                val16 = val = (dst << 1);
                                writeEA(&opcode, val16);
                                // for making the same as 7run
                                uchar carry = c_lshift(dst, 1);
                                setZSOC((val16 == 0), (val16 < 0), ((val16 >> 15) & 1) != carry, carry);
                            }                            
                        } else {
                            uchar dst = readEA(&opcode);
                            if (opcode.v) {
                                if (CL) {
                                    val8 = val = (dst << (CL & 0x1f));;
                                    writeEA(&opcode, val8);
                                    // for making the same as 7run
                                    uchar carry = c_lshift(dst, 1);
                                    setZSOC((val8 == 0), (val8 < 0), ((val8 >> 7) & 1) != carry, carry);
                                }
                            } else {
                                val8 = val = (dst << 1);
                                writeEA(&opcode, val8);
                                uchar carry = c_lshift(dst, 1);
                                setZSOC((val8 == 0), (val8 < 0), ((val8 >> 7) & 1) != carry, carry);
                            }                            
                        }
                        
                        trace_f = &X86Dump::dump_shl;
                        break;
                    }
                    case 5: { // shr
                        opcode.opname = "shr";
                        if (opcode.w) {
                            uint16 dst = readEA(&opcode);
                            if (opcode.v) { // D3
                                if (CL) { // Nbit right shift
                                    val16 = val = dst >> (CL & 0x1f);
                                    writeEA(&opcode, val16);
                                    setZSOC((val16 == 0), (val16 < 0), OF, c_rshift(dst, (CL & 0x1f))); // OF is undefined
                                }
                            } else {  // D1 (1bit right shift)
                                val16 = val = dst >> 1;
                                writeEA(&opcode, val16);
                                setZSOC((val16 == 0), (val16 < 0), (((dst >> 15) & 1) == 1), c_rshift(dst, 1));
                            }
                        } else { 
                            uchar dst = readEA(&opcode);
                            if (opcode.v) { // D2
                                if (CL) {
                                    val8 = val = dst >> (CL >> 0x1f);
                                    writeEA(&opcode, val8);
                                    setZSOC((val16 == 0), (val16 < 0), OF, c_rshift(dst, (CL & 0x1f))); // OF is undefined
                                }
                            } else { // D0
                                val8 = val = dst >> 1;
                                writeEA(&opcode, val8);
                                setZSOC((val8 == 0), (val8 < 0), (((dst >> 7) & 1) == 1), c_rshift(dst, 1));                                
                            }
                        }
                        
                        trace_f = &X86Dump::dump_shr;
                        break;
                    }
                    case 7: { // sar
                        opcode.opname = "sar";
                        if (opcode.w) {
                            int16 dst = readEA(&opcode);
                            if (opcode.v) { // D3
                                if (CL) {
                                    val16 = val = dst >> (CL & 0x1f);
                                    writeEA(&opcode, val16);
                                    setZSOC((val16 == 0), (val16 < 0), OF, c_rshift(dst, (CL & 0x1f)));
                                }
                            } else { // D1
                                val16 = val = dst >> 1;
                                writeEA(&opcode, val16);
                                setZSOC((val16 == 0), (val16 < 0), false, c_rshift(dst, 1));
                            }                            
                        } else { 
                            char dst = readEA(&opcode);
                            if (opcode.v) { // D2
                                if (CL) {
                                    val8 = val = dst >> (CL & 0x1f);
                                    writeEA(&opcode, val8);
                                    setZSOC((val8 == 0), (val8 < 0), OF, c_rshift(dst, (CL & 0x1f)));
                                }
                            } else { // D0
                                val8 = val = dst >> 1;
                                writeEA(&opcode, val8);
                                setZSOC((val8 == 0), (val8 < 0), false, c_rshift(dst, 1));
                            }
                            
                        }
                        trace_f = &X86Dump::dump_sar;                        
                        break;                       
                    }                    
                    default: {
                        fprintf(STDERR, "0xd0~3 not implemented\n");
                        trace_f = &X86Dump::dump_undefined;
                        running =false;
                        exit(1);
                        break;
                    }
                }
                
                break;
            }
            case 0xe2: { // Loop
                opcode.raws[1] = fetch();
                opcode.len = 2;
                opcode.disp = addr + (char) opcode.raws[1];
                opcode.opname = "loop";
                if (--CX) {
                    jump(opcode.disp);
                }
                trace_f = &X86Dump::dump_loop;
                break;
            }
            case 0xe3: { // JCXZ (Jump on CZ Zero)
                opcode.raws[1] = fetch();
                opcode.len = 2;
                opcode.disp = addr + (char) opcode.raws[1];
                opcode.opname = "jcxz";
                
                if (!CX) {
                    jump(opcode.disp);
                }
                trace_f = &X86Dump::dump_jcxz;
                break;
            }
            case 0xe8: { // call (Direct within Segment)
                opcode.raws[1] = fetch();
                opcode.raws[2] = fetch();
                opcode.len = 3;
                opcode.disp = (int16) reverse2(&opcode.raws[1]) + addr;
                opcode.opname = "call";
                push(nextPC()); // push return address
                jump(opcode.disp);
                trace_f = &X86Dump::dump_call1;
                break;
            }
            case 0xe9: { // jmp (Cirect within Segment)
                opcode.raws[1] = fetch();
                opcode.raws[2] = fetch();
                opcode.len = 3;
                opcode.disp = (int16) reverse2(&opcode.raws[1]) + addr;
                opcode.opname = "jmp";
                jump(opcode.disp);
                trace_f = &X86Dump::dump_jmp1;
                break;
            }
            case 0xeb: { // jmp (Direct within Segment-short)
                opcode.raws[1] = fetch();
                opcode.len = 2;
                opcode.opname = "jmp short";
                opcode.disp = addr + (char) opcode.raws[1];
                jump(opcode.disp);
                trace_f = &X86Dump::dump_jmp2;
                break;
            }
            case 0xf2:
            case 0xf3: { // rep/repe/renne
                opcode.option.len = 1;
                opcode.option.raws[0]= opcode.raws[0];
                opcode.option.z = opcode.raws[0] & 1;
                opcode.option.optype = REPEAT;
                opcode.option.active = true;

                break;                
            }
            case 0xf6:
            case 0xf7: { //idiv/mul/neg/div
                opcode.raws[1] = fetch();
                opcode.len = 2;
                opcode.w = opcode.raws[0] & 1;
                setParameter(&opcode);
                
                switch(opcode.ext) {
                    case 0: { // test (Immediate Data and Register/Memory)
                        opcode.raws[opcode.len++] = fetch();
                        if (opcode.w) {
                            opcode.raws[opcode.len++] = fetch();
                        }
                        setData(&opcode);
                        opcode.opname = "test";
                        
                        if (opcode.w) {
                            int16 dst = readEA(&opcode);
                            int16 src = opcode.data;
                            val16 = val = dst & src;
                            setZSOC((val16 == 0), val16 < 0, false, false);
                        } else {
                            char dst = readEA(&opcode);
                            char src = opcode.data;
                            val8 = val = dst & src;
                            setZSOC((val8 == 0), val8 < 0, false, false);
                        }                        
                        trace_f = &X86Dump::dump_test2;
                        break;
                    }
                    case 2: { // not
                        opcode.opname = "not";
                        if (opcode.w) {
                            writeEA(&opcode, ~readEA(&opcode));
                        } else {
                            writeEA(&opcode, ~readEA(&opcode));
                        }
                        trace_f = &X86Dump::dump_not;
                        break;
                    }
                    case 3: { // neg
                        opcode.opname = "neg";
                        if (opcode.w) {
                            int16 src = readEA(&opcode);
                            val16 = val = -src;
                            writeEA(&opcode, val16);
                            setZSOC((val16 == 0), (val16 < 0), (val != val16), (src != 0));                            
                        } else {
                            char src = readEA(&opcode);
                            val8 = val = -src;
                            writeEA(&opcode, val8);
                            setZSOC((val8 == 0), (val8 < 0), (val != val8), (src != 0));
                        }
                        trace_f = &X86Dump::dump_neg;
                        
                        break;
                    }
                    case 4: { // mul
                        opcode.opname = "mul";
                        if (opcode.w) {
                            uint16 dst = readRegister(&opcode, 0); // AX
                            uint16 src = readEA(&opcode);
                            uint32 result = dst * src;
                            writeRegister(&opcode, 2, result >> 16); // DX
                            writeRegister(&opcode, 0, result);
                            setZSOC(ZF, SF, DX, DX);  // DX is the register which store upper bits
                        } else {
                            uchar dst = readRegister(&opcode, 0); // AL
                            uchar src = readEA(&opcode);
                            uint16 result = dst * src;
                            writeRegister(&opcode, 0, result); // AX
                            bool oc = ((result >> 8) != 0);
                            setZSOC(ZF, SF, oc, oc);
                        }
                        
                        trace_f = &X86Dump::dump_mul;
                        break;
                    }
                    case 6: { // div
                        opcode.opname = "div";
                        if (opcode.w) {
                            uint16 num = readEA(&opcode);
                            uint32 numerator = (uint32)(DX << 16 | AX);
                            uint16 quot = val = numerator / num;
                            uint16 rem = numerator % num;
                            AX = quot;
                            DX = rem;
                        } else {
                            uchar num = readEA(&opcode);
                            uchar quot = (uint16)AX / num;
                            uchar rem = (uint16)AX % num;
                            AL = quot;
                            AH = rem;
                        }
                        trace_f = &X86Dump::dump_div;
                        break;
                    }
                    case 7: { // idiv
                        opcode.opname = "idiv";
                        if (opcode.w) { // f7
                            int16 num = readEA(&opcode);
                            int   numerator = (int)(DX << 16 | AX);
                            int16 quot = val = numerator / num;
                            int16 rem = numerator % num;
                            AX = quot;
                            DX = rem;
                        } else { // f6
                            char num = readEA(&opcode);
                            char quot = val = (int16)AX / (char)num;
                            char rem = (int16)AX % (char)num;
                            AL = quot;
                            AH = rem;
                        }
                        trace_f = &X86Dump::dump_idiv;                        
                        break;
                    } 
                    default: {
                        fprintf(stdout, " 0xf6/7 is not implemented\n");
                        trace_f = &X86Dump::dump_undefined;
                        running = false;
                        exit(1);
                        break;
                    }
                    break;
                }
                break;
            }
            case 0xfc: { // cld
                opcode.len = 1;
                opcode.opname = "cld";
                setDF(false);
                trace_f = &X86Dump::dump_cld;
                break;
            }
            case 0xfd: { // std
                opcode.len = 1;
                opcode.opname = "std";
                setDF(true);
                trace_f = &X86Dump::dump_std;
                break;
            }
            case 0xfe:
            case 0xff: { // inc/dec/call/jmp/push
                opcode.raws[1] = fetch();
                opcode.len = 2;
                opcode.w = opcode.raws[0] & 1;
                setParameter(&opcode);
                switch(opcode.ext) {
                    case 0: { // inc
                        opcode.opname = "inc";
                        if (opcode.w) {
                            uint16 dst = readEA(&opcode);
                            val16 = val = (int16)dst + 1;
                            writeEA(&opcode, val16);
                            setZSOC((val16 == 0), (val16 < 0), (val != val16), CF);
                        } else {
                            uchar dst = readEA(&opcode);
                            val8 = val = (char)dst + 1;
                            writeEA(&opcode, val8);
                            setZSOC((val8 == 0), (val8 < 0), (val != val8), CF);
                        }
                        trace_f = &X86Dump::dump_inc1;
                        break;
                    }
                    case 1: { // dec (register/memory)
                        opcode.opname = "dec";
                        if (opcode.w) {
                            uint16 v = readEA(&opcode);
                            val16 = val = (int16)v - 1;
                            writeEA(&opcode, val16);
                            setZSOC((val16 == 0), (val16 < 0), (val != val16), CF);
                        } else {
                            uchar v = readEA(&opcode);
                            val8 = val = (char)v - 1;
                            writeEA(&opcode, val8);
                            setZSOC((val8 == 0), (val8 < 0), (val != val8), CF);
                        }
                        trace_f = &X86Dump::dump_dec1;
                        break;
                    }
                    case 2: { // call (indirect within Segment)
                        opcode.opname = "call";
                        uint16 dst = readEA(&opcode);
                        push(nextPC());
                        jump(dst);
                        trace_f = &X86Dump::dump_call2;
                        break;
                    }
                    case 4: { // jmp (Indirect within Segment)
                        opcode.opname = "jmp";
                        uint16 dst = readEA(&opcode);
                        jump(dst);
                        trace_f = &X86Dump::dump_jmp3;
                        break;
                    }
                    case 6: { // push(register/memory)
                        opcode.opname = "push";
                        uint16 value = readEA(&opcode);
                        push(value);
                        trace_f = &X86Dump::dump_push1;                      
                        break;
                    }
                    default: {
                        fprintf(STDERR, " 0xff is not implemented\n");
                        trace_f = &X86Dump::dump_undefined;
                        running = false;
                        exit(1);
                        break;
                    }                
                }
                
                break;
            }
            default:
                fprintf(stderr, "not implemented ");
                opcode.raws[1] = fetch();
                opcode.len = 2;
                trace_f = &X86Dump::dump_undefined;
                running = false;
                cerr << trimstring((dump.*trace_f)(opcode.serialize())) << memInfo << endl;
                exit(1);
                break;
        }
        
        if (debug && trace_f) {
            cerr << trimstring((dump.*trace_f)(opcode.serialize())) << memInfo << endl;
        } else if(trace_f) {
            opcode.option.flush();
        }
    }
    
    // for debug
    if (nextstop) {
        fprintf(STDERR, "stop because of the next stop\n");
        exit(1);
    }
    
    
    
    return result;

}
