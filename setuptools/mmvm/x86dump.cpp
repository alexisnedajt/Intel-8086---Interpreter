#include <stdio.h>
#include <iostream>
#include "x86dump.h"
#include "opcode.h"

const char* X86Dump::reg8[] = {
	"al",
	"cl",
	"dl",
	"bl",
	"ah",
	"ch",
	"dh",
	"bh",
};

const char* X86Dump::reg16[] = {
	"ax",
	"cx",
	"dx",
	"bx",
	"sp",
	"bp",
	"si",
	"di",
};

const char* X86Dump::regseg[] = {
	"es",
	"cs",
	"ss",
	"ds",
};

//X86Dump::X86Dump():index(0), point(14) {
X86Dump::X86Dump():symTableP(NULL) {
}

X86Dump::X86Dump(uint16 point):symTableP(NULL) {
	this->point = point;
	//	printf("point = %d\n", this->point);
}



X86Dump::X86Dump(map<uint16, string> *symTableP) {
	this->symTableP = symTableP;
}



X86Dump::~X86Dump() {	
}

void X86Dump::writeOpCode(X86OpCode *opcode) {
    if (opcode->optype == REPEAT || opcode->optype == REPE || opcode->optype == REPNE)
        opcode->opname = "rep " + opcode->opname;
    
    Dump::writeOpCode(opcode->opname.c_str());
}

void X86Dump::writeMemory(X86OpCode *opcode, uint16 addr) {
    if (opcode->optype == OVERWRIDE) {
        string prefix = "[" + opcode->prefix + ":%04x]";
        index += sprintf((char *)&buf[index], prefix.c_str(), addr);        
    } else {
        Dump::writeMemory16(addr);        
    }
}

/*  Integrated */
string X86Dump::push_pop1(X86OpCode *opcode) {
    writeRawBytes(opcode->raws, opcode->len);
    writeOpCode(opcode);
    writeEA(opcode, reg16);
    return toString();    
}

string X86Dump::push_pop2(X86OpCode *opcode) {
    writeRawBytes(opcode->raws, opcode->len);
    writeOpCode(opcode);
    writeRegister(opcode->reg, reg16);
    return toString();   
}

string X86Dump::push_pop3(X86OpCode *opcode) {
    writeRawBytes(opcode->raws, opcode->len);
    writeOpCode(opcode);
    writeRegister(opcode->reg, regseg);
    return toString();   
}

string X86Dump::add_adc_sub_sbb_cmp1(X86OpCode* opcode) {
    const char **reg;
    
    (opcode->w) ? (reg = reg16) : (reg = reg8);
    writeRawBytes(opcode->raws, opcode->len);
    writeOpCode(opcode);
    if(opcode->d) { // to register
        writeRegister(opcode->reg, reg);
        writeComma();
        writeEA(opcode, reg);
    } else {        // from register
        writeEA(opcode, reg);
        writeComma();
        writeRegister(opcode->reg, reg);
    }
    
    return toString();
}

        
string X86Dump::add_adc_sub_sbb_cmp2(X86OpCode* opcode) {
    writeRawBytes(opcode->raws, opcode->len);
    writeOpCode(opcode);
    if((!opcode->s) && opcode->w) {
        writeEA(opcode, reg16);
        writeComma();
        writeNumber16(opcode->data);
    } else if(opcode->s && opcode->w) {
        writeEA(opcode, reg16);
        writeComma();
        writeSignedNumber16(opcode->data);
    } else {
        if(opcode->mod != 3) writeByteStr();
        writeEA(opcode, reg8);
        writeComma();
//        writeSignedNumber8(opcode->data);
        writeNumber8(opcode->data);        
    }
    return toString();
}

string X86Dump::add_adc_sub_sbb_cmp3(X86OpCode* opcode) {
    writeRawBytes(opcode->raws, opcode->len);
    writeOpCode(opcode);
    if (opcode->w) {
        writeRegister(0, reg16);
        writeComma();
        writeNumber16(opcode->data);
    } else {
        writeRegister(0, reg8);
        writeComma();
        writeSignedNumber8(opcode->data);
    }
    return toString();
}


