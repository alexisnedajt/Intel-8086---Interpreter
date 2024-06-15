#ifndef _BINARY_H_
#define _BINARY_H_

#include <sys/stat.h>
#include <string>
#include <vector>
#include "common.h"

enum OSType {
    UNIXV6,
    MINIX,
    END_OF_OSType
};

enum CPUType {
    X86,
    PDP11,
    END_OF_CUPType
};

class Binary {
private:
	string filename;
	struct stat st;
	uchar *buf;
	streamsize b_size;
	uint32 t_size;
	uint32 d_size;
        uint16 h_size;
        uint16 bss_size;
	uint32 sym_size;
	bool debug;
        string rootPath;
        
        OSType os_type;
        CPUType cpu_type;

	void readFile();
	void init();
        void setType();

public:
	Binary();
	Binary(string filename);
	~Binary();

	void setFile(string filename);
	void showBinary();
	uchar *getBuffer();
	void dumpBinary();
	void setDebug(bool debug);
        void setRootPath(string rootPath);
        string getRootPath();
        

        void   setBuffer(uchar** bufref);
        /* Accessor Methods */
        uint32 getTextSize();
        uint32 getDataSize();
        uint16 getHeaderSize();
        uint16 getBssSize();
        uint32 getSymSize();
        

	/* Disassemble */
	void disassm();
	/* Execute */
	void run(vector<string> args);
};

#endif
