#include "dump.h"
#include <string.h>

Dump::Dump():index(0), point(14) {
	memset(buf, 0, 64);
}

Dump::~Dump() {
}

void Dump::writeRawBytes(const uchar *raws, uint16 len) {
	for (int i = 0; i < len; ++i, index += 2) 
		sprintf((char *)&buf[index], "%02x", raws[i] );
}


void Dump::writeOpCode(const char *op) {
	if(index < point) {
		for (int i = index; i < point; ++i) {
			sprintf((char *)&buf[i], "%c", ' ');
		}
		index = point;
	}
	index += sprintf((char*)&buf[index], "%s ", op);
}

void Dump::writeByteStr() {
    index += sprintf((char*)&buf[index], "byte ");
}

void Dump::writeString(const char *str) {
	index += sprintf((char*)&buf[index], "%s", str);
}

void Dump::writeRegister(uchar regid, const char **reg) {
	index += sprintf((char *)&buf[index], "%s", reg[regid]);
}


void Dump::writeComma() {
	index += sprintf((char *)&buf[index], "%c", ',');
	index += sprintf((char *)&buf[index], "%c", ' ');
}
void Dump::writeNumber16(uint16 number) {
	index += sprintf((char *)&buf[index], "%04x", number);
}

void Dump::writeSignedNumber16(short number) {
    if(number < 0) {
        index += sprintf((char *)&buf[index], "-%x", ~number+1);
    } else {
        index += sprintf((char *)&buf[index], "%x", number);
    }
}

void Dump::writeMemory16(uint16 addr) {
	index += sprintf((char *)&buf[index], "[%04x]", addr);
}

void Dump::writeSignedNumber8(uchar number) {
    if(((char)number < 0) && ((char)number > -128)) {
        index += sprintf((char *)&buf[index], "-%x", ~number+1);
    } else {
        index += sprintf((char *)&buf[index], "%x", number);
    }
}

void Dump::writeNumber8(uchar number) {
    index += sprintf((char *)&buf[index], "%x", number);
}

void Dump::writeIntNumber(uint16 number) {	
	index += sprintf((char *)&buf[index], "%x", number);
}


string Dump::toString() {
	buf[index] = 0;
	string s = (const char*)buf;
	index = 0;	
	return s;
}