string X86Dump::inc_dec1(X86OpCode* opcode) {
    writeRawBytes(opcode->raws, opcode->len);
    writeOpCode(opcode);
    if(opcode->w) {
        writeEA(opcode, reg16);
    } else {
        if(opcode->mod != 3) writeByteStr();
        writeEA(opcode, reg8);
    }
    return toString();
}

string X86Dump::inc_dec2(X86OpCode *opcode) {
    writeRawBytes(opcode->raws, opcode->len);
    writeOpCode(opcode);
    writeRegister(opcode->reg, reg16);
    return toString();
}

string X86Dump::shl_shr_sar_rcl_rcr_rol_ror(X86OpCode *opcode) {
    writeRawBytes(opcode->raws, opcode->len);
    writeOpCode(opcode);
    if(opcode->w) {
        writeEA(opcode, reg16);
        writeComma();
        (opcode->v) ? writeString("cl") : writeSignedNumber8(1);
    } else {
        if (opcode->mod != 3) writeByteStr();        
        writeEA(opcode, reg8);
        writeComma();
        (opcode->v) ? writeString("cl") : writeSignedNumber8(1);        
    }
    return toString();
}

string X86Dump::and_or_xor1(X86OpCode* opcode) {
    const char **reg;
    
    (opcode->w) ? (reg = reg16) : (reg = reg8);
    writeRawBytes(opcode->raws, opcode->len);
    writeOpCode(opcode);
    if(opcode->d) { // to register
        writeRegister(opcode->reg, reg);
        writeComma();
        writeEA(opcode, reg);
    } else { // from register
        writeEA(opcode, reg);
        writeComma();
        writeRegister(opcode->reg, reg);        
    }
    return toString();
}

string X86Dump::and_or_xor2(X86OpCode* opcode) {
    writeRawBytes(opcode->raws, opcode->len);
    writeOpCode(opcode);
    if (opcode->w) {
        writeEA(opcode, reg16);            
        writeComma();
        writeNumber16(opcode->data);
    } else {
        if(opcode->mod != 3) writeByteStr();
        writeEA(opcode, reg8);
        writeComma();
//        writeSignedNumber8(opcode->data);
        writeNumber8(opcode->data);
    }
    return toString();
}

string X86Dump::and_or_xor3(X86OpCode *opcode) {
    writeRawBytes(opcode->raws, opcode->len);
    writeOpCode(opcode);
    if(opcode->w) {
        writeRegister(opcode->reg, reg16);
        writeComma();
        writeNumber16(opcode->data);
    } else {
        writeRegister(opcode->reg, reg8);
        writeComma();
//        writeSignedNumber8(opcode->data);
        writeNumber8(opcode->data);        
    }
    return toString();
}


string X86Dump::div_idiv_mul_imul(X86OpCode* opcode) {
    writeRawBytes(opcode->raws, opcode->len);
    writeOpCode(opcode);
    if(opcode->w) {
        writeEA(opcode, reg16);
    } else {
        if(opcode->mod != 3) writeByteStr();
        writeEA(opcode, reg8);
    }
    return toString();
}
/*
string X86Dump::in_out1(X86OpCode *opcode) {
    writeRawBytes(opcode->raws, opcode->len);
    writeOpCode(opcode);
    writeNumber8(opcode->port);
    writeComma();
    (opcode->w) ? writeRegister(0, reg16) : writeRegister(0, reg8);
    return toString();
}
*/

string X86Dump::simple_jmp(X86OpCode* opcode) {
    writeRawBytes(opcode->raws, opcode->len);
    writeOpCode(opcode);
    writeNumber16(opcode->disp);
    return toString();
}

string X86Dump::simple_dump(X86OpCode *opcode) {
    writeRawBytes(opcode->raws, opcode->len);
    writeOpCode(opcode);
    return toString();
}

void X86Dump::writeSymbol(uint16 addr) {
    if (symTableP) {
        map<uint16, string>::iterator it;
        if((it = symTableP->find(addr)) != symTableP->end()) {
            writeString(" ; ");
            writeString(it->second.c_str());
        }
    }
}

