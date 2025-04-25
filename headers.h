#include <stdio.h> //if you don't use scanf/printf change this include
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

typedef short bool;
#define true 1
#define false 0

#define SHKEY 300
#define MSG_KEY 0x1234

///==============================
// don't mess with this variable//
int *shmaddr; //
//===============================

int getClk()
{
    return *shmaddr;
}

/*
 * All process call this function at the beginning to establish communication between them and the clock module.
 * Again, remember that the clock is only emulation!
 */
void initClk()
{
    int shmid = shmget(SHKEY, 4, 0444);
    while ((int)shmid == -1)
    {
        // Make sure that the clock exists
        printf("Wait! The clock not initialized yet!\n");
        sleep(1);
        shmid = shmget(SHKEY, 4, 0444);
    }
    shmaddr = (int *)shmat(shmid, (void *)0, 0);
}

/*
 * All process call this function at the end to release the communication
 * resources between them and the clock module.
 * Again, Remember that the clock is only emulation!
 * Input: terminateAll: a flag to indicate whether that this is the end of simulation.
 *                      It terminates the whole system and releases resources.
 */

void destroyClk(bool terminateAll)
{
    shmdt(shmaddr);
    if (terminateAll)
    {
        killpg(getpgrp(), SIGINT);
    }
}
typedef struct processNode
{
    int processID;
    int arrivalTime;
    int runTime;
    int priority;
    int memsize;
    struct processNode *next;
} processNode;
typedef struct
{
    long mtype;
    int id;
    int arrivalTime;
    int runTime;
    int priority;
} ProcessMsg;

typedef struct
{
    processNode *frontptr;
    processNode *backptr;
} AllProcessesQueue;

struct ProcessNode *create_process(int id, int arrival, int runtime, int priority)
{
    processNode *p = malloc(sizeof(processNode));
    if (!p)
    {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    p->processID = id;
    p->arrivalTime = arrival;
    p->runTime = runtime;
    p->priority = priority;
    p->next = NULL;
    return p;
};

void enqueue(AllProcessesQueue *queue, processNode *newprocess)
{
    if (queue->backptr == NULL)
    {
        queue->frontptr = newprocess;
        queue->backptr = newprocess;
    }
    else
    {
        queue->backptr->next = newprocess;
        queue->backptr = newprocess;
    }
};
processNode *dequeue(AllProcessesQueue *queue)
{
    if (queue->frontptr == NULL)
    {
        return NULL;
    }
    processNode *temp = queue->frontptr;
    queue->frontptr = queue->frontptr->next;
    if (queue->frontptr == NULL)
    {
        queue->backptr = NULL;
    }
    return temp;
};
bool isEmpty(AllProcessesQueue *q)
{
    if (q == NULL)
        return true;
    return (q->frontptr == NULL);
}
void ClearQueue(AllProcessesQueue *q)
{
    if (q == NULL)
        return;

    while (!isEmpty(q))
    {
        processNode *current = dequeue(q);
        free(current);
    }
    q->frontptr = NULL;
    q->backptr = NULL;
}