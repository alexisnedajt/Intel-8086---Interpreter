#include <iostream>
#include <sys/stat.h>
#include "runtime.h"
#include "os.h"
#include "binary.h"
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>


#define MX_SIGINT   2
#define MX_SIGILL   4
#define MX_SIGFPE   8
#define MX_SIGSEGV 11



X86Minix::X86Minix(X86Runtime *runtime): Minix(runtime), childRuntime(NULL) {
    this->currentRuntime = runtime;

    runtime->assignRegister(&reg, &sreg);
}

X86Minix::X86Minix(X86Runtime *runtime, bool debug): Minix(runtime, debug), childRuntime(NULL) {
    this->currentRuntime = runtime;
    runtime->assignRegister(&reg, &sreg);
    this->debug = debug;
}

X86Minix::~X86Minix() {
}

int X86Minix::sigconv(int sig) {
    switch (sig) {
        case MX_SIGINT: return SIGINT;
        case MX_SIGILL: return SIGILL;
        case MX_SIGFPE: return SIGFPE;
        case MX_SIGSEGV: return SIGSEGV;
    }
    return -1;
}

int X86Minix::minixwrite(uint16 fd, uint16 addr, uint16 len) {
    if(debug) fprintf(STDERR, "<write(%d, 0x%04x, %d)", fd, addr, len);  
    int ret = write(fd, &data_mem[addr], len);
    if(debug) fprintf(STDERR, " => %d>\n", ret);    
    AX = 0; // is it true?
        
    // clea carry required?
    return ret;
}

void X86Minix::minixexit(uint16 status) {
    if(debug) fprintf(STDERR, "<exit(%d)>\n", status);        
    if (!currentRuntime->getIsFork()) exit(status);
}

void X86Minix::minixioctl(uint16 fd, uint16 req, uint16 addr) {
    if(debug) fprintf(STDERR, "<ioctl(%d, 0x%04x, 0x%04x)>\n", fd, req, addr);
    errno = EINVAL;
    AX = 0; // is it true?
}

int X86Minix::minixbrk(uint16 addr) {
    AX = 0; // is it true?
    
    if (debug) fprintf(STDERR, "<brk(0x%04x) => ", addr);
    
    if (addr < datasize || (addr >= ((SP & ~0x3ff) - 0x400))) {
        errno = ENOMEM;
        if (debug) fprintf(STDERR, "ENOMEM>\n");
        return -1;
    } else {
        brksize = addr;
        if(debug) fprintf(STDERR, "0>\n");
        return 0;
    }        
}

int X86Minix::minixopen(uint16 addr, uint16 flag, uint16 mode) {        
    if (debug) fprintf(STDERR, "<open(\"%s\", %d)", &data_mem[addr], flag);
    string path = convertPath(string((char*)&data_mem[addr]));
    int ret = open(path.c_str(), flag);
    if(debug) fprintf(STDERR, " => %d>\n", ret);    
    AX = 0;
    return ret;
}


int X86Minix::minixread(uint16 fd, uint16 addr, uint16 len) {
    if (debug) fprintf(STDERR, "<read(%d, 0x%04x, %d)", fd, addr, len);    
    int ret = read(fd, &data_mem[addr], len);    
    if(debug) fprintf(STDERR, " => %d>\n", ret);    
    AX = 0;    
    return ret;
}

int X86Minix::minixlseek(uint16 fd, uint32 offset, uint16 whence) {
    if (debug) fprintf(STDERR, "<lseek(%d, %d, %d)", fd, offset, whence);   
    int ret = lseek(fd, offset, whence);    
    if(debug) fprintf(STDERR, " => %d>\n", ret);
    AX = 0;
    return ret;
}

int X86Minix::minixclose(uint16 fd) {
    if (debug) fprintf(STDERR, "<close(%d)", fd);
    int ret = close(fd);
    if(debug) fprintf(STDERR, " => %d>\n", ret);
    AX = 0;    
    return ret;
}

int X86Minix::minixgetpid() {
    if (debug) fprintf(STDERR, "<getpid()");
//    pid = 12241;
    int ret = pid;
    if(debug) fprintf(STDERR, " => %d>\n", pid);
    AX = 0;
    return ret;
}

int X86Minix::minixaccess(uint16 addr, uint16 mode) {
    if (debug) fprintf(STDERR, "<access(\"%s\", 0%03o)", &data_mem[addr], mode);
    string path = convertPath(string((char*)&data_mem[addr]));
    int ret = access(path.c_str(), mode);
    if(debug) fprintf(STDERR, " => %d>\n", ret);
    AX = 0;
    return ret;
}

int X86Minix::minixsignal(uint16 sig, uint16 handler) {
    if (debug) fprintf(STDERR, "<signal(%d, 0x%04x)>\n", sig, handler);
    int s = sigconv(sig);
    if (s < 0) {
        errno = EINVAL;
        return -1;
    }
    
    int oh = sigary[sig].handler;
    sigary[sig].handler = handler;
    sigary[sig].mask = sigary[sig].flag = 0;
       
    return oh;
}

int X86Minix::minixsigaction(uint16 sig, uint16 act, uint16 oact) {
    if (debug) fprintf(STDERR, "<sigaction(%d, 0x%04x, 0x%04x)>\n", sig, act, oact);
    
    int s = sigconv(sig);
    if (s < 0) {
        errno = EINVAL;
        return -1;
    }
    
    if (oact) { // if NULL, oact is 0
        write16(data_mem + oact + 0, sigary[sig].handler);
        write32(data_mem + oact + 2, sigary[sig].mask);
        write16(data_mem + oact + 6, sigary[sig].flag);
    }
    sigary[sig].handler = read16(data_mem + act + 0);
    sigary[sig].mask    = read32(data_mem + act + 2);
    sigary[sig].flag    = read16(data_mem + act + 6);
    
    int ret = 0; // dummy. TODO: add signal handler of OS itself    
    return ret;
}