void X86Dump::writeEA(X86OpCode *opcode, const char **reg) {
    if ((opcode->mod == 0) && (opcode->rom == 6)) {
        string format = "[%04x]";
        if (opcode->optype == OVERWRIDE) {
            format = "[" + opcode->prefix + ":%04x]";
        }
        sprintf((char *)opcode->eastr, format.c_str(), opcode->disp & 0xffff);
        writeString(opcode->eastr);
        return;
    }
    
    if(opcode->mod == 3) {
        writeRegister(opcode->rom_reg, reg);
        return;
    } 
    
    string regstr = "";
    switch(opcode->rom) {
        case 0:
            regstr = "bx+si";
            break;
        case 1: 
            regstr = "bx+di";
            break;
        case 2: 
            regstr = "bp+si";
            break;
        case 3: 
            regstr = "bp+di";
            break;
        case 4: 
            regstr = "si";
            break;
        case 5: 
            regstr = "di";
            break;
        case 6:          
            regstr = "bp";
            break;
        case 7:
            regstr = "bx";
            break;
        default: {
            break;
        }
        
    }
    
    string prefix = "%s";
    if (opcode->optype == OVERWRIDE) {
        prefix = opcode->prefix + ":%s";        
    }
    
    string format = "";
    if (opcode->disp == 0) {
        format = "[" + prefix + "]";
        sprintf((char *) opcode->eastr, format.c_str(), regstr.c_str());
    } else if (opcode->disp < 0) {
        format = "[" + prefix + "-%x]";
        sprintf((char *) opcode->eastr, format.c_str(), regstr.c_str(), ~opcode->disp + 1);
    } else {
        format = "[" + prefix + "+%x]";
        sprintf((char *) opcode->eastr, format.c_str(), regstr.c_str(), opcode->disp);
    }
    
    writeString(opcode->eastr);
    
        

}

/*
inline static uint16 reverse2(const uchar *buf) {
	return buf[0] | buf[1] << 8;
}
*/

/* JMP (Direct within Segment) */
string X86Dump::dump_jmp1(X86OpCode *opcode) {
    writeRawBytes(opcode->raws, opcode->len);
    writeOpCode(opcode);
    writeNumber16(opcode->disp);
    writeSymbol(opcode->disp);
    return toString();
}

/* JMP (Direct with Segment-Short) */
string X86Dump::dump_jmp2(X86OpCode *opcode) {
    writeRawBytes(opcode->raws, opcode->len);
    writeOpCode(opcode);
    writeNumber16(opcode->disp);
    return toString();
}

/* JMP (Indirect within Segment) */
string X86Dump::dump_jmp3(X86OpCode *opcode) {
    writeRawBytes(opcode->raws, opcode->len);
    writeOpCode(opcode);
    writeEA(opcode, reg16);
    return toString();
}

/* JMP (Direct Intersegment) */
string X86Dump::dump_jmp4(X86OpCode *opcode) {
    char buf[10] = { 0 };
    writeRawBytes(opcode->raws, opcode->len);
    writeOpCode(opcode);
    sprintf(buf, "%04x:%04x", opcode->seg, opcode->offset);
    writeString(buf);
    return toString();
}

/* Ret (Witin Segment) */
string X86Dump::dump_ret1(X86OpCode *opcode) {
    return simple_dump(opcode);
}

/* Ret (Within Seg Adding Immed to SP)*/
string X86Dump::dump_ret2(X86OpCode *opcode) {
    writeRawBytes(opcode->raws, opcode->len);
    writeOpCode(opcode);
    writeNumber16(opcode->disp);
    return toString();
}


/* CMP (Register/Memory and Register) */
string X86Dump::dump_cmp1(X86OpCode *opcode) {
    return add_adc_sub_sbb_cmp1(opcode);
}

/* CMP (Immediate with Register/Memory) */
string X86Dump::dump_cmp2(X86OpCode *opcode) {
    return add_adc_sub_sbb_cmp2(opcode);
}

/* CMP (Immediate with Accumulator */
string X86Dump::dump_cmp3(X86OpCode *opcode) {
    return add_adc_sub_sbb_cmp3(opcode);    
}

/* JNE/JNZ */
string X86Dump::dump_jne(X86OpCode *opcode) {
    return simple_jmp(opcode);
}

/* JE/JZ */
string X86Dump::dump_je(X86OpCode *opcode) {
    return simple_jmp(opcode);
}

/* JLE/JNG */
string X86Dump::dump_jle(X86OpCode *opcode) {
    return simple_jmp(opcode);
}

/* JNL/JGE */
string X86Dump::dump_jnl(X86OpCode *opcode) {
    return simple_jmp(opcode);
}

