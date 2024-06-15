#include "binary.h"
#include "disassm.h"
#include "runtime.h"
#include <iostream>
#include <fstream>


/* Constructor */
Binary::Binary():filename(""), buf(NULL), debug(false) {
}

Binary::Binary(string filename):filename(filename), buf(NULL), debug(false) {
	init();
}

/* Destructor */
Binary::~Binary() {
	delete[] buf;
}

void Binary:: setFile(string filename) {
	this->filename = filename;
	init();
}

uchar* Binary::getBuffer() {
	return buf;
}

void Binary::setDebug(bool debug) {
	this->debug = debug;
}

void Binary::setRootPath(string rootPath) {
    this->rootPath = rootPath;
}

string Binary::getRootPath() {
    return this->rootPath;
}

uint32 Binary::getTextSize() {
    return t_size;
}

uint32 Binary::getDataSize() {
    return d_size;
}

uint16 Binary::getHeaderSize() {
    return h_size;
}

uint16 Binary::getBssSize() {
    return bss_size;
}

uint32 Binary::getSymSize() {
    return sym_size;
}

void Binary::setBuffer(uchar **bufref) {
    *bufref = buf;
}

void Binary::dumpBinary() {
	for (int i = 0; i < b_size; ++i) {
		if(i % 16 == 0) cout << "\n";
		printf("%02x ", buf[i]);
	}
	cout << "\n";
}

void Binary::disassm() {
    X86Disassm disassm;
    if (buf[0] == 0xeb && buf[1] == 0x0e) { // X86+V6
        disassm.setBinary(&buf[16], t_size);
        disassm.setSymbol(&buf[16 + t_size + d_size], sym_size);
        disassm.disassm();
    } else if (buf[0] == 0x01 && buf[1] == 0x03) { // X86+Minix
        disassm.setBinary(&buf[h_size], t_size);
        disassm.disassm();
    } else {                                     // No Header -> Assume X86MinixBinary
        disassm.setBinary(&buf[0], b_size);
        disassm.disassm();
    }
}

void Binary::run(vector<string> args) {
    vector<string> envp;
    envp.push_back("PATH=/usr:/usr/bin");
    
    
    Runtime *runtime;
    switch (cpu_type) {
        case X86: {
            runtime = new X86Runtime(os_type, debug, rootPath);
            break;
        }
        case PDP11: {
            break;
        }
        case END_OF_CUPType: {
            fprintf(STDERR, "unknown os type");
            exit(1);
            break;
        }
        default: {
            fprintf(STDERR, "unknown cpu type");
            exit(1);
            break;
        }
    }
    
    switch(os_type) {
        case UNIXV6: {
            runtime->setBinary(&buf[h_size], t_size + d_size, 0, 0, sym_size);
            break;
        }
        case MINIX: {
            runtime->setBinary(&buf[h_size], t_size, d_size, bss_size, sym_size);
            break;
        }
        case END_OF_OSType: {
            fprintf(STDERR, "unknown os type");
            exit(1);
            break;
        }
        default:
            fprintf(STDERR, "unsupported format");
            exit(1);
            break;
            
    }
    
    runtime->run(args, envp);
    
    delete runtime;
}

void Binary::setType() {

    if (buf[0] == 0xeb && buf[1] == 0x0e) {    // X86+V6
        os_type = UNIXV6;
        cpu_type = X86;
    } else if(buf[0] == 0x01 && buf[1] == 0x03) { // X86+Minix
        os_type = MINIX;
        cpu_type = X86;
    }
}

/* private method */
void Binary::init() {
	stat(filename.c_str(), &st);
	readFile();	
        
        if (buf[0] == 0xeb && buf[1] == 0x0e) { // X86+V6
            t_size = buf[2] | (buf[3] << 8);
            d_size = buf[4] | (buf[5] << 8);
            sym_size = st.st_size - (t_size + d_size + 16);
            h_size = 16;
        } else if (buf[0] == 0x01 && buf[1] == 0x03) { // X86+Minix
            h_size = buf[4];
            t_size = buf[8]  | (buf[9] << 8)  | (buf[10] << 16) | (buf[11] << 24);
            d_size = buf[12] | (buf[13] << 8) | (buf[14] << 16) | (buf[15] << 24);
            bss_size = buf[16] | (buf[17] << 8) | (buf[18] << 16) | (buf[19] << 24);
        }        
        setType();
}

void Binary::readFile() {
	ifstream fin;
	delete[] buf;
	buf = new uchar[st.st_size];
	fin.open(filename.c_str(), ios_base::in | ios_base::binary);
	fin.read((char *)buf, st.st_size);
	b_size = fin.gcount();
	fin.close();
}
