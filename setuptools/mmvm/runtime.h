#ifndef _RUNTIME_H_
#define _RUNTIME_H_

#include <vector>

#include "common.h"
#include "opcode.h"
#include "os.h"
#include "binary.h"
#include <map>


/* For 8086 */
#define AX reg[0]
#define CX reg[1]
#define DX reg[2]
#define BX reg[3]
#define SP reg[4]
#define BP reg[5]
#define SI reg[6]
#define DI reg[7]
#define ES reg[8]
#define CS reg[9]
#define SS reg[10]
#define DS reg[11]


#define AL (((uchar *)reg)[0])
#define AH (((uchar *)reg)[1])
#define CL (((uchar *)reg)[2])
#define CH (((uchar *)reg)[3])
#define DL (((uchar *)reg)[4])
#define DH (((uchar *)reg)[5])
#define BL (((uchar *)reg)[6])
#define BH (((uchar *)reg)[7])

#define C_BIT 0x0001
#define Z_BIT 0x0040
#define S_BIT 0x0080
#define O_BIT 0x0800
#define D_BIT 0x0400

//#define CF (sreg & C_BIT)
//#define ZF (sreg & Z_BIT)
//#define SF (sreg & S_BIT)
//#define OF (sreg & O_BIT)
// OF 0000 1000 0000 0000 0x0800
// DF 0000 0100 0000 0000 0x0400
// SF 0000 0000 1000 0000 0x0080
// ZF 0000 0000 0100 0000 0x0040
// CF 0000 0000 0000 0001 0x0001 

class Runtime {
 protected:
     bool debug;
     bool fixedloop;
     int numOfLoop;
     map<uint16, string> symbolTable;
     string rootPath;
//     uint16 pid;
     
public:
    Runtime(): fixedloop(false), numOfLoop(0){
    }
    virtual ~Runtime() {}
    OSType os_type = END_OF_OSType;
    virtual uchar fetch() = 0;
    virtual uint16 fetch2() = 0;
    virtual void setMemory(uchar **text_memref, uchar **data_memref = NULL) = 0;
    
    virtual void setBinary(const uchar *bin, uint32 t_size, uint32 d_size, uint32 bss_size, uint32 sym_size) = 0;
    virtual int run(vector<string> args, vector<string> envp = vector<string>()) = 0;
    virtual int run() = 0;
    
    void setDebug(bool debug) {
        this->debug = debug;
    }
    bool getDebug() {
        return debug;
    }
  
    /*
    void setPid(uint16 pid) {
        this->pid = pid;
    }
    
    int getPid() {
        return this->pid;
    }
    */
    
    /* For debug */
    void setFixedLoop(int loopCount) {
        this->fixedloop = true;
        this->numOfLoop = loopCount;
    }
};

class X86Runtime : public Runtime {

 private:
	//uint16 ax, cx, dx, bx, sp, bp, si, di; // register
//	uint16 es, cs, ss, ds;                 // register
//	bool OF, SF, ZF, CF;                   // flag
        
        uint16 sreg;
        
        uint16 reg[12];
        string memInfo;
//        uint16 debug_addr;
        int    debug_addr;
        uchar memory[65536];
        uchar d_memory[65536];
        uchar *text;
        uchar *data;
	uchar *pc;
	uint16 addr;
//	UnixV6 os;
//        X86UnixV6 os;
        OS *os;
        bool isFork;

	void init(OSType os_type);
        int systemcall();
        void resolveDisp(X86OpCode *opcode);
        
	void setData(X86OpCode *opcode);
        void setDataSW(X86OpCode *opcode);
        
        
        void setZSOC(bool zf, bool sf, bool of, bool cf);
        void setDF(bool df);
        void setParameter(X86OpCode *opcode, uchar data);
        void setParameter(X86OpCode *opcode);
        
        uint16 getEAddress(X86OpCode *opcode); // effective address
        
	void writeRegister(X86OpCode *opcode, uchar regid, uint16 value); // can be deprecated
        uint16 readRegister(X86OpCode *opcode, uchar regid);
        
        void writeEA(X86OpCode *opcode, uint16 value);
        uint16 readEA(X86OpCode *opcode);
        
        
        void writeMemoryWithSreg(X86OpCode *opcode, uint16 srg, uint16 addr, uint16 value);
        void writeMemory(X86OpCode *opcode, uint16 addr, uint16 value);
        void writeMemoryWithES(X86OpCode *opcode, uint16 addr, uint16 value);
        void writeMemoryWithDS(X86OpCode *opcode, uint16 addr, uint16 value);
        
        uint16 readMemoryWithSreg(X86OpCode *opcode, uint16 srg, uint16 addr);
        uint16 readMemory(X86OpCode *opcode, uint16 addr);
        uint16 readMemoryWithES(X86OpCode *opcode, uint16 addr);
        uint16 readMemoryWithDS(X86OpCode *opcode, uint16 addr);
        
        void processArgs(vector<string> args);	        
        void addMemInfo(string info);
	void showHeader();
        void writeSymbol();
	string getRunStatus();
        string dump_memory(uint16 addr, uchar w);
        
        
        void push(uint16 val);
        uint16 pop();        
        void jump(uint16 addr);
        
        
        
        /* for debut*/
        void showMemory(uint16 addr);


	
 public:
	X86Runtime(OSType os_type, string rootPath);
	X86Runtime(OSType os_type, bool debug, string rootPath);
	~X86Runtime();

	uchar fetch();
	uint16 fetch2();
        uint16 nextPC();
        uint16 getAddr();
        void setMemory(uchar **memref);    
        void setMemory(uchar **text_memref, uchar **data_memref = NULL);
        void assignRegister(uint16 **preg, uint16 **psreg);
	void setBinary(const uchar *bin, uint32 t_size, uint32 d_size, uint32 bss_size, uint32 sym_size);
	int run(vector<string> args, vector<string> envp = vector<string>());
//        void run(vector<string> args, vector<string> envp);
        int run();
        int getPid();
        
        
        void setIsFork(bool isFork);
        bool getIsFork();
        void copyText(const uchar *text, size_t tsize);
        void copyData(const uchar *data, size_t dsize);
        void copyRegister(const uint16 *reg);
        void copySregister(const uint16 sreg);
        void copyPcInfo(const uint16 offset, const uint16 addr);
        void copyEachSize(const uint32 brksize, const uint32 datasize, const uint32 bsssize);
        /*
        void setAX(uint16 ax);
        void setBX(uint16 bx);
        void setCX(uint16 cx);
        void setDX(uint16 dx);
        void setSP(uint16 sp);
        void setBP(uint16 bp);
        void setSI(uint16 si);
        void setDI(uint16 di);
        void setES(uint16 es);
        void setCS(uint16 cs);
        void setSS(uint16 ss);
        void setDS(uint16 ds);
        */
                
        

};

#endif
