#include <stdlib.h>
#include <time.h>
#include "shop.h"

int main() {
    srand((unsigned int)time(NULL));

    shop_init();
    shop_print();
    mover_start();

    enum { CUSTOMER_CNT = 3};
    pthread_t customer[CUSTOMER_CNT];
    Customer args[CUSTOMER_CNT];

    for (int i = 0; i < CUSTOMER_CNT; ++i) {
        args[i].id = i + 1;
        args[i].buy_need = 9000 + rand() % 2001;
    }

    customer_start(customer, args, CUSTOMER_CNT);
    customer_join(customer, CUSTOMER_CNT);

    mover_stop_join();
    shop_destroy();

    return 0;
}