/* JNLE/JE(Jump on Not Less or Equal/Greater*/
string X86Dump::dump_jnle(X86OpCode *opcode) {
    return simple_jmp(opcode);
}

/* JL(Jump on Less/Not Greater or Equal) */
string X86Dump::dump_jl(X86OpCode *opcode) {
    return simple_jmp(opcode);    
}

/* JBE(Jump on Below or Equal/Not Above)*/
string X86Dump::dump_jbe(X86OpCode *opcode) {
    return simple_jmp(opcode);    
}

/* JO (Jump on Overflow) */
string X86Dump::dump_jo(X86OpCode *opcode) {
    return simple_jmp(opcode);        
}

/* JS (Jump on Sign) */
string X86Dump::dump_js(X86OpCode *opcode) {
    return simple_jmp(opcode);
}

/* JB/JNAE(Jump on Below/Not Above or Equal */
string X86Dump::dump_jb(X86OpCode *opcode) {
    return simple_jmp(opcode);            
}

/* JNBE(Jump on Not Below or Equal/Above) */
string X86Dump::dump_jnbe(X86OpCode *opcode) {
    return simple_jmp(opcode);    
}

/* LOOP */
string X86Dump::dump_loop(X86OpCode *opcode) {
    return simple_jmp(opcode);
}

/* LOOPNZ (Loop While Not Zero/Equal) */
string X86Dump::dump_loopnz(X86OpCode *opcode) {
    return simple_jmp(opcode);
}

/* JCXZ(Jump on CX Zero)*/
string X86Dump::dump_jcxz(X86OpCode* opcode) {
    return simple_jmp(opcode);
}
/* LEA */
string X86Dump::dump_lea(X86OpCode *opcode) {
	writeRawBytes(opcode->raws, opcode->len);
//	writeOpCode("lea");
        writeOpCode(opcode);
	writeRegister(opcode->reg, reg16);
	writeComma();
	writeEA(opcode, reg16);
	return toString();
}

string X86Dump::dump_lds(X86OpCode *opcode) {
    writeRawBytes(opcode->raws, opcode->len);
    writeOpCode(opcode);
    writeRegister(opcode->reg, reg16);
    writeComma();
    writeEA(opcode, reg16);
    return toString();    
}

/* CBW */
string X86Dump::dump_cbw(X86OpCode *opcode) {
    return simple_dump(opcode);
}

/* XOR(Reg./Memory and Register to Ether) */
string X86Dump::dump_xor1(X86OpCode *opcode) {
    return and_or_xor1(opcode);
}

/* XOR(Immediate to Accumulator) */
string X86Dump::dump_xor3(X86OpCode *opcode) {
    return and_or_xor3(opcode);
}

/* AND(Reg/Memory and Register to Either */
string X86Dump::dump_and1(X86OpCode *opcode) {
    return and_or_xor1(opcode);
}


/* AND(Immediate to Registry/Memory) */
string X86Dump::dump_and2(X86OpCode *opcode) {    
    return and_or_xor2(opcode);
}

/* AND (Immediate to Accumulator) */
string X86Dump::dump_and3(X86OpCode *opcode) {
    return and_or_xor3(opcode);
}

/* OR(Reg./Memory and Register to Ether) */
string X86Dump::dump_or1(X86OpCode *opcode) {
    return and_or_xor1(opcode);
}

/* OR(Immediate to Register/Memory) */
string X86Dump::dump_or2(X86OpCode *opcode) {
    return and_or_xor2(opcode);
}

/* OR(Immediate to Accumulator) */
string X86Dump::dump_or3(X86OpCode *opcode) {
    return and_or_xor3(opcode);
}

/* INC(Register/Memory) */
string X86Dump::dump_inc1(X86OpCode *opcode) {
    return inc_dec1(opcode);
}

/* INC(Register) */
string X86Dump::dump_inc2(X86OpCode *opcode) {
    return inc_dec2(opcode);
}

/* NEG */
string X86Dump::dump_neg(X86OpCode *opcode) {
	writeRawBytes(opcode->raws, opcode->len);
        writeOpCode(opcode);
        (opcode->w) ? writeEA(opcode, reg16) : writeEA(opcode, reg8);
//	writeEA(opcode, reg16);
	return toString();
}

