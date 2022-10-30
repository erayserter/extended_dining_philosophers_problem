#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>

int ayranLeft;

void startDine(int philosopherCount);
void* dine(void* tid);
void diningCycle(void* tid);
void enterAyranQueueAndGet();
void enterEatingQueueAndEat();
void think();

void startDineForNPhilosophers(int philosopherCount) {
    pthread_t tid;
    ayranLeft = rand() % (5 * philosopherCount) + 1;

    for (int i = 0; i < philosopherCount; i++)
        pthread_create(&tid, NULL, dine, (void*)&tid);

    pthread_exit(NULL);
}

void startDine(int philosopherCount) {
    while (true) {
        startDineForNPhilosophers(philosopherCount);
        sleep(5);
    }
}

void* dine(void* tid) {
    int id = *((int *)tid);
    while(ayranLeft > 0) {
        diningCycle(tid);
    }
    return tid;
}

void diningCycle(void* tid) {
    enterAyranQueueAndGet();
    enterEatingQueueAndEat();
    think();
}

void enterAyranQueueAndGet() {

}

void enterEatingQueueAndEat() {

}

void think() {

}

int main() {
    int philosopherCount;
    srand(time(0));

    printf("Enter the number of philosophers: ");
    scanf("%d", &philosopherCount);

    startDineForNPhilosophers(philosopherCount);

    return 0;
}