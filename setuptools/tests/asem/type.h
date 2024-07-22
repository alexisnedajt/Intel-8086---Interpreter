#ifndef TYPES
#define TYPES
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <err.h>
#include <byteswap.h>
#include <string.h>

#define M3_STRING     14

//====================================For the system calls=====================================

typedef struct {int16_t m1i1, m1i2, m1i3; uint16_t m1p1, m1p2, m1p3;} mess_1;
typedef struct {int16_t m2i1, m2i2, m2i3; int32_t m2l1, m2l2; uint32_t m2p1;}__attribute__((packed)) mess_2;
typedef struct {int16_t m3i1, m3i2; uint16_t m3p1; uint32_t m3ca1;} mess_3;
typedef struct {int32_t m4l1, m4l2, m4l3, m4l4, m4l5;} mess_4;
typedef struct {int8_t m5c1, m5c2; int16_t m5i1, m5i2; int32_t m5l1, m5l2, m5l3;}mess_5;
typedef struct {int16_t m6i1, m6i2, m6i3; int32_t m6l1; uint16_t m6f1;} mess_6;


typedef struct {
  int16_t m_source;			/* who sent the message */
  int16_t m_type;			/* what kind of message is it */
  union {
	mess_1 m_m1;
	mess_2 m_m2;
	mess_3 m_m3;
	mess_4 m_m4;
	mess_5 m_m5;
	mess_6 m_m6;
  } m_u;
} message;

/* The following defines provide names for useful members. */
#define m1_i1  m_u.m_m1.m1i1
#define m1_i2  m_u.m_m1.m1i2
#define m1_p1  m_u.m_m1.m1p1

#define m2_i1  m_u.m_m2.m2i1
#define m2_i3  m_u.m_m2.m2i3
#define m2_p1  m_u.m_m2.m2p1

typedef void (*func)(message*,uint8_t*);

//====================================For the interpreter=====================================

#define Norm 0
#define Imm1 1
#define Imm2 2
#define Add1 3
#define Add2 4
#define RegR 5

typedef struct instruct{
    char* name; //name of the instruction

    int d;   //==============//
    int w;   //              //
    int mod; //  Bytes info  //
    int reg; //              //
    int rm;  //==============//

    int data;   	//===================//
    int disp; 		// Next bytes values //
    int new_data;	//===================//

    char* rg; //Might make another struct to remember these and treat them separately

    int add; //The address of an instruction if immediate address

    int type; //take the vaues of Norm | Imm1 | Imm2 | Add1 | Add2 | RegR depending on the case
} instruct;

#endif 