/* NOT */
string X86Dump::dump_not(X86OpCode *opcode) {
    writeRawBytes(opcode->raws, opcode->len);
    writeOpCode(opcode);
    if(opcode->w) {
        writeEA(opcode, reg16);
    } else {
        if(opcode->mod != 3) writeByteStr();
        writeEA(opcode, reg8);
    }
    return toString();
}

/* DIV(Divide(Unsigned)) */
string X86Dump::dump_div(X86OpCode *opcode) {
    return div_idiv_mul_imul(opcode);
    /*
    writeRawBytes(opcode->raws, opcode->len);
    writeOpCode(opcode);
    if(opcode->w) {
        writeEA(opcode, reg16);
    } else {
        if(opcode->mod != 3) writeByteStr();
        writeEA(opcode, reg8);
    }
    return toString();
    */
}
/* IDIV(Integer Divide) */
string X86Dump::dump_idiv(X86OpCode *opcode) {
    return div_idiv_mul_imul(opcode);
    /*
    writeRawBytes(opcode->raws, opcode->len);
    writeOpCode(opcode);
    if(opcode->w) {
        writeEA(opcode, reg16);        
    } else {
        if(opcode->mod != 3) writeByteStr();
        writeEA(opcode, reg8);
    }

    return toString();
     */
}

/*  Multiply (unsigned)*/
string X86Dump::dump_mul(X86OpCode* opcode) {
    return div_idiv_mul_imul(opcode);
}
/*  Integer Multiply (signed)*/
string X86Dump::dump_imul(X86OpCode* opcode) {
    return div_idiv_mul_imul(opcode);
}


/* DEC(Register/Memory) */
string X86Dump::dump_dec1(X86OpCode *opcode) {
    return inc_dec1(opcode);
}

/* DEC(Register) */
string X86Dump::dump_dec2(X86OpCode *opcode) {
    return inc_dec2(opcode);
}

/* CWD(Convert Word to Double Word) */
string X86Dump::dump_cwd(X86OpCode *opcode) {
    return simple_dump(opcode);
}

/* SHL/SAL(Shirt Logical/Arithmetic Left) */
string X86Dump::dump_shl(X86OpCode *opcode) {
    return shl_shr_sar_rcl_rcr_rol_ror(opcode);
}

/* SAR(Shirt Arithmetic Right) */
string X86Dump::dump_sar(X86OpCode *opcode) {    
    return shl_shr_sar_rcl_rcr_rol_ror(opcode);
}

/* SHR(Shift Logical Right) */
string X86Dump::dump_shr(X86OpCode *opcode) {
    return shl_shr_sar_rcl_rcr_rol_ror(opcode);
}

/*RCL (Rotate Through Carry Flag Left */
string X86Dump::dump_rcl(X86OpCode *opcode) {
    return shl_shr_sar_rcl_rcr_rol_ror(opcode);
}
/* ROL(Rotate Left) */
string X86Dump::dump_rol(X86OpCode *opcode) {
    return shl_shr_sar_rcl_rcr_rol_ror(opcode);
}
/* ROR(Rotate Rigit) */
string X86Dump::dump_ror(X86OpCode *opcode) {
    return shl_shr_sar_rcl_rcr_rol_ror(opcode);
}

/* RCR (Rotate Through Carry Right) */
string X86Dump::dump_rcr(X86OpCode *opcode) {
    return shl_shr_sar_rcl_rcr_rol_ror(opcode);
}

/* Call (Direct within Segment) */
string X86Dump::dump_call1(X86OpCode *opcode) {

    writeRawBytes(opcode->raws, opcode->len);
    writeOpCode(opcode);

    writeNumber16(opcode->disp);

    writeSymbol(opcode->disp);

    return toString();
}

/* Call (Indirect within Segment) */
string X86Dump::dump_call2(X86OpCode *opcode) {
    writeRawBytes(opcode->raws, opcode->len);
    writeOpCode(opcode);
    writeEA(opcode, reg16);
    writeSymbol(opcode->disp);
    return toString();
}

/* PUSH Register/Memory */
string X86Dump::dump_push1(X86OpCode *opcode) {
    return push_pop1(opcode);
}


/* Push Register */     
string X86Dump::dump_push2(X86OpCode *opcode) {
    return push_pop2(opcode);
}

