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

processNode *create_process(int id, int arrival, int runtime, int priority)
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


enum schedulealgo {
    HPF = 0,
    SRTN = 1,
    RR = 2
};

enum pstatus {
    RUNNING = 0,
    WAITING = 1
};

typedef struct {
    processNode* process;
    enum pstatus status;
    int givenid;
    pid_t sysstemid;
    int arrivalTime;
    int executionTime;
    int remainingTime;
    int waitingTime;
    int priority;
} pcb;

pcb* pcb_init(processNode* processPtr){
    pcb* pcbobj;
    pcbobj->givenid = processPtr->processID;
    pcbobj->sysstemid = fork(); //creates process
    pcbobj->process = processPtr;
    pcbobj->arrivalTime = processPtr->arrivalTime;
    pcbobj->executionTime = 0;
    pcbobj->remainingTime = processPtr->runTime;
    pcbobj->waitingTime = 0;
    pcbobj->priority = processPtr->priority;
    return pcbobj;
};
//note it need a lot of improvments I know it is not right I am making the first version of the code and I will improve it later
// Define the maximum size of the queue
#define CirQ_SIZE 1000

typedef struct {
pcb *queue[CirQ_SIZE];
int front ;
int rear ;
} CircularQueue;

// Function to check if the queue is full
int isFullCir(CircularQueue* q)
{
    // If the next position is the front, the queue is full
    return (q->rear + 1) % CirQ_SIZE == q->front;
}

// Function to check if the queue is empty
int isEmptyCir(CircularQueue* q)
{
    // If the front hasn't been set, the queue is empty
    return q->front == -1;
}

// Function to enqueue (insert) an element
void enqueueCir(CircularQueue* q, pcb *data)
{
    // If the queue is full, print an error message and
    // return
    if (isFullCir(q)) {
        printf("Queue overflow\n");
        return;
    }
    // If the queue is empty, set the front to the first
    // position
    if (q->front == -1) {
        q->front = 0;
    }
    // Add the data to the queue and move the rear pointer
    q->rear = (q->rear + 1) % CirQ_SIZE;
    q->queue[q->rear] = data;
    //printf("Element %d inserted\n", data.process->processID);
}


// Function to dequeue (remove) an element
pcb *dequeueCir(CircularQueue* q)
{
    // If the queue is empty, print an error message and
    // return -1
    if (isEmptyCir(q)) {
        printf("Queue underflow\n");
        return NULL;
    }
    // Get the data from the front of the queue
    pcb *data = q->queue[q->front];
    // If the front and rear pointers are at the same
    // position, reset them
    if (q->front == q->rear) {
        q->front = q->rear = -1;
    }
    else {
        // Otherwise, move the front pointer to the next
        // position
        q->front = (q->front + 1) % CirQ_SIZE;
    }
    // Return the dequeued data
    return data;
}