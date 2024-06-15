/* 
 * File:   message.h
 * Author: hiroaki
 * 
 *
 * Created on October 16, 2014, 7:56 AM
 * Definition of message in minix (src/include/minix/type.h is the original)
 */

#ifndef _MESSAGE_H_
#define	_MESSAGE_H_

// the followings are originals
//typedef struct {int m1i1, m1i2, m1i3; char *m1p1, *m1p2, *m1p3;} mess_1;
//typedef struct {int m2i1, m2i2, m2i3; long m2l1, m2l2; char *m2p1;} mess_2;
//typedef struct {int m3i1, m3i2; char *m3p1; char m3ca1[14];} mess_3;
//typedef struct {long m4l1, m4l2, m4l3, m4l4, m4l5;} mess_4;
//typedef struct {char m5c1, m5c2; int m5i1, m5i2; long m5l1, m5l2, m5l3;}mess_5;
//typedef struct {int m6i1, m6i2, m6i3; long m6l1; int m6f1;} mess_6;

typedef struct {uint16 m1i1, m1i2, m1i3; uint16 m1p1, m1p2, m1p3;} mess_1;
typedef struct {uint16 m2i1, m2i2, m2i3; uint32 m2l1, m2l2; uint16 m2p1;} mess_2;
typedef struct {uint16 m3i1, m3i2; uint16 m3p1; char m3ca1[14];} mess_3;
typedef struct {uint32 m4l1, m4l2, m4l3, m4l4, m4l5;} mess_4;
typedef struct {char m5c1, m5c2; uint16 m5i1, m5i2; uint32 m5l1, m5l2, m5l3;}mess_5;
typedef struct {uint16 m6i1, m6i2, m6i3; uint32 m6l1; uint16 m6f1;} mess_6;


typedef struct {
  uint16 m_source;                 /* who sent the message */
  uint16 m_type;                   /* what kind of message is it */
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
#define m1_i3  m_u.m_m1.m1i3
#define m1_p1  m_u.m_m1.m1p1
#define m1_p2  m_u.m_m1.m1p2
#define m1_p3  m_u.m_m1.m1p3

#define m2_i1  m_u.m_m2.m2i1
#define m2_i2  m_u.m_m2.m2i2
#define m2_i3  m_u.m_m2.m2i3
#define m2_l1  m_u.m_m2.m2l1
#define m2_l2  m_u.m_m2.m2l2
#define m2_p1  m_u.m_m2.m2p1

#define m3_i1  m_u.m_m3.m3i1
#define m3_i2  m_u.m_m3.m3i2
#define m3_p1  m_u.m_m3.m3p1
#define m3_ca1 m_u.m_m3.m3ca1

#define m4_l1  m_u.m_m4.m4l1
#define m4_l2  m_u.m_m4.m4l2
#define m4_l3  m_u.m_m4.m4l3
#define m4_l4  m_u.m_m4.m4l4
#define m4_l5  m_u.m_m4.m4l5

#define m5_c1  m_u.m_m5.m5c1
#define m5_c2  m_u.m_m5.m5c2
#define m5_i1  m_u.m_m5.m5i1
#define m5_i2  m_u.m_m5.m5i2
#define m5_l1  m_u.m_m5.m5l1
#define m5_l2  m_u.m_m5.m5l2
#define m5_l3  m_u.m_m5.m5l3

#define m6_i1  m_u.m_m6.m6i1
#define m6_i2  m_u.m_m6.m6i2
#define m6_i3  m_u.m_m6.m6i3
#define m6_l1  m_u.m_m6.m6l1
#define m6_f1  m_u.m_m6.m6f1



#endif	/* MESSAGE_H */