/* Push Segment Register */
string X86Dump::dump_push3(X86OpCode *opcode) {
    return push_pop3(opcode);
}

/* POP Register/Memory */
string X86Dump::dump_pop1(X86OpCode *opcode) {
    return push_pop1(opcode);
}

/* Pop Register */
string X86Dump::dump_pop2(X86OpCode *opcode) {
    return push_pop2(opcode);
}

/* POP Segment Register*/
string X86Dump::dump_pop3(X86OpCode *opcode) {
    return push_pop3(opcode);  
}

/* Mov (Register/Memory to/from Register) */
string X86Dump::dump_mov1(X86OpCode *opcode) {
	const char **reg;

	writeRawBytes(opcode->raws, opcode->len);
	
	if(opcode->w) {
//		writeOpCode("mov");
                writeOpCode(opcode);
		reg = reg16;
	} else {
//		writeOpCode("mov");
                writeOpCode(opcode);
		reg = reg8;
	}

	if(opcode->d) { /* to register */
		writeRegister(opcode->reg, reg); 
		writeComma();
		writeEA(opcode, reg);
		/*
		if(opcode->mod == 3) {
			writeRegister(opcode->rom_reg, reg);
		} else {
			writeString(opcode->eastr);
		}
		*/
	} else {        /* from register */
		writeEA(opcode, reg);
		/*
		if(opcode->mod == 3) {
			writeRegister(opcode->rom_reg, reg);
		} else {
			writeString(opcode->eastr);
		}
		*/
		writeComma();
		writeRegister(opcode->reg, reg);
	}

	return toString();
}


/* Mov (Immediate to Register/Memory) */
string X86Dump::dump_mov2(X86OpCode *opcode) {
    writeRawBytes(opcode->raws, opcode->len);
    writeOpCode(opcode);
    if(opcode->w) {
        writeEA(opcode, reg16);
    } else {
        if(opcode->mod != 3) writeByteStr();
        writeEA(opcode, reg8);
    }
    writeComma();
    if(opcode->w) {
        writeNumber16(opcode->data);
    } else {
//        writeSignedNumber8(opcode->data);
        writeNumber8(opcode->data);
    }
    
    return toString();
}



/* Mov (Immediate to Register) */
string X86Dump::dump_mov3(X86OpCode *opcode) {
    writeRawBytes(opcode->raws, opcode->len);
    writeOpCode(opcode);
    if(opcode->w) {
        writeRegister(opcode->reg, reg16);
        writeComma();
        writeNumber16(opcode->data);
        //		writeNumber16(reverse2(&opcode->raws[1]));
    } else {
        writeRegister(opcode->reg, reg8);
        writeComma();
//        writeSignedNumber8(opcode->data);
        writeNumber8(opcode->data);
    }
    
    return toString();
}

/* Mov (Memory to Accumulator) */
string X86Dump::dump_mov4(X86OpCode *opcode) {
    writeRawBytes(opcode->raws, opcode->len);
    writeOpCode(opcode);
    
    if(opcode->w) {
        writeRegister(0, reg16); // ax
    } else {
        writeRegister(0, reg8); // al
    }
    writeComma();
    writeMemory(opcode, opcode->addr);
//    writeMemory16(opcode->addr);
    return toString();    
}


/* Mov (Accumulator to Memory) */
string X86Dump::dump_mov5(X86OpCode *opcode) {
    writeRawBytes(opcode->raws, opcode->len);
    writeOpCode(opcode);
    writeMemory(opcode, opcode->addr);
//    writeMemory16(opcode->addr);
    writeComma();
    if(opcode->w) {
        writeRegister(0, reg16); // ax
    } else {
        writeRegister(0, reg8);  // al
    }
    
    return toString();	
}

/* Mov(Register/Memory to Segment Register) */
string X86Dump::dump_mov6(X86OpCode *opcode) {
    writeRawBytes(opcode->raws, opcode->len);
    writeOpCode(opcode);
    writeRegister(opcode->reg, regseg);
    writeComma();
    writeEA(opcode, reg16);
    return toString();
}

/* Mov(Segment Register to Register/Memory) */
string X86Dump::dump_mov7(X86OpCode *opcode) {
    writeRawBytes(opcode->raws, opcode->len);
    writeOpCode(opcode);
    writeEA(opcode, reg16);
    writeComma();
    writeRegister(opcode->reg, regseg);
    return toString();
}

