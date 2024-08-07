#ifndef SYSCALLS
#define SYSCALLS

#include "type.h"
#include <unistd.h>

#define NCALLS		  78	/* number of system calls allowed */

#define EXIT		   1 
#define FORK		   2 
#define READ		   3 
#define WRITE		   4 
#define OPEN		   5 
#define CLOSE		   6 
#define WAIT		   7
#define CREAT		   8 
#define LINK		   9 
#define UNLINK		  10 
#define WAITPID		  11
#define CHDIR		  12 
#define TIME		  13
#define MKNOD		  14 
#define CHMOD		  15 
#define CHOWN		  16 
#define BRK		      17
#define STAT		  18 
#define LSEEK		  19
#define GETPID		  20
#define MOUNT		  21 
#define UMOUNT		  22 
#define SETUID		  23
#define GETUID		  24
#define STIME		  25
#define PTRACE		  26
#define ALARM		  27
#define FSTAT		  28 
#define PAUSE		  29
#define UTIME		  30 
#define ACCESS		  33 
#define SYNC		  36 
#define KILL		  37
#define RENAME		  38
#define MKDIR		  39
#define RMDIR		  40
#define DUP		      41 
#define PIPE		  42 
#define TIMES		  43
#define SETGID		  46
#define GETGID		  47
#define SIGNAL		  48
#define IOCTL		  54
#define FCNTL		  55
#define EXEC		  59
#define UMASK		  60 
#define CHROOT		  61 
#define SETSID		  62

void Write(message* m, uint8_t* data_area);
void Exit(message* m, uint8_t* data_area);
void Ioctl(message* m, uint8_t* data_area);
void Brk(message* m, uint8_t* data_area);

extern func syscalls_l[NCALLS];

extern uint16_t registers[8];
extern uint8_t* data_area;

extern int dump;

#endif 