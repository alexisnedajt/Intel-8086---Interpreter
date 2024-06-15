#include <iostream>
#include "common.h"
#include "os.h"
#include "message.h"
#include "runtime.h"
#include <unistd.h>
#include <errno.h>

#define O_CREAT 00100

Minix::Minix() {    
}

Minix::Minix(Runtime *runtime) {
    createPid();
    this->debug = false;
    this->runtime = runtime;
    runtime->setMemory(&text_mem, &data_mem);
}

Minix::Minix(Runtime *runtime, bool debug) {
    createPid();
    this->debug = debug;
    this->runtime = runtime;
    runtime->setMemory(&text_mem, &data_mem);
}

Minix::~Minix() {    
}

void Minix::processArgs(const vector<string>& args, const vector<string>& envp) {
    string env = envp[0];

    vector<uint16> addr;
    uint16 env_head;
    uint16 args_size = args.size();

    uint16 len = env.length();
    len++;
    
    for (int i = 0; i < args_size; ++i) {
        len += args[i].length();
        len++;
    }
    
    if (len % 2) data_mem[--SP] = 0;
    
    data_mem[--SP] = '\0';
    for (int i = (env.length()-1); i >= 0; --i) {
        data_mem[--SP] = env.at(i);
    }
    env_head = SP;
    
    for (int i = args_size - 1; i >= 0; --i) {
        const char *arg = args[i].c_str();
        data_mem[--SP] = '\0';
        for (int j = args[i].length() - 1; j >= 0; --j) {
            data_mem[--SP] = arg[j];
        }
        addr.push_back(SP);
    }
    
    // Delimiter between char and env pointer
    data_mem[--SP] = 0;
    data_mem[--SP] = 0;
    
    // add env address
    data_mem[--SP] = env_head >> 8;
    data_mem[--SP] = env_head;
    
    // Delimiter between char and env pointer
    data_mem[--SP] = 0;
    data_mem[--SP] = 0;
    
    // add args address
    for (int i = 0; i < addr.size(); ++i) {
        data_mem[--SP] = addr[i] >> 8;
        data_mem[--SP] = addr[i];
    }
    
    data_mem[--SP] = args_size >> 8;
    data_mem[--SP] = args_size;
    
}
void Minix::resetsignal() {
    memset((void*)sigary, 0, sizeof(sigary));
}

