#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>

void initNewTurn();
void* startDine(void* philosopherID);
void dine(long int philospherID);
void diningCycle(long int philosopherID);
bool waitAndGetAyran(long int philosopherID);
void waitAndEat(long int philosopherID);
void pickupChopstick(long int philosopherID);
void putdownChopstick(long int philosopherID);
void testPhilosopherCanEat(int order);
int findPhilosopher(long int philosopherID);
void printInfo();
void* setTurn();
void init();

typedef enum {ayranWaiting, hungry, eating, thinking} philosopherState;

struct mutexInfo {
    pthread_t* tid;
    pthread_mutex_t ayranMutex;
    pthread_mutex_t chopstickMutex;
    pthread_mutex_t garsonLock;
    pthread_mutex_t garsonCallLock;
    pthread_cond_t garsonCondition;
    pthread_cond_t newTurnWait;
    pthread_cond_t* eatingCondition;
} mutexInfo;

philosopherState* states;
int* eatCount;
int philosopherCount;
int ayranLeft;
int totalAyran;
int processNumber = 0;

void startDineForNPhilosophers() {
    pthread_t tid;
    pthread_create(&tid, NULL, setTurn, (void*)&tid);

    for (int i = 0; i < philosopherCount; i++)
        pthread_create(&(mutexInfo.tid[i]), NULL, startDine, (void*)&(mutexInfo.tid[i]));

    for (int i = 0; i < philosopherCount; i++)
        pthread_join(mutexInfo.tid[i], NULL);
}

void* setTurn() {
    while(true) {
        pthread_mutex_lock(&mutexInfo.garsonLock);
        pthread_cond_wait(&mutexInfo.garsonCondition, &mutexInfo.garsonLock);

        printInfo();

        initNewTurn();

        sleep(5);

        for (int i = 0; i < philosopherCount; i++)
            pthread_cond_signal(&mutexInfo.newTurnWait);
        pthread_mutex_unlock(&mutexInfo.garsonLock);
    }
}

void* startDine(void* philosopherID) {
    long int id = *((long int *)philosopherID);
    int order = findPhilosopher(id);
    while (true) {
        dine(id);
        pthread_mutex_lock(&mutexInfo.garsonCallLock);
        if (++processNumber == philosopherCount)
            pthread_cond_signal(&mutexInfo.garsonCondition);
        pthread_cond_wait(&mutexInfo.newTurnWait, &mutexInfo.garsonCallLock);
        pthread_mutex_unlock(&mutexInfo.garsonCallLock);
    }
}

void dine(long int philosopherID) {
    while(ayranLeft > 0) {
        diningCycle(philosopherID);
    }
}

void diningCycle(long int philosopherID) {
    bool hasAyran = waitAndGetAyran(philosopherID);
    if (hasAyran)
        waitAndEat(philosopherID);
}

bool waitAndGetAyran(long int philosopherID) {
    states[findPhilosopher(philosopherID)] = ayranWaiting;
    pthread_mutex_lock(&(mutexInfo.ayranMutex));
    if (ayranLeft <= 0) {
        pthread_mutex_unlock(&(mutexInfo.ayranMutex));
        return false;
    }
    ayranLeft--;
    pthread_mutex_unlock(&(mutexInfo.ayranMutex));
    return true;
}

void waitAndEat(long int philosopherID) {
    pickupChopstick(philosopherID);
    int order = findPhilosopher(philosopherID);
    eatCount[order]++;
    putdownChopstick(philosopherID);
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

void printInfo() {
    printf("---------------------------\n");
    printf("Surahide %d tas ayran var.\n\n", totalAyran);
    for (int i = 0; i < philosopherCount; i++)
        printf("Process %d, %d porsiyon yedi.\n", i, eatCount[i]);
}

void init() {
    states = (philosopherState*) malloc(sizeof(philosopherState) * philosopherCount);
    mutexInfo.tid = (pthread_t*) malloc(sizeof(pthread_t) * philosopherCount);
    mutexInfo.eatingCondition = (pthread_cond_t*) malloc(sizeof(pthread_cond_t) * philosopherCount);
    eatCount = (int*) malloc(sizeof(int) * philosopherCount);

    pthread_mutex_init(&(mutexInfo.ayranMutex), NULL);
    pthread_mutex_init(&(mutexInfo.chopstickMutex), NULL);
    pthread_mutex_init(&(mutexInfo.garsonLock), NULL);
    pthread_mutex_init(&(mutexInfo.garsonCallLock), NULL);

    pthread_cond_init(&mutexInfo.garsonCondition, NULL);
    pthread_cond_init(&mutexInfo.newTurnWait, NULL);

    for (int i = 0; i < philosopherCount; i++)
        pthread_cond_init(&((mutexInfo.eatingCondition)[i]), NULL);
}

void initNewTurn() {
    for(int i = 0;i < philosopherCount; i++) {
        eatCount[i] = 0;
        states[i] = thinking;
    }
    totalAyran = ayranLeft = rand() % (5 * philosopherCount) + 1;
    processNumber = 0;
}

int main() {
    srand(time(0));

    printf("Enter the number of philosophers: ");
    scanf("%d", &philosopherCount);

    init();
    initNewTurn();

    startDineForNPhilosophers();

    return 0;
}