#ifndef _X86DUMP_H_
#define _X86DUMP_H_

#include <stdio.h>
#include <string.h>
#include "common.h"
#include "disassm.h"
#include "dump.h"
#include <map>
#include <string>

class X86Dump : public Dump {
private:
	static const char *reg8  [];
	static const char *reg16 [];
	static const char *regseg[];
	map<uint16, string> *symTableP; // const is not acceptable. why??
	

	void writeSymbol(uint16 addr);
	void writeEA(X86OpCode *opcode, const char **reg);

public:

	X86Dump();
	X86Dump(uint16 point);
	X86Dump(map<uint16, string> *symTableP);
	~X86Dump();
        
        void writeOpCode(X86OpCode *opcode);
        void writeMemory(X86OpCode *opcode, uint16 addr);
        
        /* Integrated */
        string push_pop1(X86OpCode *opcode);
        string push_pop2(X86OpCode *opcode);
        string push_pop3(X86OpCode *opcode);
        
        string add_adc_sub_sbb_cmp1(X86OpCode *opcode);
        string add_adc_sub_sbb_cmp2(X86OpCode *opcode);
        string add_adc_sub_sbb_cmp3(X86OpCode *opcode);
        
        string inc_dec1(X86OpCode *opcode);
        string inc_dec2(X86OpCode *opcode);
        
        string shl_shr_sar_rcl_rcr_rol_ror(X86OpCode *opcode);
        
        string and_or_xor1(X86OpCode *opcode);
        string and_or_xor2(X86OpCode *opcode);
        string and_or_xor3(X86OpCode *opcode);
        
        string div_idiv_mul_imul(X86OpCode *opcode);
        
        //string in_out1(X86OpCode *opcode);
        
        
        
        string simple_jmp(X86OpCode *opcode);
        
        string simple_dump(X86OpCode *opcode);
        
        
        /* Corresponds to each opcode */
	
	/* for push */
	string dump_push1(X86OpCode *opcode);
	string dump_push2(X86OpCode *opcode);
        string dump_push3(X86OpCode *opcode);

	/* for pop */
        string dump_pop1(X86OpCode *opcode);
	string dump_pop2(X86OpCode *opcode);
	string dump_pop3(X86OpCode *opcode);

	/* for mov */
	string dump_mov1(X86OpCode *opcode);
	string dump_mov2(X86OpCode *opcode);
	string dump_mov3(X86OpCode *opcode);
	string dump_mov4(X86OpCode *opcode);
	string dump_mov5(X86OpCode *opcode);
        string dump_mov6(X86OpCode *opcode);
        string dump_mov7(X86OpCode *opcode);

	/* for add */
	string dump_add1(X86OpCode *opcode);
	string dump_add2(X86OpCode *opcode);
	string dump_add3(X86OpCode *opcode);

	/* for sub */
	string dump_sub1(X86OpCode *opcode);
	string dump_sub2(X86OpCode *opcode);
        string dump_sub3(X86OpCode *opcode);
        
        /* for sbb */
        string dump_sbb1(X86OpCode *opcode);
        string dump_sbb2(X86OpCode *opcode);
        string dump_sbb3(X86OpCode *opcode);
        
        /* for adc */
        string dump_adc1(X86OpCode *opcode);
        string dump_adc2(X86OpCode *opcode);
        string dump_adc3(X86OpCode *opcode);        

	/* for call */
	string dump_call1(X86OpCode *opcode);
        string dump_call2(X86OpCode *opcode);

	/* for jmp */
	string dump_jmp1(X86OpCode *opcode);
	string dump_jmp2(X86OpCode *opcode);
	string dump_jmp3(X86OpCode *opcode);
        string dump_jmp4(X86OpCode *opcode);

	/* for ret */
	string dump_ret1(X86OpCode *opcode);
        string dump_ret2(X86OpCode *opcode);

	/* for cmp */
	string dump_cmp1(X86OpCode *opcode);
	string dump_cmp2(X86OpCode *opcode);
        string dump_cmp3(X86OpCode *opcode);

	/* for je/jz */
	string dump_je(X86OpCode *opcode);

	/* for jle/jng */
	string dump_jle(X86OpCode *opcode);

	/* for jne */
	string dump_jne(X86OpCode *opcode);

	/* for jnl */
	string dump_jnl(X86OpCode *opcode);

	/* for loop */
	string dump_loop(X86OpCode *opcode);
        
        /* for loopnz */
        string dump_loopnz(X86OpCode *opcode);
        
