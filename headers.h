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
#include <math.h>

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
    //processNode* process;
    enum pstatus status;
    int givenid;
    pid_t sysstemid;
    int arrivalTime;
    int executionTime;
    int remainingTime;
    int waitingTime;
    int priority;
} pcb;

// pcb* pcb_init(processNode* processPtr, int sysid){
//     pcb* pcbobj;
//     pcbobj->givenid = processPtr->processID;
//     pcbobj->sysstemid = sysid; //creates process
//     //pcbobj->process = processPtr;
//     pcbobj->arrivalTime = processPtr->arrivalTime;
//     pcbobj->executionTime = 0;
//     pcbobj->remainingTime = processPtr->runTime;
//     pcbobj->waitingTime = 0;
//     pcbobj->priority = processPtr->priority;
//     return pcbobj;
// };

pcb pcb_init(ProcessMsg* processmsg, int sysid){
    pcb pcbobj;
    pcbobj.givenid = processmsg->id;
    pcbobj.sysstemid = -1;
    //pcbobj->process = 
    pcbobj.arrivalTime = processmsg->arrivalTime;
    pcbobj.executionTime = 0; 
    pcbobj.remainingTime = processmsg->runTime;
    pcbobj.waitingTime = 0;
    pcbobj.priority = processmsg->priority;
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
        // printf("Queue underflow\n"); //for now leave it like this
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

/*Here I assume the first process with the low remaining time will be at index 0
*/
#define MAX 100
typedef struct
{
    pcb *Process[MAX];
    int size;
} PriQueue;

void PriQueue_insert(PriQueue *pq ,pcb *p){ // insert at the Queue if there is free places otherwise return
    if(pq->size >= MAX){
        printf("Queue is Full....");
        return;
    }

    int i = pq->size - 1;
    while(i >= 0 && pq->Process[i]->remainingTime > p->remainingTime){ //Loop from the end of the Queue till you find your proper place to insert your self
        pq->Process[i + 1] = pq->Process[i];
        i--;
    }
    pq->Process[i+1] = p;
    pq->size ++;
}

pcb* PriQueue_pop(PriQueue *pq) { // Getting the 
    if (pq->size == 0) {
        printf("Queue is empty");
        return NULL;
    }
    pcb *p = pq->Process[0];

    for (int i = 1; i < pq->size; i++) {
        pq->Process[i - 1] = pq->Process[i];
    }
    pq->size--;
    return p;
}

pcb* PriQueue_peek(PriQueue *pq) {
    if (pq->size == 0) {
        printf("Queue is empty");
        return NULL;
    }
    return pq->Process[0];
}