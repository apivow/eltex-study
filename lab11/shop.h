#ifndef SHOP_H
#define SHOP_H

#include <pthread.h>

enum { SHOP_CNT = 5};

typedef struct {
    int id;
    int buy_need;
} Customer;

void shop_init();
void shop_destroy();
void shop_print();

void mover_start();
void mover_stop_join();

void customer_start(pthread_t customer[], Customer args[], int cnt);
void customer_join(pthread_t customer[], int cnt);

#endif