        /* for jcxz */
        string dump_jcxz(X86OpCode *opcode);

	/* for lea */
	string dump_lea(X86OpCode *opcode);
        
        /* for lds*/
        string dump_lds(X86OpCode *opcode);

	/* for cbw */
	string dump_cbw(X86OpCode *opcode);

	/* for xor */
	string dump_xor1(X86OpCode *opcode);
        string dump_xor3(X86OpCode *opcode);

	/* for or */
	string dump_or1(X86OpCode *opcode);
        string dump_or2(X86OpCode *opcode);
        string dump_or3(X86OpCode *opcode);
        
        /* for and */
        string dump_and1(X86OpCode *opcode);
	string dump_and2(X86OpCode *opcode);
        string dump_and3(X86OpCode *opcode);

	/* for inc */
	string dump_inc1(X86OpCode *opcode);
	string dump_inc2(X86OpCode *opcode);

	/* for neg */
	string dump_neg(X86OpCode *opcode);
        
        /* for not */
        string dump_not(X86OpCode *opcode);
        
        /* for div */
        string dump_div(X86OpCode *opcode);

	/* for idiv */
	string dump_idiv(X86OpCode *opcode);
        
        /* for mul */
        string dump_mul(X86OpCode *opcode);
        
        /* for imul */
        string dump_imul(X86OpCode *opcode);

	/* for dec */
	string dump_dec1(X86OpCode *opcode);
	string dump_dec2(X86OpCode *opcode);

	/* for cwd */
	string dump_cwd(X86OpCode *opcode);

	/* for shl */
	string dump_shl(X86OpCode *opcode);
        
	/* for sar */
	string dump_sar(X86OpCode *opcode);
        
        /* for shr */
        string dump_shr(X86OpCode *opcode);
        
        /* for rcl */
        string dump_rcl(X86OpCode *opcode);
        
        /* for rcr */
        string dump_rcr(X86OpCode *opcode);
        
        /* for rol */
        string dump_rol(X86OpCode *opcode);

        /* for ror */
        string dump_ror(X86OpCode *opcode);
        
        /* for jnb */
        string dump_jnb(X86OpCode *opcode);
        
        /* for test */
        string dump_test1(X86OpCode *opcode);
        string dump_test2(X86OpCode *opcode);
        string dump_test3(X86OpCode *opcode);
        
        /* for xchg */
        string dump_xchg1(X86OpCode *opcode);
        string dump_xchg2(X86OpCode *opcode);
        
        /* for jnle/ge */
        string dump_jnle(X86OpCode *opcode);
        
        /* for jl */
        string dump_jl(X86OpCode *opcode);
        
        /* for jbe */
        string dump_jbe(X86OpCode *opcode);

        /* for jo */
        string dump_jo(X86OpCode *opcode);
        
        /* for js */
        string dump_js(X86OpCode *opcode);
        
        /* for jb */
        string dump_jb(X86OpCode *opcode);
        
        /* for jnbe */
        string dump_jnbe(X86OpCode *opcode);
        
        /* for lods(b) */
        string dump_lods(X86OpCode *opcode);
        
        /* for scas */
        string dump_scas(X86OpCode *opcode);
        
        /* for stos */
        string dump_stos(X86OpCode *opcode);
        
        /* for halt */
        string dump_hlt(X86OpCode *opcode);
        
        /* for cld */
        string dump_cld(X86OpCode *opcode);
        
        /* for cil */
        string dump_cli(X86OpCode *opcode);
        
        /* for std */
        string dump_std(X86OpCode *opcode);
        
        /* for sti */
        string dump_sti(X86OpCode *opcode);
        
        /* for movs */
        string dump_movs(X86OpCode *opcode);
        
        /* for cmps */
        string dump_cmps(X86OpCode *opcode);
        
        
        /* for iret */
        string dump_iret(X86OpCode *opcode);
        
        /* for out */
        string dump_out1(X86OpCode *opcode);
        string dump_out2(X86OpCode *opcode);
        
        /* for in*/
        string dump_in1(X86OpCode *opcode);
        string dump_in2(X86OpCode *opcode);
        
        /* for retf */
        string dump_retf(X86OpCode *opcode);
        
        /* for pushf */
        string dump_pushf(X86OpCode *opcode);
        
        /* for popf */
        string dump_popf(X86OpCode *opcode);

        /* for interrupt */
	string dump_int(X86OpCode *opcode);
        string dump_int3(X86OpCode *opcode);
        
	string dump_undefined(X86OpCode *opcode);
};

#endif
