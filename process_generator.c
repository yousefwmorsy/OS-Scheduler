#include "headers.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

#define MAX_LINE_LENGTH 256
typedef struct
{
    int id;
    int arrival;
    int runtime;
    int priority;
} Process;

void clearResources(int);
AllProcessesQueue *queue;
AllProcessesQueue *SentQueue;

int main(int argc, char *argv[])
{
    signal(SIGINT, clearResources);
    // TODO Initialization
    // 1. Read the input files.
    FILE *inputfile = fopen("processes.txt", "r");
    if (inputfile == NULL)
    {
        printf("Error opening file");
    }

    //dynamic allocation of the queue
    queue = malloc(sizeof(AllProcessesQueue));


    char line[MAX_LINE_LENGTH];
    queue->frontptr = NULL;
    queue->backptr = NULL;

    //reads the file line by line and creates a process node for each line and enqueues it to the queue
    while (fgets(line, sizeof(line), inputfile))
    {

        if (line[0] == '#')
        {
            continue;
        }
        int id, arrival, runtime, priority;

        if (sscanf(line, "%d\t%d\t%d\t%d", &id, &arrival, &runtime, &priority) == 4)
        {
            processNode *newprocess = create_process(id, arrival, runtime, priority);
            enqueue(queue, newprocess);
        }
    }
    fclose(inputfile);
    // 2. Ask the user for the chosen scheduling algorithm and its parameters, if there are any.

    printf("Please choose a scheduling algorithm:\n");
    printf("0. HPF\t");
    printf("1. SRTN\t");
    printf("2. RR\t");
    int algo = -1;
    int timeQuantum;
    while (algo < 0 || algo > 2)
    {
        printf("Please enter the number of the algorithm you want to use: ");
        scanf("%d", &algo);
        if (algo < 0 || algo > 2)
        {
            printf("Invalid choice. Please try again.\n");
        }
    }
    if (algo == 2)
    {
        printf("Please enter the time quantum: ");
        scanf("%d", &timeQuantum);
    }

    // 3. Initiate and create the scheduler and clock processes.
    int clock = fork();
    if (clock == 0)
    {
        execl("./Compiled/clk.out", "./Compiled/clk.out", NULL);
        perror("clock execl failed\n");
    };

    int scheduler = fork();
    if (scheduler == -1)
    {
        printf("scheduler fork failed\n");
    }
    else if (scheduler == 0)
    {
        execl("./Compiled/scheduler.out", "./Compiled/scheduler.out", algo, timeQuantum, NULL);

        printf("scheduler execl failed\n");
    }
    // 4. Use this function after creating the clock process to initialize clock
    initClk();


    // To get time use this
    int x = getClk();
    //  printf("current time is %d\n", x);
    printf("current time is %d\n", x);
    // TODO Generation Main Loop

    
    // 5. Create a data structure for processes and provide it with its parameters.
    // created the queue in step 1

    // 6. Send the information to the scheduler at the appropriate time.
    int MessageQueueId = msgget(MSG_KEY, IPC_CREAT | 0666);

    processNode *Curr;
    SentQueue = malloc(sizeof(AllProcessesQueue));

    SentQueue->frontptr = NULL;
    SentQueue->backptr = NULL;
    while (!isEmpty(queue))
    {
        //for debugging purposes
        // int x = getClk();
        // printf("current time is %d\n", x);

        Curr = dequeue(queue);
        enqueue(SentQueue, Curr);
        while (Curr->arrivalTime > getClk())
        {
        }

        ProcessMsg msg;
        msg.mtype = 1;
        msg.id = Curr->processID;
        msg.arrivalTime = Curr->arrivalTime;
        msg.runTime = Curr->runTime;
        msg.priority = Curr->priority;
        if (msgsnd(MessageQueueId, &msg, sizeof(ProcessMsg), 0) == -1)
        {
            printf("msgsnd failed");
            exit(1);
        }
    }
    // Send a message to indicate that all processes have been sent to the scheduler
    ProcessMsg msg;
    msg.mtype = 2;
    msgsnd(MessageQueueId, &msg, sizeof(ProcessMsg), 0) ;
    
    printf("All processes sent to the scheduler\n");

    waitpid(scheduler, NULL, 0);
    printf("Scheduler terminated\n");

    // 7. Clear clock resources
    destroyClk(true);
}

void clearResources(int signum)
{
    // TODO Clears all resources in case of interruption
    // ClearQueue(queue);
    ClearQueue(SentQueue);
    free(queue);
    free(SentQueue);
    destroyClk(true);
    exit(0);
}
