#include <pthread.h>
#include <err.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>

typedef struct coord{
    int x;
    int y;
} coord;

void* thr_func(void* arg){
    coord* c = (coord*)arg;
    int x = c->x;
    int y = c->y;
    double dist = (sqrt((x*x + y*y)));
    int a = (int)dist;
    if (a%2 == 0){
        printf("(%i, %i)\n", x, y);
    }
}

int main(){
    for (int i = 0; i < 100000; i++){
        for (int j = 0; j <= i; j++){
            coord c;
            c.x = i;
            c.y = j;
            pthread_t thr;
            pthread_create(&thr, NULL, thr_func, &c);

            pthread_detach(thr);
        }
    }
    return 0;
}