/* ADD Reg/Memory with Register to Ether */
string X86Dump::dump_add1(X86OpCode *opcode) {
    return add_adc_sub_sbb_cmp1(opcode);    
}

/* ADD Immediate to Register/Memory */
string X86Dump::dump_add2(X86OpCode *opcode) {
    return add_adc_sub_sbb_cmp2(opcode);    
}

/* ADD(Immediate to Accumulator) */
string X86Dump::dump_add3(X86OpCode *opcode) {
    return add_adc_sub_sbb_cmp3(opcode);    
}

/* ADC (Add with Carry) */
string X86Dump::dump_adc1(X86OpCode* opcode) {
    return add_adc_sub_sbb_cmp1(opcode);  
}

/* ADC (Immediate to Register/Memory) */
string X86Dump::dump_adc2(X86OpCode *opcode) {
    return add_adc_sub_sbb_cmp2(opcode);
}

/* ADC (Immediate to Accumulator)*/
string X86Dump::dump_adc3(X86OpCode *opcode) {
    return add_adc_sub_sbb_cmp3(opcode);
}

/* SUB Reg/Memory and Register to Enter */
string X86Dump::dump_sub1(X86OpCode *opcode) {
    return add_adc_sub_sbb_cmp1(opcode);
}

/* SUB Immediate from Register/Memory */
string X86Dump::dump_sub2(X86OpCode *opcode) {
    return add_adc_sub_sbb_cmp2(opcode);
}

/* SUB(Immediate from Accumulator) */
string X86Dump::dump_sub3(X86OpCode *opcode) {
    return add_adc_sub_sbb_cmp3(opcode);
}

/* SSB(Reg./Memory and Register to Either)*/
string X86Dump::dump_sbb1(X86OpCode *opcode) {
    return add_adc_sub_sbb_cmp1(opcode);
}

/* SSB (Immediate from Register/Memory) */
string X86Dump::dump_sbb2(X86OpCode *opcode) {
    return add_adc_sub_sbb_cmp2(opcode);
}

/* SBB (Immediate from Accumulator) */
string X86Dump::dump_sbb3(X86OpCode *opcode) {
    return add_adc_sub_sbb_cmp3(opcode);
}


string X86Dump::dump_jnb(X86OpCode *opcode) {
    return simple_jmp(opcode);
}

/* Test(Register/Memory and Register) */
string X86Dump::dump_test1(X86OpCode *opcode) {
    writeRawBytes(opcode->raws, opcode->len);
    writeOpCode(opcode);    
    if(opcode->w) {
        writeEA(opcode, reg16);
        writeComma();
        writeRegister(opcode->reg, reg16);
    } else {
        writeEA(opcode, reg8);        
        writeComma();
        writeRegister(opcode->reg, reg8);
    }
    
    return toString();
}

/* Test = And Function to flags, no Result(Immediate Data and Register/Memory) */
string X86Dump::dump_test2(X86OpCode* opcode) {
    writeRawBytes(opcode->raws, opcode->len);
    writeOpCode(opcode);
    if(opcode->w) {
        writeEA(opcode, reg16);
        writeComma();
        writeNumber16(opcode->data);
    } else {
        if(opcode->mod != 3) writeByteStr();
        writeEA(opcode, reg8);
        writeComma();
//        writeSignedNumber8(opcode->data);
        writeNumber8(opcode->data);        
    }
    return toString();
}

/* Test (Immediate Data and Accumulator)*/
string X86Dump::dump_test3(X86OpCode *opcode) {
    writeRawBytes(opcode->raws, opcode->len);
    writeOpCode(opcode);
    if(opcode->w) {
        writeRegister(0, reg16);
        writeComma();
        writeNumber16(opcode->data);
    } else {
        writeRegister(0, reg8);
        writeComma();
        writeSignedNumber8(opcode->data);        
    }
    return toString();
}


/* XCHG(Exchange) */
string X86Dump::dump_xchg1(X86OpCode *opcode) {
    writeRawBytes(opcode->raws, opcode->len);
    writeOpCode(opcode);    
    if(opcode->w) {
        writeEA(opcode, reg16);        
        writeComma();
        writeRegister(opcode->reg, reg16);
    } else {
        writeEA(opcode, reg8);                
        writeComma();
        writeRegister(opcode->reg, reg8);

    }
    return toString();
}

