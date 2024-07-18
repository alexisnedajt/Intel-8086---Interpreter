#include "syscalls.h"

func syscalls_l[NCALLS] = {Exit,0,0,Write,0,0,0,0,0,0,
                        0,0,0,0,0,0,Brk,0,0,0,
                        0,0,0,0,0,0,0,0,0,0,
                        0,0,0,0,0,0,0,0,0,0,
                        0,0,0,0,0,0,0,0,0,0,
                        0,0,0,Ioctl,0,0,0,0,0,0};

void Write(message* m, uint8_t* data_area){
    printf("\n<write(%i, 0x%04hx, %i)",m->m1_i1,m->m1_p1,m->m1_i2);
    for(int i = 0; i<m->m1_i2; i++){
        printf("%c",data_area[(long)(m->m1_p1)+i]);
    }
    data_area[registers[3]+2] = m->m1_i2;
    registers[0] = 0;
    printf(" => %i>",m->m1_i2);
}

void Exit(message* m, uint8_t* data_area){
    printf("\n<exit(%i)>\n",m->m1_i1);
    data_area = data_area;
    exit(m->m1_i1);
}

void Ioctl(message*m, uint8_t* data_area){
    printf("\n<ioctl(%i, 0x%04x, 0x%04x)",m->m2_i1,m->m2_i3,m->m2_p1);
    data_area[registers[3]+2] = 0xea;
    data_area[registers[3]+3] = 0xff;
    registers[0] = 0;
}

void Brk(message* m, uint8_t* data_area){
    printf("\n<brk(0x%04x) => 0>", m->m1_p1);
    registers[0] = 0;
    data_area[registers[3]+2] = 0;
    data_area[registers[3]+3] = 0;
    data_area[registers[3]+18] = m->m1_p1 & 0x00ff;
    data_area[registers[3]+19] = m->m1_p1 >> 8;
}