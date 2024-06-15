#ifndef _V6DUMP_H_
#define _V6DUMP_H_

//#include "disassm.h"
#include "dump.h"

class Disassm;

class V6Dump : public Dump {
private:
	Disassm *disassm;

public:
	V6Dump();
	V6Dump(Disassm *disassm);
	~V6Dump();

	void dumpSysCall();


};

#endif
