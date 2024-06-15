#ifndef _DISASSM_H_
#define _DISASSM_H_

#include "common.h"
#include "string.h"
#include "v6dump.h"
#include "opcode.h"
#include <map>


class Disassm {
public:
	virtual uchar fetch() = 0;
	virtual void fetch2(uchar *buf) = 0;
	virtual uint32 getAddress() = 0;
};


class X86Disassm : public Disassm {
private:
	uchar *buf;
	uchar *p, *end;
	uint16 addr;
	V6Dump v6dump;
	map<uint16, string> symbolTable;
	

	void writeSymbol(uint16 addr);
	void setParameter(X86OpCode *opcode, uchar data);
	void resolve(X86OpCode *opcode);
	void resolveDisp(X86OpCode *opcode);
	void resolveEA(X86OpCode *opcode);
	void setData(X86OpCode *opcode);
	void setDataSW(X86OpCode *opcode);


public:
	X86Disassm();
	~X86Disassm();
	void setBinary(const uchar *bin, uint16 size);
	void setSymbol(const uchar *bin, uint16 sym_size);

	void disassm();
	void systemcall();
	uchar fetch();
	void fetch2(uchar *buf);
	uint32 getAddress();
};


#endif
