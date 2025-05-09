#define _GNU_SOURCE
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
#include <string.h>

typedef short bool;
#define true 1
#define false 0

#define SHKEY 300
#define MSG_KEY 0x1234

#define maxMemory 1024
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
    int memsize;
} ProcessMsg;

typedef struct
{
    processNode *frontptr;
    processNode *backptr;
} AllProcessesQueue;


enum schedulealgo
{
    HPF = 0,
    SRTN = 1,
    RR = 2
};

enum pstatus
{
    RUNNING = 0,
    WAITING = 1
};

typedef struct
{
    // processNode* process;
    enum pstatus status;
    int givenid;
    pid_t systemid;
    int arrivalTime;
    int executionTime;
    int remainingTime;
    int waitingTime;
    int priority;
    int memsize;
    int memStart;
    int memEnd;
} pcb;

/*
The struct of the Memory be like a binary tree 
*/
typedef struct Memory_Block
{
    int start; // The start index of the new block
    int size; // The block size
    pcb *process; // PCB pointer to the process
    bool is_free; // Indicates if the block is free or not
    bool is_used;
    struct Memory_Block * left; // Pointer to the left chunk
    struct Memory_Block * right; // Pointer to the right chunk
} Memory_Block;


// Initializing the memory block givin the start index and it's size
Memory_Block* memory_init(int start, int size){ 
    Memory_Block *head = (Memory_Block*)malloc(sizeof(Memory_Block));
    head->start = start;
    head->size = size;
    head->is_free = true;
    head->is_used = false;
    head->process = NULL;
    head->left = NULL;
    head->right = NULL;
    return head;
}
 
// Trying to allocate a new memory for the new process
Memory_Block* allocate_memory(pcb *newProcess, Memory_Block *head, char place){
    //1. Check if the current block acceptable to put the chunk in it.
    if(head == NULL || head->size < newProcess->memsize){
        return NULL;
    }
    
    //2. Check if it possible for the process fits in the child 
    if(head->is_used == false &&head->is_free && head->size/2 < newProcess->memsize){ //process can't fit in the child -> allocate in current block
        head->is_free = false;
        head->is_used = true ;
        head->process = newProcess;
        newProcess->memStart = head->start;
        newProcess->memEnd = head->start + head->size -1 ;
        printf("Allocated at size %d: %c\n", head->size, place);
        return head;
    } else if (head->is_used == true){return NULL ;}
    else if(head->size/2 >= newProcess->memsize){ //if it can fit in the child, check if it is free or not
        if(!head->left || !head->right){ // If the left and right blocks are not initialized, initialize them
            head->left = memory_init(head->start, head->size/2);
            head->right = memory_init(head->start + head->size/2, head->size/2);
        }
        head->is_free = false; // Mark the current block as not free
        Memory_Block* newBlock = allocate_memory(newProcess, head->left, 'L');
        if(! newBlock){
            newBlock = allocate_memory(newProcess, head->right, 'R');
        }
        return newBlock;
    }
    return NULL;
}

void free_memory(Memory_Block* head, int pid){
    if(!head) return;
    
    if(head->process && head->process->systemid == pid){
        head->is_free = true;
        head->is_used = false;
        return;
    }

    free_memory(head->left, pid);
    free_memory(head->right, pid);

    if(head->left && head->right && head->left->is_free && head->right->is_free){
        free(head->left);
        free(head->right);
        head->left = NULL;
        head->right = NULL;
        head->is_free = true;
        head->is_used = false;
    }
}

processNode *create_process(int id, int arrival, int runtime, int priority, int memoSize)
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
    p->memsize = memoSize;
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


