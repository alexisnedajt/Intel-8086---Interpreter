#ifndef _OS_H_
#define _OS_H_

#include "common.h"
#include <map>
#include <vector>

class Runtime;
class X86Runtime;

class OS {
 protected:
     bool debug;
     uint32 brksize;
     uint32 datasize;
     uint32 bsssize;
     string rootPath;
     int pid;
     
     void createPid();
     
 public:
     OS():debug(false), brksize(0){}
     virtual int execSysCall() = 0;
     virtual ~OS() {}
     virtual map<uint16, string> createSymbolTable(const uchar *bin, uint16 size) = 0;
     virtual void processArgs(const vector<string>& args, const vector<string>& envp = vector<string>()) = 0;
     
     uint16 read16(uchar *p) {
         return p[0] | p[1] << 8;
     }
     uint32 read32(uchar *p) {
         return p[0] | p[1] << 8 | p[2] << 16 | p[3] << 24;
     }
     void write16(uchar *p, uint16 value) {
         p[0] = value;
         p[1] = value >> 8;
     }
     void write32(uchar *p, uint32 value) {
         p[0] = value;
         p[1] = value >> 8;
         p[2] = value >> 16;
         p[3] = value >> 24;
     }
     
     void setBrkSize(uint32 brksize) {
         this->brksize = brksize;
     }     
     void setDataSize(uint32 datasize) {
         this->datasize = datasize;
     }
     void setBssSize(uint32 bsssize) {
         this->bsssize = bsssize;
     }
     void setRootPath(string rootPath) {
         this->rootPath = rootPath;
     }
     
     int getPid() {
         return this->pid;
     }
     
     string convertPath(string path);
     
     
};

class UnixV6 : public OS {
protected:
     Runtime *runtime;
//     uchar **memref;
     uchar *memory;
     uint16 *mreg16;
     uint16 *reg; // register;
     uint16 *sreg; // status register
public:
    UnixV6();
    UnixV6(Runtime *runtime);
    UnixV6(Runtime *runtime, bool debug);
    ~UnixV6();
    int execSysCall();
    
    map<uint16, string> createSymbolTable(const uchar *bin, uint16 size);
    void processArgs(const vector<string>& args, const vector<string>& envp = vector<string>());
    
    
 
    virtual void v6indirect(uint16 addr) = 0;
    virtual void v6write(uint16 addr, uint16 len) = 0;
    virtual void v6exit() = 0;
    virtual void clearCarry() = 0;
 
};


class Minix : public OS {
protected:
    Runtime *runtime;
    uchar *text_mem;
    uchar *data_mem;
    uint16 *mreg16;
    uint16 *reg;
    uint16 *sreg;
    
    typedef struct _sigact {
      uint16 handler;
      uint32 mask;
      uint16 flag;
    } sigact;
    
    sigact sigary[17];
    
    void resetsignal();
    
public:
    Minix();
    Minix(Runtime *runtime);
    Minix(Runtime *runtime, bool debug);
    ~Minix();
    int execSysCall();
    map<uint16, string> createSymbolTable(const uchar *bin, uint16 size);
    void processArgs(const vector<string>& args, const vector<string>& envp = vector<string>());
    

    
    virtual int   minixwrite(uint16 fd, uint16 addr, uint16 len) = 0;
    virtual void  minixexit(uint16 status) = 0;
    virtual void  minixioctl(uint16 fd, uint16 req, uint16 addr) = 0;
    virtual int   minixbrk(uint16 addr) = 0;
    virtual int   minixopen(uint16 addr, uint16 flag, uint16 mode) = 0;
    virtual int   minixclose(uint16 fd) = 0;
    virtual int   minixread(uint16 fd, uint16 addr, uint16 len) = 0;
    virtual int   minixlseek(uint16 fd, uint32 offset, uint16 whence) = 0;
    virtual int   minixgetpid() = 0;
    virtual int   minixaccess(uint16 addr, uint16 mode) = 0;
    virtual int   minixsigaction(uint16 sig, uint16 act, uint16 oact) = 0;
    virtual int   minixcreat(uint16 addr, uint16 mode) = 0;
    virtual int   minixstat(uint16 name, uint16 buf) = 0;
    virtual int   minixtime() = 0;
    virtual int   minixgetuid() = 0;
    virtual int   minixgetgid() = 0;
    virtual int   minixunlink(uint16 addr) = 0;
    virtual int   minixfork() = 0;
    virtual int   minixwait(int *status) = 0;
    virtual int   minixexec(string path, const vector<string>& args, const vector<string>& envp) = 0;
    virtual int   minixsignal(uint16 sig, uint16 handler) = 0;
    virtual int   minixchmod(uint16 addr, uint16 mode) = 0;
//    virtual int   minixexec(string path, vector<string> args, vector<string> envp) = 0;    
};

class X86UnixV6 : public UnixV6 {
    
private:
//    uint16 *ax, *cx, *dx, *bx, *sp, *bp, *si, *di;

//    uint16 *reg; // register;
//    uint16 *sreg; // status register

public:
    X86UnixV6();
    X86UnixV6(X86Runtime *runtime);
    X86UnixV6(X86Runtime *runtime, bool debug);
    ~X86UnixV6();

    void v6indirect(uint16 addr);
    void v6write(uint16 addr, uint16 len);
    void v6exit();
    void clearCarry();
};

class X86Minix : public Minix {
private:
    X86Runtime *currentRuntime;
    X86Runtime *childRuntime;
    int sigconv(int sig);

 
public:
    X86Minix();
    X86Minix(X86Runtime *runtime);
    X86Minix(X86Runtime *runtime, bool debug);
    ~X86Minix();
    
    
    int   minixwrite(uint16 fd, uint16 addr, uint16 len);
    void  minixexit(uint16 status);
    void  minixioctl(uint16 fd, uint16 req, uint16 addr);
    int   minixbrk(uint16 addr);
    int   minixopen(uint16 addr, uint16 flag, uint16 mode);
    int   minixread(uint16 fd, uint16 addr, uint16 len);
    int   minixlseek(uint16 fd, uint32 offset, uint16 whence);
    int   minixclose(uint16 fd);
    int   minixgetpid();
    int   minixaccess(uint16 addr, uint16 mode);
    int   minixsigaction(uint16 sig, uint16 act, uint16 oact);
    int   minixcreat(uint16 addr, uint16 mode);
    int   minixstat(uint16 name, uint16 buf);
    int   minixtime();
    int   minixgetuid();
    int   minixgetgid();
    int   minixunlink(uint16 addr);
    int   minixfork();
    int   minixwait(int *status);
//    int   minixexec(string path, vector<string> args, vector<string> envp);
    int   minixexec(string path, const vector<string>& args, const vector<string>& envp);
    int   minixsignal(uint16 sig, uint16 handler);
    int   minixchmod(uint16 addr, uint16 mode);
};



#endif
