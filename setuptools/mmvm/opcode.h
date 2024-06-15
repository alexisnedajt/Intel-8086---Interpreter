#ifndef _OPCODE_H_
#define _OPCODE_H_

#include "common.h"
#include <string.h>

typedef enum {
    REPEAT,
    REPE,
    REPNE,
    OVERWRIDE,
    NONE,
} OPTYPE;

typedef struct _Options {
    uchar raws[4];
    uint16 len;
    uchar z;
    bool active;
    string prefix;
    string adstr;
    OPTYPE optype;
    
    _Options():len(0), active(false), optype(NONE) {
        memset(raws, 0, 4);
    }
    ~_Options() {        
    }
    
    void flush() {
        memset(raws, 0, 4);
        len = z = 0;
        active = false;
        prefix = "";
        adstr = "";
        optype = NONE;
    }
    
} Options;

typedef struct _X86OpCode {
	uchar raws[8];
	uint16 len;
	uchar w;   /* if word or not */
	uchar d;   /* to register or from register */
	uchar s;
	uchar v;   
        uchar z;   /*  string primitive */
	uchar mod; /* mode */
	uchar reg; /* register */
	uchar ext; /* extension number in ModR/M byte */
	uchar rom; /* R/M in ModR/M byte */
	//	uint16 disp; /* displacement */
	int16 disp; /* displacement */
	uint16 ea;
	char eastr[32];
	uchar data_p;  /* start index for data */
	uchar rom_reg;
	uint16 data;  /* data */
        uchar port;   /* port */
	uint16 addr;  /* memory address */
        string opname; /* operator name */
        
        uint16 offset; /* for jmpf */
        uint16 seg;    /* for jmpf */
        
        
        OPTYPE optype;
        string prefix;

        Options option;

	void flush() {
            memset(raws, 0, 8);
            len = w = d = s = v = 0;
            mod = reg = rom = ext = 0;
            ea = disp = 0;
            eastr[0] = 0;
            data_p = 1;
            data = addr = port = 0;
            rom_reg = 0;
            opname = "";            
            optype = NONE;
            prefix = "";
            offset = seg = 0;
            
	}
        
        _X86OpCode* serialize() {
            if(!option.active)
                return this;
            
            this->optype = option.optype;
            this->prefix = option.prefix;
            
            for (int i = len-1; i >= 0; --i) {
                raws[i + option.len] = raws[i];
            }
            for (int i = 0; i < option.len; ++i) {
                raws[i] = option.raws[i];
            }
            len += option.len;
            
//            if (option.optype == REPEAT) opname = option.prefix + " " + opname;
//            if(option.prefix == "rep") 
            
            option.flush();
            
            return this;

        }
} X86OpCode;

#endif
