#include "syscalls.h"

func syscalls_l[NCALLS] = {Exit,0,0,Write};

void Write(message* m, uint8_t* data_area){
    printf("\n<write(%i, 0x%04hx, %i)",m->m1_i1,m->m1_p1,m->m1_i2);
    for(int i = 0; i<m->m1_i2; i++){
        printf("%c",data_area[(long)(m->m1_p1)+i]);
    }
    printf(" => %i>",m->m1_i2);
}

void Exit(message* m, uint8_t* data_area){
    data_area = data_area;
    printf("\n<exit(%i)\n",m->m1_i1);
    exit(m->m1_i1);
}