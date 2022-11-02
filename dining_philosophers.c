#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>

void startDine();
void* dine(void* philospherID);
void diningCycle(long int philosopherID);
bool enterAyranQueueAndGet(long int philosopherID);
void enterEatingQueueAndEat(long int philosopherID);
void pickupChopstick(long int philosopherID);
void putdownChopstick(long int philosopherID);
void testPhilosopherCanEat(int order);
int findPhilosopher(long int philosopherID);

typedef enum {ayranWaiting, hungry, eating, thinking} philosopherState;
struct mutexInfo {
    pthread_t* tid;
    pthread_mutex_t ayranMutex;
    pthread_mutex_t chopstickMutex;
    pthread_cond_t* eatingCondition;
} mutexInfo;

philosopherState* states;
int* eatCount;
int philosopherCount;
int ayranLeft;

void init() {
    states = (philosopherState*) malloc(sizeof(philosopherState) * philosopherCount);
    mutexInfo.tid = (pthread_t*) malloc(sizeof(pthread_t) * philosopherCount);
    mutexInfo.eatingCondition = (pthread_cond_t*) malloc(sizeof(pthread_cond_t) * philosopherCount);
    eatCount = (int*) malloc(sizeof(int) * philosopherCount);

    pthread_mutex_init(&(mutexInfo.ayranMutex), NULL);
    pthread_mutex_init(&(mutexInfo.chopstickMutex), NULL);

    for(int i = 0;i < philosopherCount; i++) {
        pthread_cond_init(&((mutexInfo.eatingCondition)[i]), NULL);
        eatCount[i] = 0;
        states[i] = thinking;
    }
}

void startDineForNPhilosophers() {
    ayranLeft = rand() % (5 * philosopherCount) + 1;
    printf("total ayran count: %d\n\n", ayranLeft);

    init();

    for (int i = 0; i < philosopherCount; i++)
        pthread_create(&(mutexInfo.tid[i]), NULL, dine, (void*)&(mutexInfo.tid[i]));

    for (int i = 0; i < philosopherCount; i++)
        pthread_join(mutexInfo.tid[i], NULL);
    
    for (int i = 0; i < philosopherCount; i++)
        printf("philosopher %d ate %d times.\n", i, eatCount[i]);

    printf("=============================\n\nyeni tur basliyor\n\n=============================\n");
}

void startDine() {
    while (true) {
        startDineForNPhilosophers();
        sleep(5);
    }
}

void* dine(void* philosopherID) {
    long int id = *((long int *)philosopherID);
    while(ayranLeft > 0) {
        diningCycle(id);
    }
    return philosopherID;
}

void diningCycle(long int philosopherID) {
    sleep(0);
    bool hasAyran = enterAyranQueueAndGet(philosopherID);
    if (hasAyran)
        enterEatingQueueAndEat(philosopherID);
}

bool enterAyranQueueAndGet(long int philosopherID) {
    states[findPhilosopher(philosopherID)] = ayranWaiting;
    pthread_mutex_lock(&(mutexInfo.ayranMutex));
    if (ayranLeft <= 0) {
        pthread_mutex_unlock(&(mutexInfo.ayranMutex));
        return false;
    }
    ayranLeft--;
    sleep(0);
    pthread_mutex_unlock(&(mutexInfo.ayranMutex));
    return true;
}

void enterEatingQueueAndEat(long int philosopherID) {
    pickupChopstick(philosopherID);
    int order = findPhilosopher(philosopherID);
    eatCount[order]++;
    sleep(0);
    putdownChopstick(philosopherID);
    sleep(0);
}

void pickupChopstick(long int philosopherID) {
    int order = findPhilosopher(philosopherID);
    pthread_mutex_lock(&mutexInfo.chopstickMutex);
    states[order] = hungry;
    testPhilosopherCanEat(order);
    if (states[order] != eating)
        pthread_cond_wait(&mutexInfo.eatingCondition[order], &mutexInfo.chopstickMutex);
    pthread_mutex_unlock(&mutexInfo.chopstickMutex);
}

void putdownChopstick(long int philosopherID) {
    int order = findPhilosopher(philosopherID);
    pthread_mutex_lock(&mutexInfo.chopstickMutex);
    states[order] = thinking;
    testPhilosopherCanEat((order + (philosopherCount - 1)) % philosopherCount);
    testPhilosopherCanEat((order + 1) % philosopherCount);
    pthread_mutex_unlock(&mutexInfo.chopstickMutex);
}

void testPhilosopherCanEat(int order) {
    if ((states[order] == hungry) &&
        states[(order + (philosopherCount - 1)) % philosopherCount] != eating &&
        states[(order + 1) % philosopherCount] != eating) {
            states[order] = eating;
            pthread_cond_signal(&mutexInfo.eatingCondition[order]);
        }
}

int findPhilosopher(long int philosopherID) {
    for (int i = 0; i < philosopherCount; i++)
        if((long int) mutexInfo.tid[i] == philosopherID)
            return i;
    return -1;
}

int main() {
    srand(time(0));

    printf("Enter the number of philosophers: ");
    scanf("%d", &philosopherCount);

    startDine();

    return 0;
}