int X86Minix::minixcreat(uint16 addr, uint16 mode) {
    if (debug) fprintf(STDERR, "<creat(\"%s\", 0%03o)", &data_mem[addr], mode);
    string path = convertPath(string((char*)&data_mem[addr]));
    int ret = creat(path.c_str(), mode);
    if(debug) fprintf(STDERR, " => %d>\n", ret);
    AX = 0;
    return ret;
    
}

int X86Minix::minixstat(uint16 name, uint16 buf) {
    if (debug) fprintf(STDERR, "<stat(\"%s\", 0x%04x)", &data_mem[name], buf);
    string path = convertPath(string((char*)&data_mem[name]));
    struct stat st;
    int ret = stat(path.c_str(), &st);
    
    if (ret != -1) {
		write16(data_mem + buf + 0, st.st_dev);
        write16(data_mem + buf + 2, st.st_ino);
        write16(data_mem + buf + 4, st.st_mode);
        write16(data_mem + buf + 6, st.st_nlink);
        write16(data_mem + buf + 8, st.st_uid);
        write16(data_mem + buf + 10, st.st_gid);
        write16(data_mem + buf + 12, st.st_rdev);
        write32(data_mem + buf + 14, st.st_size);
        write32(data_mem + buf + 18, 0x545864f3); // for test
        write32(data_mem + buf + 22, 0x54584b5c); // for test
        write32(data_mem + buf + 26, 0x54584b5c); // for test
    }
    
    if(debug) fprintf(STDERR, " => %d>\n", ret);
    AX = 0;
    return ret;
}

 int X86Minix::minixtime() {
     if (debug) fprintf(STDERR, "<time()");
     int ret = 1416646658; // for debug
     if (debug) fprintf(STDERR, " => %d>\n", ret);
     AX = 0;
     return ret;
     
 }
 
 int X86Minix::minixgetuid() {
     if (debug) fprintf(STDERR, "<getuid()");
     int ret = getuid();
     if (debug) fprintf(STDERR, " => %d>\n", ret);
     AX = 0;
     return ret;
 }
 
 int X86Minix::minixgetgid() {
     if (debug) fprintf(STDERR, "<getgid()");
     int ret = getgid();
     if (debug) fprintf(STDERR, " => %d>\n", ret);
     AX = 0;
     return ret;
 }
 
 int X86Minix::minixunlink(uint16 addr) {
     if (debug) fprintf(STDERR, "<unlink(\"%s\")", &data_mem[addr]);
     string path = convertPath((char*)&data_mem[addr]);
     int ret = unlink(path.c_str());
     if (debug) fprintf(STDERR, " => %d>\n", ret);     
     AX = 0;
     return ret;
 }
 
 int X86Minix::minixfork() {
     // create child runtime
     write16(&data_mem[BX + 2], 0); // for child process
     childRuntime = new X86Runtime(MINIX, debug, rootPath);
     childRuntime->setIsFork(true);
     childRuntime->copyText(text_mem, 0x10000);
     childRuntime->copyData(data_mem, 0x10000);
     childRuntime->copyRegister(reg);
     childRuntime->copySregister(*sreg);
     childRuntime->copyPcInfo(currentRuntime->nextPC(), currentRuntime->getAddr());
     childRuntime->copyEachSize(brksize, datasize, bsssize);
     AX = 0;
     
     int pid = childRuntime->getPid();
     if (debug) fprintf(STDERR, "<fork() => %d>\n", pid);
     return pid;
 }
 
 int X86Minix::minixwait(int *status) {
     int ret = pid = 0;
     if (debug) fprintf(STDERR, "<wait()>\n");
     if (childRuntime) {
         pid = childRuntime->getPid();
         ret = childRuntime->run();
         *status = ret << ret;
         if (debug) fprintf(STDERR, "<wait() => %d, 0x%x>\n", pid, *status );

     }
     AX = 0;     
     delete childRuntime;     
     return pid;
 }
 
 int X86Minix::minixexec(string path, const vector<string>& args, const vector<string>& envp) {
     
     uchar *buf;
     string convPath = convertPath(path);
     Binary newBinary;
     newBinary.setFile(convPath);
     newBinary.setBuffer(&buf);
     
     if (debug) {
         fprintf(STDERR, "<exec(\"%s\"", path.c_str());
         for (int i = 1; i < args.size(); ++i) {
             fprintf(STDERR, ", \"%s\"", args[i].c_str());
         }
         fprintf(STDERR, ")>\n");
     }
          
     // clear memory
     memset(text_mem, 0, 0x10000);
     memset(data_mem, 0, 0x10000);
     currentRuntime->setBinary(&buf[newBinary.getHeaderSize()],
             newBinary.getTextSize(),
             newBinary.getDataSize(),
             newBinary.getBssSize(),
             newBinary.getSymSize());     
     SP = 0;
     processArgs(args, envp);
     resetsignal();
     
     return 0;
 }
 
int X86Minix::minixchmod(uint16 addr, uint16 mode) {
    if (debug) fprintf(STDERR, "<chmod(\"%s\", 0%03o)", &data_mem[addr], mode);
    string path = convertPath(string((char*)&data_mem[addr]));
    int ret = chmod(path.c_str(), mode);
    if(debug) fprintf(STDERR, " => %d>\n", ret);
    AX = 0;
    return ret;

}
