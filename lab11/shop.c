#include "shop.h"
#include <pthread.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>

static int shop_stock[SHOP_CNT];
static pthread_mutex_t shop_mutex[SHOP_CNT];
static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

static volatile int stop_mover = 0;
static pthread_t mover_tid;

static int rand_range(unsigned int *seed, int min, int max) {
    if (max <= min) return min;
    unsigned int span = (unsigned int)(max - min + 1);
    return min + (int)(rand_r(seed) % span);
}

static void log_msg(const char *who, int id, const char *fmt, ...) {
    pthread_mutex_lock(&log_mutex);
    printf("%s#%d: ", who, id);
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
    printf("\n");
    fflush(stdout);
    pthread_mutex_unlock(&log_mutex);
}

static int try_lock_shop(unsigned int *seed) {
    int start = rand_range(seed, 0, SHOP_CNT - 1);
    for (int i = 0; i < SHOP_CNT; ++i) {
        int idx = (start + i) % SHOP_CNT;
        if (pthread_mutex_trylock(&shop_mutex[idx]) == 0) return idx;
    }
    return -1;
}

static void *customer_thread(void *arg) {
    Customer *c = (Customer *)arg;
    int id = c->id;
    int need = c->buy_need;

    unsigned int seed = (unsigned int)time(NULL)
        ^(unsigned int)(uintptr_t)pthread_self()
        ^(unsigned int)(id * 2654435761u);

    log_msg("Покупатель", id, "старт. Потребность = %d", need);

    while (need > 0) {
        int shop_idx = -1;
        while ((shop_idx = try_lock_shop(&seed)) == -1) usleep(50 * 1000);

        int on_shelf = shop_stock[shop_idx];
        if (on_shelf > 0) {
            log_msg("Покупатель", id, "зашел в магазин[%d], на полке %d", shop_idx, on_shelf);
            shop_stock[shop_idx] = 0;
            need -= on_shelf;
            log_msg("Покупатель", id, "взял %d, потребность теперь %d", on_shelf, need);
        } else {
            log_msg("Покупатель", id, "защед в магазин[%d], пусто, ничего не купил", shop_idx);
        }
        
        pthread_mutex_unlock(&shop_mutex[shop_idx]);

        if (need > 0) {
            log_msg("Покупатель", id, "засыпаю на 2с");
            sleep(2);
            log_msg("Покупатель", id, "проснулся, продолжаю");
        }
    }
    log_msg("Покупатель", id, "завершен. Потребность = %d", need);
    pthread_exit(NULL);
}

static void *mover_thread(void *unused) {
    (void)unused;
    unsigned int seed = (unsigned int)time(NULL)
        ^(unsigned int)(uintptr_t)pthread_self()
        ^ 0x9e3779b9u;

    log_msg("Грузчик", 0, "старт");

    while (!stop_mover) {
        int shop_idx = -1;
        while (!stop_mover && (shop_idx = try_lock_shop(&seed)) == -1) usleep(50 * 1000);
        if (stop_mover) break;

        int before = shop_stock[shop_idx];
        shop_stock[shop_idx] = before + 500;
        log_msg("Грузчик", 0, "магазин[%d] было %d, добавил 500, стало %d", shop_idx, before, shop_stock[shop_idx]);

        pthread_mutex_unlock(&shop_mutex[shop_idx]);
        log_msg("Грузчик", 0, "сплю 1 с");
        sleep(1);
    }

    log_msg("Грузчик", 0, "остановлен");
    pthread_exit(NULL);
}

void shop_init() {
    for (int i = 0; i < SHOP_CNT; ++i) pthread_mutex_init(&shop_mutex[i], NULL);
    for (int i = 0; i < SHOP_CNT; ++i) shop_stock[i] = 400 + rand() % 201;
    stop_mover = 0;
}

void shop_destroy() {
    for (int i = 0; i < SHOP_CNT; ++i) pthread_mutex_destroy(&shop_mutex[i]);
    pthread_mutex_destroy(&log_mutex);
}

void shop_print() {
    pthread_mutex_lock(&log_mutex);
    printf("Начальные запасы магазинов: ");
    for (int i = 0; i < SHOP_CNT; ++i) printf("%d%s", shop_stock[i], (i + 1 < SHOP_CNT ? " " : "\n"));
    fflush(stdout);
    pthread_mutex_unlock(&log_mutex);
}

void mover_start() {
    pthread_create(&mover_tid, NULL, mover_thread, NULL);
}

void mover_stop_join() {
    stop_mover = 1;
    pthread_join(mover_tid, NULL);
}

void customer_start(pthread_t customer[], Customer args[], int cnt) {
    for (int i = 0; i < cnt; ++i) pthread_create(&customer[i], NULL, customer_thread, &args[i]);
}

void customer_join(pthread_t customer[], int cnt) {
    for (int i = 0; i < cnt; ++i) pthread_join(customer[i], NULL);
}