/* XCHG (Register with Accumulator) */
string X86Dump::dump_xchg2(X86OpCode *opcode) {
    writeRawBytes(opcode->raws, opcode->len);
    writeOpCode(opcode);
    writeRegister(opcode->reg, reg16);
    writeComma();
    writeString("ax");
    return toString();
}

/*LODS(Load Byte/Wd to AL/AX) */
string X86Dump::dump_lods(X86OpCode *opcode) {
    return simple_dump(opcode);

}

/* SCAS(Scan Byte/Word) */
string X86Dump::dump_scas(X86OpCode *opcode) {
    return simple_dump(opcode);    
}

/* Stor Byte/Wd from AL/A */
string X86Dump::dump_stos(X86OpCode *opcode) {
    return simple_dump(opcode);        
}

/* HLT */
string X86Dump::dump_hlt(X86OpCode *opcode) {
    return simple_dump(opcode);            
}

/* CLD */
string X86Dump::dump_cld(X86OpCode *opcode) {
    return simple_dump(opcode);
}

/* CLI */
string X86Dump::dump_cli(X86OpCode *opcode) {
    return simple_dump(opcode);
}

/* STD(Set Direction) */
string X86Dump::dump_std(X86OpCode *opcode) {
    return simple_dump(opcode);
}

/* STI(Set Interupt) */
string X86Dump::dump_sti(X86OpCode *opcode) {
    return simple_dump(opcode);
}

/* MOVS (Move Byte/Word) */
string X86Dump::dump_movs(X86OpCode *opcode) {
    return simple_dump(opcode);    
}

/* CMPS (Compare Byte/Word) */
string X86Dump::dump_cmps(X86OpCode *opcode) {
    return simple_dump(opcode);        
}

/* IRET (Interrupt Return) */
string X86Dump::dump_iret(X86OpCode *opcode) {
    return simple_dump(opcode);
}

/* OUT (Fixed Port)*/
string X86Dump::dump_out1(X86OpCode *opcode) {
    writeRawBytes(opcode->raws, opcode->len);
    writeOpCode(opcode);
    writeNumber8(opcode->port);
    writeComma();
    (opcode->w) ? writeRegister(0, reg16) : writeRegister(0, reg8);
    return toString();
}
/* OUT (Variable Port) */
string X86Dump::dump_out2(X86OpCode *opcode) {
    writeRawBytes(opcode->raws, opcode->len);
    writeOpCode(opcode);
    if (opcode->w) {
        writeString("dx, ax");
    } else {
        writeString("dx, al");
    }
    return toString();
}
/* In (Fixed Port) */
string X86Dump::dump_in1(X86OpCode *opcode) {
    writeRawBytes(opcode->raws, opcode->len);
    writeOpCode(opcode);
    (opcode->w) ? writeRegister(0, reg16) : writeRegister(0, reg8);
    writeComma();
    writeNumber8(opcode->port);
    return toString();
}
/* IN (Variable Port) */
string X86Dump::dump_in2(X86OpCode *opcode) {
    writeRawBytes(opcode->raws, opcode->len);
    writeOpCode(opcode);
    if(opcode->w) {
        writeString("ax, dx");
    } else {
        writeString("al, dx");
    }
    return toString();
}

/* RETF */
string X86Dump::dump_retf(X86OpCode *opcode) {
    return simple_dump(opcode);
}

/* PUSHF */
string X86Dump::dump_pushf(X86OpCode *opcode) {
    return simple_dump(opcode);
}

/*POPF */
string X86Dump::dump_popf(X86OpCode* opcode) {
    return simple_dump(opcode);
}

string X86Dump::dump_int(X86OpCode *opcode) {
    writeRawBytes(opcode->raws, opcode->len);
    writeOpCode(opcode);
    writeIntNumber(opcode->raws[1]);
    return toString();
}

/* Interrupt 3 */
string X86Dump::dump_int3(X86OpCode *opcode) {
    return simple_dump(opcode);
}

string X86Dump::dump_undefined(X86OpCode *opcode) {
    writeRawBytes(opcode->raws, opcode->len);
    opcode->opname = "(undefined)";
    writeOpCode(opcode);    
    //	writeOpCode("(undefined)");
    return toString();
}

