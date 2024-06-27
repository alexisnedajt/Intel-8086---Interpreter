#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <err.h>
#include <byteswap.h>
#include <string.h>
#include "syscalls.h"

#define XOR 0b00001100

#define MOV1 0b100010
#define MOV2 0b1100011
#define MOV3 0b1011
#define MOV4 0b1010000 //TODO
#define MOV5 0b1010001 //TODO
#define MOV6 0b10001110
#define MOV7 0b10001100

#define PUSH1 0b11111111 // call2 and call4
#define PUSH2 0b01010
#define PUSH3 0b000 //TODO

#define POP1 0b10001111
#define POP2 0b01011
#define POP3 0b111

#define XCHG1 0b1000011
#define XCHG2 0b10010

#define IN1 0b1110010
#define IN2 0b1110110

#define OUT1 0b1110011
#define OUT2 0b1110111

#define XLAT 0b11010111
#define LEA 0b10001101
#define LDS 0b11000101
#define LES 0b11000100
#define LAHF 0b10011111
#define SAHF 0b10011110
#define PUSHF 0b10011100
#define POPF 0b10011101

#define ADD1 0b000000
#define ADD2 0b100000
#define ADD3 0b0000010

#define ADC1 0b000100
#define ADC3 0b0001010

#define INC1 0b1111111
#define INC2 0b01000

#define AAA 0b00110111
#define BAA 0b00100111

#define SUB1 0b001010
#define SUB3 0b0010110

#define SSB1 0b000110
#define SSB3 0b000111

#define DEC2 0b01001

#define CMP1 0b001110
#define CMP3 0b0011110

#define AAS 0b00111111
#define DAS 0b00101111
#define MUL 0b1111011

#define AAM1 0b11010100

#define AAD1 0b11010101
#define AAD2 0b00001010

#define CBW 0b10011000
#define CWD 0b10011001

#define INT1 0b11001101

#define XOR1 0b001100
#define XOR2 0b1000000 // and or2
#define XOR3 0b0011010

#define OR1 0b000010
#define OR3 0b0000110

#define JNB 0b01110011

#define CLC 0b11111000
#define CMC 0b11110101
#define STC 0b11111001
#define CLD 0b11111100
#define STD 0b11111101
#define CLI 0b11111010
#define STI 0b11111011
#define HLT 0b11110100
#define WAIT1 0b10011011
#define ESC 0b11011 //TODO
#define LOCK 0b11110000

#define TEST1 0b1000010
#define TEST3 0b1010100

#define JE 0b01110100
#define JL 0b01111100
#define JLE 0b01111110
#define JB 0b01110010
#define JBE 0b01110110
#define JP 0b01111010
#define JO 0b01110000
#define JS 0b01111000
#define JNE 0b01110101
#define JNL 0b01111101
#define JNLE 0b01111111
#define JNBE 0b01110111
#define JNP 0b01111011
#define JNO 0b01110001
#define JNS 0b01111001
#define LOOP 0b11100010
#define LOOPZ 0b11100001
#define LOOPNZ 0b11100000
#define JCXZ 0b11100011
#define INT2 0b11001100
#define INTO 0b11001110
#define IRET 0b11001111

#define JMP1 0b11101001 
#define JMP2 0b11101011 
#define JMP3 0b11111111 //Same for JMP5
#define JMP4 0b11101010

#define RET1 0b11000011
#define RET2 0b11000010
#define RET3 0b11001011
#define RET4 0b11001010

#define CALL1 0b11101000
#define CALL3 0b10011010

#define STOS 0b1010101
#define LODS 0b1010110
#define SCAS 0b1010111
#define MOVS 0b1010010
#define CMPS 0b1010011
#define REP 0b1111001

#define TEST2 0b1111011

#define AND1 0b001000
#define AND3 0b0010010

#define NOT 0b1111011
#define SHL 0b110100 //and SHR/SAR/ROL/ROR/RCL/RCR

typedef struct instruct{
    char* name;
    int w;
    int d;
    int mod;
    int reg;
    int rm;
    int data;
    int disp;
} instruct;