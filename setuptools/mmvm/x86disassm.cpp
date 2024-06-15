#include "disassm.h"
#include "x86dump.h"
#include <string.h>
#include <iostream>
#include "disassm.h"

struct symbol {
    char name[8];
    uint16 type;
    uint16 addr;
};

X86Disassm::X86Disassm() : buf(NULL), addr(0), v6dump(this) {
}

X86Disassm::~X86Disassm() {
    delete[] buf;
}

void X86Disassm::setBinary(const uchar *bin, uint16 size) {
    buf = new uchar[size];
    memcpy(buf, bin, size);
    p = &buf[0];
    end = &buf[size];
}

void X86Disassm::setSymbol(const uchar *bin, uint16 sym_size) {
    struct symbol *sym;
    char name[9] = {0};

    string sym_name;
    if (sym_size == 0) {
        return;
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
}

uchar X86Disassm::fetch() {
    addr++;
    return *p++;
}

void X86Disassm::fetch2(uchar *buf) {
    buf[0] = fetch();
    buf[1] = fetch();
}

uint32 X86Disassm::getAddress() {
    return addr;
}

void X86Disassm::systemcall() {
    v6dump.dumpSysCall();
}

inline static string addrToString(uint16 addr, X86OpCode opcode) {

    if (opcode.option.active) return opcode.option.adstr;

    char buf[5] = {0};
    sprintf(buf, "%04x", addr);
    return buf;
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

inline static void flush(string line, string opStr) {
    cerr << line << ": " << trimstring(opStr) << endl;
}

inline static uint16 reverse2(const uchar *buf) {
    return buf[0] | buf[1] << 8;
}

void X86Disassm::writeSymbol(uint16 addr) {
    auto it = symbolTable.find(addr);
    if (it != symbolTable.end()) {
        cout << it->second << ":\n";
    }
}

void X86Disassm::setData(X86OpCode *opcode) {
    if (opcode->w) {
        opcode->data = reverse2(&opcode->raws[opcode->data_p]);
    } else {
        opcode->data = opcode->raws[opcode->data_p];
    }
}

void X86Disassm::setDataSW(X86OpCode *opcode) {
    if ((!opcode->s) && opcode->w) {
        opcode->data = reverse2(&opcode->raws[opcode->data_p]);
    } else if (opcode->s && opcode->w) {
        opcode->data = (char) opcode->raws[opcode->data_p];
    } else {
        opcode->data = opcode->raws[opcode->data_p];
    }
}

void X86Disassm::setParameter(X86OpCode *opcode, uchar data) {
    opcode->mod = (data >> 6) & 3;
    opcode->reg = opcode->ext = (data >> 3) & 7;
    opcode->rom = data & 7;
    opcode->data_p++;
    resolveDisp(opcode);
}

void X86Disassm::resolveDisp(X86OpCode *opcode) {
    switch (opcode->mod) {
        case 0:
        {
            
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
        case 1:
        {           
            opcode->raws[opcode->len++] = fetch();
            opcode->disp = (char) opcode->raws[opcode->data_p++];
            break;
        }
        case 2:
        { 
            opcode->raws[opcode->len++] = fetch();
            opcode->raws[opcode->len++] = fetch();
            opcode->disp = (int16) ((opcode->raws[opcode->data_p] | opcode->raws[opcode->data_p + 1] << 8));
            opcode->data_p += 2;          
            break;
        }
        case 3:
        {
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

/* not used */
void X86Disassm::resolveEA(X86OpCode *opcode) {
    if ((opcode->mod == 0) && (opcode->rom == 6)) {
        opcode->raws[opcode->len++] = fetch();
        opcode->raws[opcode->len++] = fetch();        
        opcode->disp = reverse2(&opcode->raws[2]);
        opcode->data_p += 2;
        return;
    }


}
/* not used */
void X86Disassm::resolve(X86OpCode *opcode) {
    resolveDisp(opcode);
}

void X86Disassm::disassm() {
    X86OpCode opcode;
    X86Dump dump(&symbolTable);
    string adstr;

    while (true) {
        opcode.flush();
        if (p >= end) {
            break;
        }

        writeSymbol(addr);
        adstr = addrToString(addr, opcode);
        switch (opcode.raws[0] = fetch()) {

            case 0x00:
            case 0x01:
            case 0x02:
            case 0x03:
            { // add(Reg/Memory with Register to Ether)
                if (p == end) {
                    opcode.len = 1;
                    flush(adstr, dump.dump_undefined(&opcode));
                    break;
                }

                opcode.raws[1] = fetch();
                opcode.len = 2;
                opcode.opname = "add";
                opcode.w = opcode.raws[0] & 1;
                opcode.d = (opcode.raws[0] >> 1) & 1;
                setParameter(&opcode, opcode.raws[1]);       
                flush(adstr, dump.dump_add1(opcode.serialize()));
                break;
            }
            case 0x04:
            case 0x05:
            { // add (Immediate to Accumulator)
                opcode.raws[1] = fetch();
                opcode.len = 2;
                if ((opcode.w = opcode.raws[0] & 1)) {
                    opcode.raws[2] = fetch();
                    opcode.len++;
                }
                opcode.opname = "add";
                setData(&opcode);
                flush(adstr, dump.dump_add3(opcode.serialize()));
                break;
            }
            case 0x06: { // push(segment register) ; es
                opcode.len = 1;
                opcode.opname = "push";
                opcode.reg = 0;
                flush(adstr, dump.dump_push3(opcode.serialize()));
                break;
            }
            case 0x07:
            { // pop(segment register) ; es
                opcode.len = 1;
                opcode.opname = "pop";
                opcode.reg = 0;
                flush(adstr, dump.dump_pop3(opcode.serialize()));
                break;
            }
            case 0x08:
            case 0x09:
            case 0x0a:
            case 0x0b:
            { // or (Reg/Memory and Register to Ether)
                opcode.raws[1] = fetch();
                opcode.len = 2;
                opcode.w = opcode.raws[0] & 1;
                opcode.d = (opcode.raws[0] >> 1) & 1;
                opcode.opname = "or";
                setParameter(&opcode, opcode.raws[1]);
                //             flush(adstr, dump.dump_or1(&opcode));
                flush(adstr, dump.dump_or1(opcode.serialize()));
                break;
            }
            case 0x0c:
            case 0x0d: { // or(immediate to accumulator)
                opcode.raws[1] = fetch();
                opcode.len = 2;
                if ((opcode.w = opcode.raws[0] & 1)) {
                    opcode.raws[2] = fetch();
                    opcode.len++;                    
                }
                opcode.opname = "or";
                setData(&opcode);
                flush(adstr, dump.dump_or3(opcode.serialize()));
                break;
            }
            case 0x0e: { // push(segment register) ; cs
                opcode.len = 1;
                opcode.opname = "push";
                opcode.reg = 1;
                flush(adstr, dump.dump_push3(opcode.serialize()));
                break;
            }
            case 0x0f:
            { // pop(segment register) ; cs -> this is not existed
                opcode.len = 1;
                flush(adstr, dump.dump_undefined(opcode.serialize()));
                break;
            }
            case 0x10:
            case 0x11:
            case 0x12:
            case 0x13:
            { // adc
                opcode.raws[1] = fetch();
                opcode.len = 2;
                opcode.opname = "adc";
                opcode.w = opcode.raws[0] & 1;
                opcode.d = (opcode.raws[0] >> 1) & 1;
                setParameter(&opcode, opcode.raws[1]);
                flush(adstr, dump.dump_adc1(opcode.serialize()));
                break;
            }
            case 0x14:
            case 0x15: { // adc(Immediate to Accumulator)
                opcode.raws[1] = fetch();
                opcode.len = 2;
                if ((opcode.w = opcode.raws[0] & 1)) {
                    opcode.raws[2] = fetch();
                    opcode.len++;
                }
                opcode.opname = "adc";
                setData(&opcode);
                flush(adstr, dump.dump_adc3(opcode.serialize()));
                break;
            }
            case 0x16: { // push (segment register) ; ss
                opcode.len = 1;
                opcode.opname = "push";
                opcode.reg = 2;
                flush(adstr, dump.dump_push3(opcode.serialize()));
                break;
            }
            case 0x17:
            { // pop(segment register) ; ss
                opcode.len = 1;
                opcode.opname = "pop";
                opcode.reg = 2;
                flush(adstr, dump.dump_pop3(opcode.serialize()));
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
                setParameter(&opcode, opcode.raws[1]);
                flush(adstr, dump.dump_sbb1(opcode.serialize()));
                break;
            }
            case 0x1c:
            case 0x1d: { // sbb (Immediate from Accumulator)
                opcode.raws[1] = fetch();
                opcode.len = 2;
                if ((opcode.w = opcode.raws[0] & 1)) opcode.raws[opcode.len++] = fetch();
                setData(&opcode);
                opcode.opname = "sbb";
                flush(adstr, dump.dump_sbb3(opcode.serialize()));
                break;
            }
            case 0x1e: { // push (segment register) ; ds
                opcode.len = 1;
                opcode.opname = "push";
                opcode.reg = 3;
                flush(adstr, dump.dump_push3(opcode.serialize()));
                break;
            }
            case 0x1f:
            { // pop(segment register) ; ds
                opcode.len = 1;
                opcode.opname = "pop";
                opcode.reg = 3;
                flush(adstr, dump.dump_pop3(opcode.serialize()));
                break;
            }            
            case 0x20:
            case 0x21:
            case 0x22:
            case 0x23:
            { // and(Reg./Memory and Register to Either)
                opcode.raws[1] = fetch();
                opcode.len = 2;
                opcode.w = opcode.raws[0] & 1;
                opcode.d = (opcode.raws[0] >> 1) & 1;
                opcode.opname = "and";
                setParameter(&opcode, opcode.raws[1]);
                flush(adstr, dump.dump_and1(opcode.serialize()));
                break;
            }
            case 0x24:
            case 0x25: { // and (Immediate to Accumulator)
                opcode.raws[1] = fetch();
                opcode.len = 2;
                if((opcode.w = opcode.raws[0] & 1)) {
                    opcode.raws[2] = fetch();
                    opcode.len++;
                }
                opcode.opname = "and";                
                setData(&opcode);
                flush(adstr, dump.dump_and3(opcode.serialize()));
                break;
            }
            case 0x26: { // es override
                opcode.option.len = 1;
                opcode.option.raws[0] = opcode.raws[0];
                opcode.option.prefix = "es";
                opcode.option.optype = OVERWRIDE;
                opcode.option.active = true;
                opcode.option.adstr = adstr;
                break;
            }
            case 0x28:
            case 0x29:
            case 0x2a:
            case 0x2b:
            { // sub(Reg/Memory and Register to Either)
                opcode.raws[1] = fetch();
                opcode.len = 2;
                opcode.w = opcode.raws[0] & 1;
                opcode.d = (opcode.raws[0] >> 1) & 1;
                opcode.opname = "sub";
                setParameter(&opcode, opcode.raws[1]);
                flush(adstr, dump.dump_sub1(opcode.serialize()));
                break;
            }
            case 0x2c:
            case 0x2d:
            { // sub(Immediate from Accumulator)
                opcode.raws[1] = fetch();
                opcode.w = opcode.raws[0] & 1;
                opcode.len = 2;
                if (opcode.w) {
                    opcode.raws[2] = fetch();
                    opcode.len++;
                }

                setData(&opcode);
                opcode.opname = "sub";
                flush(adstr, dump.dump_sub3(opcode.serialize()));
                break;
            }
            case 0x2e: { // cs segment override
                opcode.option.len = 1;
                opcode.option.raws[0] = opcode.raws[0];
                opcode.option.prefix = "cs";
                opcode.option.optype = OVERWRIDE;
                opcode.option.active = true;
                opcode.option.adstr = adstr;
                break;
            }
            case 0x30:
            case 0x31:
            case 0x32:
            case 0x33:
            { // xor (Reg/Memory and Register to Ether)
                opcode.raws[1] = fetch();
                opcode.len = 2;
                opcode.w = opcode.raws[0] & 1;
                opcode.d = (opcode.raws[0] >> 1) & 1;
                opcode.opname = "xor";
                setParameter(&opcode, opcode.raws[1]);
                flush(adstr, dump.dump_xor1(opcode.serialize()));
                break;
            }
            case 0x34: { // xor (Immediate to Accumulator)
                opcode.raws[1] = fetch();
                opcode.len = 2;
                if((opcode.w = opcode.raws[0] & 1)) {
                    opcode.raws[2] = fetch();
                    opcode.len++;
                }
                opcode.opname = "xor";
                setData(&opcode);
                flush(adstr, dump.dump_xor3(opcode.serialize()));
                break;
            }
            case 0x36: { // ss segment override
                opcode.option.len = 1;
                opcode.option.raws[0] = opcode.raws[0];
                opcode.option.prefix = "ss";
                opcode.option.optype = OVERWRIDE;
                opcode.option.active = true;
                opcode.option.adstr = adstr;
                break;
            }
            case 0x38:
            case 0x39:
            case 0x3a:
            case 0x3b:
            { // comapre (Register/Memory and Register)
                opcode.raws[1] = fetch();
                opcode.len = 2;
                opcode.w = opcode.raws[0] & 1;
                opcode.d = (opcode.raws[0] >> 1) & 1;
                opcode.opname = "cmp";
                setParameter(&opcode, opcode.raws[1]);
                flush(adstr, dump.dump_cmp1(opcode.serialize()));
                break;
            }
            case 0x3c:
            case 0x3d:
            { // compare(Immediate with Accumulator)
                opcode.raws[1] = fetch();
                opcode.w = opcode.raws[0] & 1;
                opcode.len = 2;
                if (opcode.w) {
                    opcode.raws[2] = fetch();
                    opcode.len++;
                }
                setData(&opcode);
                opcode.opname = "cmp";
                flush(adstr, dump.dump_cmp3(opcode.serialize()));
                break;
            }
            case 0x40:
            case 0x41:
            case 0x42:
            case 0x43:
            case 0x44:
            case 0x45:
            case 0x46:
            case 0x47:
            { // inc (Register)
                opcode.len = 1;
                opcode.reg = opcode.raws[0] & 7;
                opcode.opname = "inc";
                flush(adstr, dump.dump_inc2(opcode.serialize()));
                break;
            }
            case 0x48:
            case 0x49:
            case 0x4a:
            case 0x4b:
            case 0x4c:
            case 0x4d:
            case 0x4e:
            case 0x4f:
            { // dec (Register)
                opcode.len = 1;
                opcode.reg = opcode.raws[0] & 7;
                opcode.opname = "dec";
                flush(adstr, dump.dump_dec2(opcode.serialize()));
                break;
            }
            case 0x50:
            case 0x51:
            case 0x52:
            case 0x53:
            case 0x54:
            case 0x55:
            case 0x56:
            case 0x57:
            { // push(2)
                opcode.len = 1;
                opcode.w = 1;
                opcode.reg = opcode.raws[0] & 7;
                opcode.opname = "push";
                flush(adstr, dump.dump_push2(opcode.serialize()));
                break;
            }
            case 0x58:
            case 0x59:
            case 0x5a:
            case 0x5b:
            case 0x5c:
            case 0x5d:
            case 0x5e:
            case 0x5f:
            { // pop(2)
                opcode.len = 1;
                opcode.w = 1;
                opcode.reg = opcode.raws[0] & 7;
                opcode.opname = "pop";
                flush(adstr, dump.dump_pop2(opcode.serialize()));
                break;
            }
            case 0x70:
            { // jo
                opcode.raws[1] = fetch();
                opcode.len = 2;
                opcode.disp = (char) opcode.raws[1] + addr;
                opcode.opname = "jo";
                flush(adstr, dump.dump_jo(opcode.serialize()));
                break;
            }
            case 0x72:
            { // jb/jnae
                opcode.raws[1] = fetch();
                opcode.len = 2;
                opcode.disp = (char) opcode.raws[1] + addr;
                opcode.opname = "jb";
                flush(adstr, dump.dump_jb(opcode.serialize()));
                break;
            }
            case 0x73:
            { // jnb
                opcode.raws[1] = fetch();
                opcode.len = 2;
                opcode.disp = (char) opcode.raws[1] + addr;
                opcode.opname = "jnb";
                flush(adstr, dump.dump_jnb(opcode.serialize()));
                break;
            }
            case 0x74:
            { // je/jz
                opcode.raws[1] = fetch();
                opcode.len = 2;
                opcode.disp = (char) opcode.raws[1] + addr;
                opcode.opname = "je";
                flush(adstr, dump.dump_je(opcode.serialize()));
                break;
            }
            case 0x75:
            { // jne/jnz
                opcode.raws[1] = fetch();
                opcode.len = 2;
                opcode.disp = (char) opcode.raws[1] + addr;
                opcode.opname = "jne";
                flush(adstr, dump.dump_jne(opcode.serialize()));
                break;
            }
            case 0x76:
            { // jbe
                opcode.raws[1] = fetch();
                opcode.len = 2;
                opcode.disp = (char) opcode.raws[1] + addr;
                opcode.opname = "jbe";
                flush(adstr, dump.dump_jbe(opcode.serialize()));
                break;
            }
            case 0x77:
            { // jnbe
                opcode.raws[1] = fetch();
                opcode.len = 2;
                opcode.disp = (char) opcode.raws[1] + addr;
                opcode.opname = "jnbe";
                flush(adstr, dump.dump_jnbe(opcode.serialize()));
                break;
            }
            case 0x78: { // js 
                opcode.raws[1] = fetch();
                opcode.len = 2;
                opcode.disp = (char) opcode.raws[1] + addr;
                opcode.opname = "js";
                flush(adstr, dump.dump_js(opcode.serialize()));
                break;
            }
            case 0x7c:
            { // jl
                opcode.raws[1] = fetch();
                opcode.len = 2;
                opcode.disp = (char) opcode.raws[1] + addr;
                opcode.opname = "jl";
                flush(adstr, dump.dump_jl(opcode.serialize()));
                break;
            }
            case 0x7d:
            { // jnl/jge
                opcode.raws[1] = fetch();
                opcode.len = 2;
                opcode.disp = (char) opcode.raws[1] + addr;
                opcode.opname = "jnl";
                flush(adstr, dump.dump_jnl(opcode.serialize()));
                break;
            }
            case 0x7e:
            { // jle/jng
                opcode.raws[1] = fetch();
                opcode.len = 2;
                opcode.disp = (char) opcode.raws[1] + addr;
                opcode.opname = "jle";
                flush(adstr, dump.dump_jle(opcode.serialize()));
                break;
            }
            case 0x7f:
            {
                opcode.raws[1] = fetch();
                opcode.len = 2;
                opcode.disp = (char) opcode.raws[1] + addr;
                opcode.opname = "jnle";
                flush(adstr, dump.dump_jnle(opcode.serialize()));
                break;
            }
            case 0x81:
            { // add/adc/sub/sbb/cmp
                opcode.raws[1] = fetch();
                opcode.raws[2] = fetch();
                opcode.raws[3] = fetch();
                opcode.s = 0;
                opcode.w = 1;
                opcode.len = 4;
                setParameter(&opcode, opcode.raws[1]);
                setDataSW(&opcode);
                switch (opcode.ext) {
                    case 0:
                    { // add
                        opcode.opname = "add";
                        flush(adstr, dump.dump_add2(opcode.serialize()));
                        break;
                    }
                    case 1: { // or
                        opcode.opname = "or";
                        flush(adstr, dump.dump_or2(opcode.serialize()));
                        break;
                    }
                    case 2:
                    { // adc
                        opcode.opname = "adc";
                        fprintf(stdout, "adc not implemented\n");
                        break;
                    }
                    case 4:
                    { // and(Immediate to Register/Memory)
                        opcode.opname = "and";
                        flush(adstr, dump.dump_and2(opcode.serialize()));
                        break;
                    }
                    case 5:
                    { // sub
                        opcode.opname = "sub";
                        flush(adstr, dump.dump_sub2(opcode.serialize()));
                        break;
                    }
                    case 3:
                    { // sbb
                        opcode.opname = "sbb";
                        flush(adstr, dump.dump_sbb2(opcode.serialize()));
//                        fprintf(stdout, "ssb not implemented\n");
                        break;
                    }
                    case 7:
                    { // cmp
                        opcode.opname = "cmp";
                        flush(adstr, dump.dump_cmp2(opcode.serialize()));
                        break;
                    }
                    default:
                        fprintf(stdout, "0x81 not implemented\n");
                        break;
                }

                break;
            }
            case 0x80:
            case 0x82:
            case 0x83:
            { // add/adc/sub/sbb/cmp/and
                opcode.raws[1] = fetch();
                opcode.raws[2] = fetch();
                opcode.len = 3;
                opcode.w = opcode.raws[0] & 1;
                opcode.s = (opcode.raws[0] >> 1) & 1;
                setParameter(&opcode, opcode.raws[1]);
                setDataSW(&opcode);
                switch (opcode.ext) {
                    case 0:
                    { // add
                        opcode.opname = "add";
                        flush(adstr, dump.dump_add2(opcode.serialize()));
                        break;
                    }
                    case 2:
                    { // adc
                        opcode.opname = "adc";
                        flush(adstr, dump.dump_adc2(opcode.serialize()));
                        break;
                    }
                    case 4:
                    { // and
                        opcode.opname = "and";
                        flush(adstr, dump.dump_and2(opcode.serialize()));
                        break;
                    }
                    case 5:
                    { // sub
                        opcode.opname = "sub";
                        flush(adstr, dump.dump_sub2(opcode.serialize()));
                        break;
                    }
                    case 3:
                    { // ssb
                        opcode.opname = "sbb";
                        flush(adstr, dump.dump_sbb2(opcode.serialize()));
                        break;
                    }
                    case 7:
                    { // cmp
                        opcode.opname = "cmp";
                        flush(adstr, dump.dump_cmp2(opcode.serialize()));
                        break;
                    }
                    default:
                        fprintf(stderr, " 0x80~83 not implemented\n");
                        break;
                }

                break;
            }
            case 0x84:
            case 0x85:
            { // test
                opcode.raws[1] = fetch();
                opcode.len = 2;
                opcode.w = opcode.raws[0] & 1;
                opcode.opname = "test";
                setParameter(&opcode, opcode.raws[1]);
                flush(adstr, dump.dump_test1(opcode.serialize()));
                break;
            }
            case 0x86:
            case 0x87:
            { // xchg
                opcode.raws[1] = fetch();
                opcode.len = 2;
                opcode.w = opcode.raws[0] & 1;
                opcode.opname = "xchg";
                setParameter(&opcode, opcode.raws[1]);
                flush(adstr, dump.dump_xchg1(opcode.serialize()));
                break;
            }
            case 0x88:
            case 0x89:
            case 0x8a:
            case 0x8b:
            { // mov (Register/Memory to/from Register)
                opcode.raws[1] = fetch();
                opcode.len = 2;
                opcode.w = opcode.raws[0] & 1;
                opcode.d = (opcode.raws[0] >> 1) & 1;
                opcode.opname = "mov";
                setParameter(&opcode, opcode.raws[1]);
                flush(adstr, dump.dump_mov1(opcode.serialize()));
                break;
            }
            case 0x8c:
            { // mov (Register/Memory to Segment Register)
                opcode.raws[1] = fetch();
                opcode.len = 2;                
                opcode.opname = "mov";
                setParameter(&opcode, opcode.raws[1]);
                opcode.reg &= 3; // 0reg
                flush(adstr, dump.dump_mov7(opcode.serialize()));
                break;
            }
            case 0x8d:
            { // lea (Load EA to Register)
                opcode.raws[1] = fetch();
                opcode.len = 2;
                opcode.opname = "lea";
                setParameter(&opcode, opcode.raws[1]);
                flush(adstr, dump.dump_lea(opcode.serialize()));
                break;
            }
            case 0x8e:
            { // mov (Segment Register to Register/Memory)
                opcode.raws[1] = fetch();
                opcode.len = 2;
                opcode.opname = "mov";
                setParameter(&opcode, opcode.raws[1]);
                opcode.reg &= 3;
                flush(adstr, dump.dump_mov6(opcode.serialize()));
                break;
            }
            case 0x8f: {
                opcode.raws[1] = fetch();
                opcode.len = 2;
                opcode.w = 1;
                setParameter(&opcode, opcode.raws[1]);
                switch(opcode.ext) {
                    case 0: { // pop(register/memory)
                        opcode.opname = "pop";
                        flush(adstr, dump.dump_pop1(opcode.serialize()));
                        break;
                    }
                    default: {
                        fprintf(stdout, "0x8f is not implemented\n");
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
                opcode.reg = opcode.raws[0] & 7;
                opcode.opname = "xchg";
                flush(adstr, dump.dump_xchg2(opcode.serialize()));
                break;
            }
            case 0x98:
            { // cbw (Convert Byte to Word
                opcode.len = 1;
                opcode.opname = "cbw";
                flush(adstr, dump.dump_cbw(opcode.serialize()));
                break;
            }
            case 0x99:
            { // cwd (Convert Word to Double Word)
                opcode.len = 1;
                opcode.opname = "cwd";
                flush(adstr, dump.dump_cwd(opcode.serialize()));
                break;
            }
            case 0x9c: {
                opcode.len = 1;
                opcode.opname = "pushf";
                flush(adstr, dump.dump_pushf(opcode.serialize()));
                break;
            }
            case 0x9d: {
                opcode.len = 1;
                opcode.opname = "popf";
                flush(adstr, dump.dump_popf(opcode.serialize()));
                break;
            }            
            case 0xa0:
            case 0xa1:
            { // mov (Memory to Accumulator)
                opcode.raws[1] = fetch();
                opcode.raws[2] = fetch();
                opcode.len = 3;
                opcode.w = opcode.raws[0] & 1;
                opcode.addr = reverse2(&opcode.raws[1]);
                opcode.opname = "mov";
                flush(adstr, dump.dump_mov4(opcode.serialize()));
                break;
            }
            case 0xa2:
            case 0xa3:
            { // mov (Accumulator to Memory)
                opcode.raws[1] = fetch();
                opcode.raws[2] = fetch();
                opcode.len = 3;
                opcode.w = opcode.raws[0] & 1;
                opcode.addr = reverse2(&opcode.raws[1]);
                opcode.opname = "mov";
                flush(adstr, dump.dump_mov5(opcode.serialize()));
                break;
            }
            case 0xa4:
            case 0xa5:
            { // movs
                opcode.len = 1;
                if ((opcode.w = opcode.raws[0] & 1)) {
                    opcode.opname = "movsw";
                } else {
                    opcode.opname = "movsb";
                }

                flush(adstr, dump.dump_movs(opcode.serialize()));
                break;
            }
            case 0xa6:
            case 0xa7:
            { // cmps
                opcode.len = 1;
                (opcode.w = opcode.raws[0] & 1) ?
                        opcode.opname = "cmpsw" : opcode.opname = "cmpsb";
                flush(adstr, dump.dump_cmps(opcode.serialize()));
                break;
            }
            case 0xa8:
            case 0xa9:
            { // test(immediate to accumulator)
                opcode.raws[1] = fetch();
                opcode.len = 2;
                if ((opcode.w = opcode.raws[0] & 1)) {
                    opcode.raws[2] = fetch();
                    opcode.len++;
                }
                opcode.opname = "test";
                setData(&opcode);
                flush(adstr, dump.dump_test3(opcode.serialize()));
                break;
            }
            case 0xaa:
            case 0xab:
            { // stos
                opcode.len = 1;
                (opcode.w = opcode.raws[0] & 1) ?
                        opcode.opname = "stosw" : opcode.opname = "stosb";
                flush(adstr, dump.dump_stos(opcode.serialize()));

                break;
            }
            case 0xac:
            case 0xad:
            { // lods(b)(Load Byte/Wd to AL/AX))
                opcode.len = 1;
                //              opcode.w = opcode.raws[0] & 1;
                //                opcode.opname = "lods";

                (opcode.w = opcode.raws[0] & 1) ?
                        opcode.opname = "lodsw" : opcode.opname = "lodsb";

                flush(adstr, dump.dump_lods(opcode.serialize()));
                break;
            }
            case 0xae:
            case 0xaf:
            { // scas
                opcode.len = 1;
                (opcode.w = opcode.raws[0] & 1) ?
                        opcode.opname = "scasw" : opcode.opname = "scasb";
                flush(adstr, dump.dump_scas(opcode.serialize()));
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
            { // mov (immediate to register: w = 0)
                opcode.raws[1] = fetch();
                opcode.len = 2;
                opcode.w = 0;
                opcode.reg = opcode.raws[0] & 7;
                opcode.opname = "mov";
                setData(&opcode);
                flush(adstr, dump.dump_mov3(opcode.serialize()));
                break;
            }
            case 0xb8:
            case 0xb9:
            case 0xba:
            case 0xbb:
            case 0xbc:
            case 0xbd:
            case 0xbe:
            case 0xbf:
            { // mov (immediate to register: w = 1)
                opcode.raws[1] = fetch();
                opcode.raws[2] = fetch();
                opcode.len = 3;
                opcode.w = 1;
                opcode.reg = opcode.raws[0] & 7;
                opcode.opname = "mov";
                setData(&opcode);
                flush(adstr, dump.dump_mov3(opcode.serialize()));
                break;
            }            
            case 0xc2: { // ret (Within seg Adding Immed to SP)
                opcode.raws[1] = fetch();
                opcode.raws[2] = fetch();
                opcode.len = 3;
                opcode.disp = (int16) reverse2(&opcode.raws[1]);
                opcode.opname = "ret";
                flush(adstr, dump.dump_ret2(opcode.serialize()));
                break;
            }
            case 0xc3:
            { // ret(Within Segment)
                opcode.len = 1;
                opcode.opname = "ret";
                flush(adstr, dump.dump_ret1(opcode.serialize()));
                break;
            }
            case 0xc5: { // LDS(Load Pointer to DS)
                opcode.raws[1] = fetch();
                opcode.len = 2;
                opcode.opname = "lds";
                setParameter(&opcode, opcode.raws[1]);
                flush(adstr, dump.dump_lds(opcode.serialize()));
                break;
                break;
            }
            case 0xc6:
            { // mov (immediate to Register/Memory)
                opcode.raws[1] = fetch();
                opcode.raws[2] = fetch();
                opcode.len = 3;
                opcode.w = 0;
                opcode.opname = "mov";
                setParameter(&opcode, opcode.raws[1]);
                setData(&opcode);
                flush(adstr, dump.dump_mov2(opcode.serialize()));
                break;

                break;
            }
            case 0xc7:
            { // mov (immediate to Register/Memory)
                opcode.raws[1] = fetch();
                opcode.raws[2] = fetch();
                opcode.raws[3] = fetch();
                opcode.len = 4;
                opcode.w = 1;
                opcode.opname = "mov";
                setParameter(&opcode, opcode.raws[1]);
                setData(&opcode);
                flush(adstr, dump.dump_mov2(opcode.serialize()));
                break;
            }
            case 0xcb: { // retf
                opcode.len = 1;
                opcode.opname = "retf";
                flush(adstr, dump.dump_retf(opcode.serialize()));
                break;
            }
            case 0xcc: {
                opcode.len = 1;
                opcode.opname = "int3";
                flush(adstr, dump.dump_int3(opcode.serialize()));
                break;
            }
            case 0xcd:
            { // Interupt
                opcode.raws[1] = fetch();
                opcode.len = 2;
                opcode.opname = "int";
                flush(adstr, dump.dump_int(opcode.serialize()));

                if (opcode.raws[1] == 7) {
                    systemcall();
                }
                break;
            }
            case 0xcf: { // IRET
                opcode.len = 1;
                opcode.opname = "iret";
                flush(adstr, dump.dump_iret(opcode.serialize()));
                break;
            }
            case 0xd0:
            case 0xd1:
            case 0xd2:
            case 0xd3:
            { // shl/sal/shr/sar/rol/ror/rcl/rcr/call
                opcode.raws[1] = fetch();
                opcode.len = 2;
                opcode.w = opcode.raws[0] & 1;
                opcode.v = (opcode.raws[0] >> 1) & 1;
                setParameter(&opcode, opcode.raws[1]);

                switch (opcode.ext) {
                    case 0: { // rol
                        opcode.opname = "rol";
                        flush(adstr, dump.dump_rol(opcode.serialize()));
                        break;
                    }
                    case 1: {
                        opcode.opname = "ror";
                        flush(adstr, dump.dump_ror(opcode.serialize()));
                        break;
                    }
                    case 2:
                    { // rcl
                        opcode.opname = "rcl";
                        flush(adstr, dump.dump_rcl(opcode.serialize()));
                        break;
                    }
                    case 3: { // rcr
                        opcode.opname = "rcr";
                        flush(adstr, dump.dump_rcr(opcode.serialize()));
                        break;
                    }
                    case 4:
                    { // shl/sal
                        opcode.opname = "shl";
                        flush(adstr, dump.dump_shl(opcode.serialize()));
                        break;
                    }
                    case 5:
                    { // shr
                        opcode.opname = "shr";
                        flush(adstr, dump.dump_shr(opcode.serialize()));
                        break;
                    }
                    case 7:
                    { // sar
                        opcode.opname = "sar";
                        flush(adstr, dump.dump_sar(opcode.serialize()));
                        break;
                    }
                    default:
                    {
                        fprintf(stdout, "0xd0~3 not implemented\n");
                        break;
                    }
                }

                break;
            }
            case 0xe0: { // loopnz
                opcode.raws[1] = fetch();
                opcode.len = 2;
                opcode.disp = addr + (char) opcode.raws[1];
                opcode.opname = "loopnz";
                flush(adstr, dump.dump_loopnz(opcode.serialize()));
                break;
            }
            case 0xe2:
            { // loop
                opcode.raws[1] = fetch();
                opcode.len = 2;
                opcode.disp = addr + (char) opcode.raws[1];
                opcode.opname = "loop";
                flush(adstr, dump.dump_loop(opcode.serialize()));
                break;
            }
            case 0xe3:
            { // jcxz
                opcode.raws[1] = fetch();
                opcode.len = 2;
                opcode.disp = addr + (char) opcode.raws[1];
                opcode.opname = "jcxz";
                flush(adstr, dump.dump_jcxz(opcode.serialize()));
                break;
            }
            case 0xe4:
            case 0xe5: { // IN (Fixed port)
                opcode.raws[1] = fetch();
                opcode.len = 2;
                opcode.opname = "in";
                opcode.w = opcode.raws[0] & 1;
                opcode.port = opcode.raws[1];
                flush(adstr, dump.dump_in1(opcode.serialize()));
                break;
            }
            case 0xe6:
            case 0xe7: { // Out (Fixed port)
                opcode.raws[1] = fetch();
                opcode.len = 2;
                opcode.opname = "out";
                opcode.w = opcode.raws[0] & 1;
                opcode.port = opcode.raws[1];
                flush(adstr, dump.dump_out1(opcode.serialize()));

                break;
            }
            case 0xe8:
            { // call(Direct within Segment)
                opcode.raws[1] = fetch();
                opcode.raws[2] = fetch();
                opcode.len = 3;
                opcode.disp = (int16) reverse2(&opcode.raws[1]) + addr;
                opcode.opname = "call";
                flush(adstr, dump.dump_call1(opcode.serialize()));
                break;
            }
            case 0xe9:
            { // jmp(Direct within Segment)			
                opcode.raws[1] = fetch();
                opcode.raws[2] = fetch();
                opcode.len = 3;
                opcode.disp = (int16) reverse2(&opcode.raws[1]) + addr;
                opcode.opname = "jmp";
                flush(adstr, dump.dump_jmp1(opcode.serialize()));
                break;
            }
            case 0xea: { // jumf(Direct Intersegment)
                opcode.raws[1] = fetch();
                opcode.raws[2] = fetch();
                opcode.raws[3] = fetch();
                opcode.raws[4] = fetch();
                opcode.len = 5;
                opcode.offset = reverse2(&opcode.raws[1]);
                opcode.seg    = reverse2(&opcode.raws[3]);
                opcode.opname = "jmpf";
                flush(adstr, dump.dump_jmp4(opcode.serialize()));
                break;                
            }
            case 0xeb:
            { // jmp(Direct within Segment-Short)
                opcode.raws[1] = fetch();
                opcode.len = 2;
                opcode.disp = addr + (char) opcode.raws[1];
                opcode.opname = "jmp short";
                flush(adstr, dump.dump_jmp2(opcode.serialize()));
                break;
            }
            case 0xec:
            case 0xed: { // in (variable port)
                opcode.len = 1;
                opcode.w = opcode.raws[0] & 1;
                opcode.opname = "in";
                flush(adstr, dump.dump_in2(opcode.serialize()));
                break;
            }
            case 0xee:
            case 0xef: { // out (variable port)
                opcode.len = 1;
                opcode.w = opcode.raws[0] & 1;
                opcode.opname = "out";
                flush(adstr, dump.dump_out2(opcode.serialize()));
                break;
            }
            case 0xf2:
            case 0xf3:
            { // rep
                opcode.option.len = 1;
                opcode.option.raws[0] = opcode.raws[0];
                opcode.option.z = opcode.raws[0] & 1;
                opcode.option.prefix = "rep";
                opcode.option.optype = REPEAT;
                opcode.option.active = true;
                opcode.option.adstr = adstr;
                break;
            }
            case 0xf4:
            { // hlt
                opcode.len = 1;
                opcode.opname = "hlt";
                flush(adstr, dump.dump_hlt(opcode.serialize()));
                break;
            }
            case 0xf6:
            case 0xf7:
            { // neg/mul/imul/div/idiv
                opcode.raws[1] = fetch();
                opcode.len = 2;
                opcode.w = opcode.raws[0] & 1;
                setParameter(&opcode, opcode.raws[1]);
                switch (opcode.ext) {
                    case 0:
                    { // test
                        opcode.raws[opcode.len++] = fetch();
                        if (opcode.w) {
                            opcode.raws[opcode.len++] = fetch();
                        }
                        setData(&opcode);
                        opcode.opname = "test";
                        flush(adstr, dump.dump_test2(opcode.serialize()));
                        break;
                    }
                    case 2:
                    { // not
                        opcode.opname = "not";
                        flush(adstr, dump.dump_not(opcode.serialize()));
                        break;
                    }
                    case 3:
                    { // neg
                        opcode.opname = "neg";
                        flush(adstr, dump.dump_neg(opcode.serialize()));
                        break;
                    }
                    case 4: { // mul
                        opcode.opname = "mul";
                        flush(adstr, dump.dump_mul(opcode.serialize()));
                        break;
                    }
                    case 5: { // imul
                        opcode.opname = "imul";
                        flush(adstr, dump.dump_imul(opcode.serialize()));
                        break;
                    }
                    case 6:
                    { // div
                        opcode.opname = "div";
                        flush(adstr, dump.dump_div(opcode.serialize()));
                        break;
                    }
                    case 7:
                    { // idiv
                        opcode.opname = "idiv";
                        flush(adstr, dump.dump_idiv(opcode.serialize()));
                        break;
                    }
                    default:
                    {
                        fprintf(stdout, " 0xf6/7 is not implemented\n");
                        break;
                    }
                }
                break;

            }
            case 0xfa: { // cli
                opcode.len = 1;
                opcode.opname = "cli";
                flush(adstr, dump.dump_cli(opcode.serialize()));
                break;
            }
            case 0xfb: { // sti
                opcode.len = 1;
                opcode.opname = "sti";
                flush(adstr, dump.dump_sti(opcode.serialize()));
                break;
            }
            case 0xfc:
            { // cld
                opcode.len = 1;
                opcode.opname = "cld";
                flush(adstr, dump.dump_cld(opcode.serialize()));
                break;
            }
            case 0xfd:
            { // std
                opcode.len = 1;
                opcode.opname = "std";
                flush(adstr, dump.dump_std(opcode.serialize()));
                break;
            }
            case 0xfe:
            case 0xff:
            { // push/call/jmp
                opcode.raws[1] = fetch();
                opcode.len = 2;
                opcode.w = opcode.raws[0] & 1;
                setParameter(&opcode, opcode.raws[1]);
                switch (opcode.ext) {
                    case 0:
                    { // inc (Register/Memory)
                        opcode.opname = "inc";
                        flush(adstr, dump.dump_inc1(opcode.serialize()));
                        break;
                    }
                    case 1:
                    {
                        opcode.opname = "dec";
                        flush(adstr, dump.dump_dec1(opcode.serialize()));
                        break;
                    }
                    case 2:
                    {
                        opcode.opname = "call";
                        flush(adstr, dump.dump_call2(opcode.serialize()));
                        break;
                    }
                    case 4:
                    { // jmp (Indirect within Segment)
                        opcode.opname = "jmp";
                        flush(adstr, dump.dump_jmp3(opcode.serialize()));
                        break;
                    }
                    case 6:
                    { // push(register/memory)
                        opcode.opname = "push";
                        flush(adstr, dump.dump_push1(opcode.serialize()));
                        break;
                    }
                    default:
                    {
                        fprintf(stdout, "0xff is not implemented\n");
                        break;
                    }
                }

                break;
            }
            default:
            { // undefined
                opcode.len = 1;
                flush(adstr, dump.dump_undefined(opcode.serialize()));
                break;
            }
        }

    }
}