int Minix::execSysCall() {
    int result = -1;
    
    message *m = (message*)&data_mem[BX];
   
    int16 ret;
    switch(m->m_type) {
        case 1: { // exit
            minixexit(m->m1_i1);
            result = m->m1_i1;
            break;
        }
        case 2: { // fork
            int ret = minixfork();
            (ret == -1) ? (m->m_type = -errno) : (m->m_type = ret);
            break;
        }
        case 3: { // read
            int ret = minixread(read16(data_mem + BX + 4), 
                    read16(data_mem + BX + 10), read16(data_mem + BX + 6));

            (ret == -1) ? (m->m_type = -errno) : (m->m_type = ret);                       
            break;
        }
        case 4: { // write
            ret = minixwrite(m->m1_i1, m->m1_p1, m->m1_i2);
            (ret == -1) ? (m->m_type = -errno) : (m->m_type = ret);
            break;
        }
        case 5: { // open
            uint16 len  = read16(data_mem + BX + 4);
            uint16 flag = read16(data_mem + BX + 6);             
            int ret;
            if (flag & O_CREAT) {
                uint16 mode_t = read16(data_mem + BX + 8);
                uint16 addr   = read16(data_mem + BX + 10);
                ret = minixopen(addr, flag, mode_t);
            } else {
                uint16 addr;
                (len <= 14) ? (addr = BX + 10) : (addr = read16(data_mem + BX + 8));
                ret = minixopen(addr, flag, 0);                
            }
            m->m_type = ret;
            break;
        }
        case 6: { // close
            uint16 fd = read16(data_mem + BX + 4);
            m->m_type = minixclose(fd);
            break;
        }
        case 7: { // wait
            int status;
            int ret = minixwait(&status);
            (ret == -1) ? m->m_type = -errno : m->m_type = ret;
            write16(data_mem + BX + 4, status);
            break;
        }
        case 8: { // creat
            int16 len = read16(data_mem + BX + 4);
            uint16 mode = read16(data_mem + BX + 6);
            uint16 addr;
            if (len <= 14) {
                addr = BX + 10;
            } else {
                addr = read16(data_mem + BX + 8);
            }
            
            int ret = minixcreat(addr, mode);
            (ret == -1) ? m->m_type = -errno : m->m_type = ret;
            break;
        }
        case 10: { // unlink
            int16 len = read16(data_mem + BX + 4);
            uint16 addr;
            if (len <= 14) {
                addr = BX + 10;
            } else {
                addr = read16(data_mem + BX + 8);
            }            
            int ret = minixunlink(addr);
            (ret == -1) ? m->m_type = -errno : m->m_type = ret;
            break;
        }
        case 13: { // time
            int ret = minixtime();
            if (ret >= 0) {
                write32(data_mem + BX + 10, ret);
                m->m_type = 0;
            }
            break;
        }
        case 15: { // chmod
            uint16 len = read16(data_mem + BX + 4);
            uint16 mode = read16(data_mem + BX + 6);
            uint16 addr;
            if (len <= 14) {
                addr = BX + 10;
            } else {
                addr = read16(data_mem + BX + 8);
            }
            int ret = minixchmod(addr, mode);
            (ret == -1) ? m->m_type = -errno : m->m_type = ret;            
            break;
        }
        case 17: { // brk
            uint16 addr = read16(data_mem + BX + 10);
            
            if (minixbrk(addr) == -1) {
                m->m_type = -errno;
            } else {
                m->m_type = 0;
                write16(data_mem + BX + 18, brksize);
                
            }
            break;
        }
        case 18: { // stat system call
            uint16 name = read16(data_mem + BX + 10);
            uint16 buf = read16(data_mem + BX + 12);            
            int ret = minixstat(name, buf);            
            (ret == -1) ? m->m_type = -errno : m->m_type = ret;
            break;
        }
        case 19: { // lseek            
            int ret = minixlseek(read16(data_mem + BX + 4),
                    read32(data_mem + BX + 10), read16(data_mem + BX + 6));
            
            (ret == -1) ? (m->m_type = -errno) : (m->m_type = 0);
            if (ret != -1) {
                write32(data_mem + BX + 10, ret);
            }
            break;
        }
        case 20: { // getpid
            m->m_type = minixgetpid();
            break;
        }
        case 24: { // getuid
            m->m_type = minixgetuid();
            break;
        }
        case 33: { // access
            uint16 len = read16(data_mem + BX + 4);
            uint16 mode = read16(data_mem + BX + 6);
            uint16 addr;
            if (len <= 14) {
                addr = BX + 10;
            } else {
                addr = read16(data_mem + BX + 8);
            }            
            int ret = minixaccess(addr, mode);
            (ret == -1) ? m->m_type = -errno : m->m_type = ret;
            break;
        }
        case 47: { // getgid
            m->m_type = minixgetgid();
            break;
        }
        case 48: { // signal
            uint16 sig = read16(data_mem + BX + 4);
            uint16 h   = read16(data_mem + BX + 14);
            int ret = minixsignal(sig, h);
            (ret == -1) ? m->m_type = -errno : m->m_type = ret;            
            break;
        }
        case 54: { // ioctl
            uint16 fd = read16(data_mem + BX + 4);
            uint16 request = read16(data_mem + BX + 8);
            uint16 address = read16(data_mem + BX + 18);
			minixioctl(fd, request, address);
            m->m_type = -errno;
            break;
        }
        case 59: { // exec system call
            uint16 path = read16(data_mem + BX + 10);
            uint16 fddr = read16(data_mem + BX + 12);
            string pathName = (char*)&data_mem[path];
            vector<string> args;
            uint16 index = fddr + 2;
            while (true) {
                uint16 argaddr = read16(data_mem + index);
                if (argaddr == 0) break;
                args.push_back((char *)&data_mem[argaddr + fddr]);
                index += 2;
            }
            index += 2;
            vector<string> envp;
            while (true) { 
                uint16 envaddr = read16(data_mem + index);
                if (envaddr == 0) break;
                envp.push_back((char*)&data_mem[envaddr + fddr]);
                index += 2;
            }            
            int ret = minixexec(pathName, args, envp);
            (ret == -1) ? m->m_type = -errno : m->m_type = ret;            
            break;
        }
        case 71: { // sigaction
            uint16 sig  = read16(data_mem + BX + 6);
            uint16 act  = read16(data_mem + BX + 10);
            uint16 oact = read16(data_mem + BX + 12);
            int ret = minixsigaction(sig, act, oact);            
            (ret == -1) ? m->m_type = -errno : m->m_type = ret;            
            break;
            
        }
        default: {
            fprintf(STDERR, "%d: unsupported system call in minix\n", m->m_type);
            exit(1);
            break;
        }
    }
    
    return result;
    
}

map<uint16, string> Minix::createSymbolTable(const uchar* bin, uint16 sym_size) {
    map<uint16, string> symbolTable;
    
    return symbolTable;
}

