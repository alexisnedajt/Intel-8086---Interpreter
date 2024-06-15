#ifndef _DUMP_H_
#define _DUMP_H_

#include "common.h"
#include <string>

class Dump {
protected:
	uint16 index;
	uint16 point;
	uchar buf[64];
	void writeRawBytes(const uchar *raws, uint16 len);
	void writeOpCode(const char *op);
        void writeByteStr();
	void writeString(const char *op);
	//	void writeRegister16(uchar regid, const char **reg16);
	//	void writeRegister8(uchar regid, const char **reg8);
	void writeRegister(uchar regid, const char **reg);
	void writeComma();
	void writeNumber16(uint16 number);
        void writeSignedNumber16(short number);
	void writeMemory16(uint16 addr);
	void writeSignedNumber8(uchar number);
        void writeNumber8(uchar number);
	void writeIntNumber(uint16 number);
	string toString();

public:
	Dump();
	~Dump();
};

#endif
