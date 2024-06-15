#include <stdio.h>
#include <iostream>
#include "v6dump.h"
#include "disassm.h"

V6Dump::V6Dump() {
}

V6Dump::V6Dump(Disassm* disassm):Dump() {
	this->disassm = disassm;
}

V6Dump::~V6Dump() {
}

inline static string addrToString(uint16 addr) {
	char buf[5] = { 0 };
	sprintf(buf, "%04x", addr);
	return buf;
}

inline static void flush(string line, string str) {
	cout << line << ": " << str << "\n";
}

void V6Dump::dumpSysCall() {
	uint16 addr = disassm->getAddress();
	uchar systype = disassm->fetch();

	switch(systype) {
	case 0: { /* indirect */
		uchar buf[2];
		writeRawBytes(&systype, 1);
		writeOpCode(";");
		writeString("sys indir");
		flush(addrToString(addr), toString());

		addr = disassm->getAddress();
		disassm->fetch2(buf);
		writeRawBytes(buf, 2);
		writeOpCode(";");
		writeString("arg");
		flush(addrToString(addr), toString());

		
		break;
	}
	case 1: {  /* exit  */
		writeRawBytes(&systype, 1);
		writeOpCode(";");
		writeString("sys exit");
		flush(addrToString(addr), toString());
		break;
	}
	case 4: {   /* write  */
		uchar buf[2];
		writeRawBytes(&systype, 1);
		writeOpCode(";");
		writeString("sys write");
		flush(addrToString(addr), toString());

		addr = disassm->getAddress();
		disassm->fetch2(buf);
		writeRawBytes(buf, 2);
		writeOpCode(";");
		writeString("arg");
		flush(addrToString(addr), toString());

		addr = disassm->getAddress();
		disassm->fetch2(buf);
		writeRawBytes(buf, 2);
		writeOpCode(";");
		writeString("arg");
		flush(addrToString(addr), toString());
		break;
	}
	default: {
		fprintf(stdout, "unddefined system call\n");
		break;
	}
	}

	
}