// pcb* pcb_init(processNode* processPtr, int sysid){
    //     pcb* pcbobj;
    //     pcbobj->givenid = processPtr->processID;
    //     pcbobj->systemid = sysid; //creates process
    //     //pcbobj->process = processPtr;
    //     pcbobj->arrivalTime = processPtr->arrivalTime;
    //     pcbobj->executionTime = 0;
    //     pcbobj->remainingTime = processPtr->runTime;
    //     pcbobj->waitingTime = 0;
    //     pcbobj->priority = processPtr->priority;
    //     return pcbobj;
    // };
    
    pcb pcb_init(ProcessMsg *processmsg, int sysid)
    {
        pcb pcbobj;
        pcbobj.givenid = processmsg->id;
        pcbobj.systemid = sysid; // Set to the forked process ID
        pcbobj.status = WAITING;
        pcbobj.arrivalTime = processmsg->arrivalTime;
        pcbobj.executionTime = processmsg->runTime;
        pcbobj.remainingTime = processmsg->runTime;
        pcbobj.waitingTime = 0;
        pcbobj.priority = processmsg->priority;
        pcbobj.memsize = processmsg->memsize;
        pcbobj.memStart=0;
        pcbobj.memEnd = 0;
        return pcbobj;
    };
    // note it need a lot of improvments I know it is not right I am making the first version of the code and I will improve it later
    //  Define the maximum size of the queue
    #define CirQ_SIZE 1000
    
    typedef struct
    {
        pcb *queue[CirQ_SIZE];
        int front;
        int rear;
    } CircularQueue;
    
    // Function to check if the queue is full
    int isFullCir(CircularQueue *q)
    {
        // If the next position is the front, the queue is full
        return (q->rear + 1) % CirQ_SIZE == q->front;
    }
    
    // Function to check if the queue is empty
    int isEmptyCir(CircularQueue *q)
    {
        // If the front hasn't been set, the queue is empty
        return q->front == -1;
    }
    int nextIndex(int i)
    {
        return (i + 1) % CirQ_SIZE;
    }
    // Function to enqueue (insert) an element
    void enqueueCir(CircularQueue *q, pcb *data)
    {
        // If the queue is full, print an error message and
        // return
        if (isFullCir(q))
        {
            printf("Queue overflow\n");
            return;
        }
        // If the queue is empty, set the front to the first
        // position
        if (q->front == -1)
        {
            q->front = 0;
        }
        // Add the data to the queue and move the rear pointer
        q->rear = (q->rear + 1) % CirQ_SIZE;
        q->queue[q->rear] = data;
        // printf("Element %d inserted\n", data.process->processID);
    }
    
    // Function to dequeue (remove) an element
    pcb *dequeueCir(CircularQueue *q)
    {
        // If the queue is empty, print an error message and
        // return -1
        if (isEmptyCir(q))
        {
            // printf("Queue underflow\n"); //for now leave it like this
            return NULL;
        }
        // Get the data from the front of the queue
        pcb *data = q->queue[q->front];
        // If the front and rear pointers are at the same
        // position, reset them
        if (q->front == q->rear)
        {
            q->front = q->rear = -1;
        }
        else
        {
        // Otherwise, move the front pointer to the next
        // position
        q->front = (q->front + 1) % CirQ_SIZE;
    }
    // Return the dequeued data
    return data;
}

typedef struct PCBNode
{
    pcb PCB;
    struct PCBNode *next;
} PCBNode;

typedef struct
{
    PCBNode *head;
} PCBPriQ;

// Initialize the priority queue
PCBPriQ *PCBPriQ_init()
{
    PCBPriQ *queue = malloc(sizeof(PCBPriQ));
    if (!queue)
    {
        printf("Memory allocation failed\n");
        exit(1);
    }
    queue->head = NULL;
    return queue;
}

// Insert a PCB into the priority queue
void PCBPriQ_enqueue(PCBPriQ *queue, pcb *newPCB)
{
    PCBNode *newNode = malloc(sizeof(PCBNode));
    if (!newNode)
    {
        printf("Memory allocation failed\n");
        exit(1);
    }
    newNode->PCB = *newPCB;
    newNode->next = NULL;
    
    // If the queue is empty, just add the new PCB
    if (queue->head == NULL)
    {
        queue->head = newNode;
        return;
    }
    
    // If the head process is RUNNING, we must not insert before it
    if (queue->head->PCB.status == RUNNING)
    {
        // Insert after head, even if new process has higher priority
        if (queue->head->next == NULL)
        {
            // If head is the only element
            queue->head->next = newNode;
        }
        else
        {
            // Find appropriate position after head based on priority
            PCBNode *current = queue->head;
            while (current->next != NULL && current->next->PCB.priority <= newPCB->priority)
            {
                current = current->next;
            }
            newNode->next = current->next;
            current->next = newNode;
        }
    }
    // If head is not RUNNING, use normal priority-based insertion
    else if (newPCB->priority < queue->head->PCB.priority)
    {
        // New PCB has higher priority than head
        newNode->next = queue->head;
        queue->head = newNode;
    }
    else
    {
        // Traverse the list to find correct position based on priority
        PCBNode *current = queue->head;
        while (current->next != NULL && current->next->PCB.priority <= newPCB->priority)
        {
            current = current->next;
        }
        newNode->next = current->next;
        current->next = newNode;
    }
}
// Remove the highest priority PCB from the queue
pcb PCBPriQ_dequeue(PCBPriQ *queue)
{
    if (queue->head == NULL)
    {
        fprintf(stderr, "Queue underflow\n");
        exit(1);
    }
    PCBNode *temp = queue->head;
    pcb highestPriorityPCB = temp->PCB;
    queue->head = queue->head->next;
    free(temp);
    return highestPriorityPCB;
}

// Peek at the highest priority PCB without removing it
pcb *PCBPriQ_peek(PCBPriQ *queue)
{
    if (queue->head == NULL)
    {
        return NULL; // Queue is empty
    }
    return &queue->head->PCB;
}

// Check if the priority queue is empty
int PCBPriQ_isEmpty(PCBPriQ *queue)
{
    return queue->head == NULL;
}

// Clear the priority queue
void PCBPriQ_clear(PCBPriQ *queue)
{
    while (!PCBPriQ_isEmpty(queue))
    {
        PCBPriQ_dequeue(queue);
    }
    free(queue);
}

// Print the givenid of all PCBs in the priority queue in the format: P1 -> P2 -> P3 where P1 is the head
void PCBPriQ_printGivenIDs(PCBPriQ *queue)
{
    PCBNode *current = queue->head;
    printf("PCBPriQ: ");
    while (current != NULL)
    {
        printf("P%d(%c)", current->PCB.givenid, current->PCB.status == RUNNING ? 'R' : 'W');
        if (current->next != NULL)
        {
            printf(" -> ");
        }
        current = current->next;
    }
    printf("\n");
}

/*Here I assume the first process with the low remaining time will be at index 0
*/
#define MAX 100
typedef struct
{
    pcb **Process;
    int size;
    int current_capacity;
} SRTN_PriQueue;

SRTN_PriQueue *SRTN_PriQueue_init(int capacity)
{
    SRTN_PriQueue *pq = malloc(sizeof(SRTN_PriQueue));
    if (!pq)
    {
        perror("Failed to allocate SRTN_PriQueue");
        exit(1);
    }
    pq->Process = malloc(capacity * sizeof(pcb *));
    if (!pq->Process)
    {
        perror("Failed to allocate process array");
        free(pq);
        exit(1);
    }
    pq->size = 0;
    pq->current_capacity = capacity;
    return pq;
}

void SRTN_PriQueue_insert(SRTN_PriQueue *pq, pcb *p)
{ // insert at the Queue if there is free places otherwise return
    if (pq->size >= pq->current_capacity)
    {
        printf("Queue is Full....");
        return;
    }

    int i = pq->size - 1;
    while (i >= 0 && pq->Process[i]->remainingTime > p->remainingTime)
    { // Loop from the end of the Queue till you find your proper place to insert your self
        pq->Process[i + 1] = pq->Process[i];
        i--;
    }
    pq->Process[i + 1] = p;
    pq->size++;
    printf("Inserted process ID=%d with remainingTime=%d into SRTN queue\n", p->givenid, p->remainingTime);
}

void RR_insert(CircularQueue *queue, pcb *data)
{ // insert at the Queue if there is free places otherwise return
    if (isFullCir(queue))
    {
        printf("Queue overflow\n");
        return;
    }
    enqueueCir(queue, data);
}

pcb *SRTN_PriQueue_pop(SRTN_PriQueue *pq)
{ // Getting the
    if (pq->size == 0)
    {
        printf("Queue is empty");
        return NULL;
    }
    pcb *p = pq->Process[0];
    
    for (int i = 1; i < pq->size; i++)
    {
        pq->Process[i - 1] = pq->Process[i];
    }
    pq->size--;
    return p;
}

pcb *SRTN_PriQueue_peek(SRTN_PriQueue *pq)
{
    if (pq->size == 0)
    {
        printf("Queue is empty");
        return NULL;
    }
    return pq->Process[0];
}

void SRTN_PriQueue_free(SRTN_PriQueue *pq)
{
    free(pq->Process);
    free(pq);
}


typedef struct Blocked_Processes{
    PCBNode *head;
} Blocked_Processes;

Blocked_Processes* initialize_blockedQueue(){
    Blocked_Processes *BP = malloc(sizeof(Blocked_Processes));
    if(!BP){
        perror("Error in initializing Blocked_Processes");
        exit(1);
    }
    BP->head = NULL;
    return BP;
}

void blockedQueue_enqueue(Blocked_Processes *BP, pcb* process){
    PCBNode *newNode = malloc(sizeof(PCBNode));
    if (!newNode)
    {
        printf("Memory allocation failed\n");
        exit(1);
    }
    newNode->PCB = *process;
    newNode->next = NULL;
    
    // If the queue is empty, just add the new PCB
    if (BP->head == NULL)
    {
        BP->head = newNode;
        return;
    }
    
    PCBNode *walker = BP->head;
    while(walker->next){
        walker = walker->next;
    }
    walker->next = newNode;
    newNode->next = NULL;
}

pcb blockedQueue_dequeue(Blocked_Processes* BP){
    if (BP->head == NULL){
        fprintf(stderr, "Queue underflow\n");
        exit(1);
    }
    PCBNode *temp = BP->head;
    pcb process = temp->PCB;
    BP->head = BP->head->next;
    free(temp);
    return process;
}

pcb *blockedQueue_peek(Blocked_Processes *queue)
{
    printf("Entering Peek in blocked Queue\n");
    if (queue->head == NULL)
    {
        printf("The Head is NULL in Blocked Queue\n");
        return NULL; // Queue is empty
    }
    return &queue->head->PCB;
}

// Check if the queue is empty
int blockedQueue_isEmpty(Blocked_Processes *queue)
{
    return queue->head == NULL;
}

// Clear the  queue
void blockedQueue_clear(Blocked_Processes *queue)
{
    while (!blockedQueue_isEmpty(queue))
    {
        blockedQueue_dequeue(queue);
    }
    free(queue);
}