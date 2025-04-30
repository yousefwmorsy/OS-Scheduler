#include "headers.h"
#include <math.h>
#define maxProcess 1000 // will change and it later and probably remove it from the code
float WTA = 0;          // waiting time of the process
int WT = 0;             // turnaround time of the process
int TotalTime = 0;      // execution time of the process
int countGlobal = 0;    // number of processes in the system
int IdleTime = 0;
PCBPriQ *PriQ;             // ready queue for HPF
SRTN_PriQueue *SRTN_Queue; // Priority Queue for SRTN
CircularQueue *queue;      // queue for RR
pcb *pcbtempRR;            // process that is running now
CircularQueue *queue;      // queue for RR
pcb *pcbtempRR;            // process that is running now

enum schedulealgo algo; // algorithm type
// SRTN_PriQueue *SRTN_Queue; // Priority Queue for SRTN
int MessageQueueId;
bool readyQNotEmpty(int algo)
{
    switch (algo)
    {
    case HPF:
        // printf("Q Empty %d", PCBPriQ_isEmpty(PriQ));
        return (PCBPriQ_isEmpty(PriQ) == 0);
        break;
    case SRTN:
        return (SRTN_Queue->size != 0);
        break;
    case RR:
        return (isEmptyCir(queue) != 1);
        break;
    }
}

void schedulerlogPrint(FILE *fp, int currentTime, pcb *pcb, char status[])
// AHMED ABD-ELjALeel!!!!!!!! please use this and don't waste your time and copy the arguments from the use of the function in roundRobin so you don't waste time filling the arguments
{
    if (fp == NULL)
    {
        printf("Error opening file!\n");
        return;
    }
    int processID = pcb->givenid;
    int arrivalTime = pcb->arrivalTime;
    int totalTime = pcb->executionTime;
    int remainingTime = pcb->remainingTime;
    int waitingTime = pcb->waitingTime;
    if (strcmp(status, "started") == 0)
    {
        WT += waitingTime;
        TotalTime += totalTime;
    }

    if (strcmp(status, "finished") == 0)
    {
        int turnaroundTime = currentTime - arrivalTime;
        float weightedTA = (float)turnaroundTime / totalTime;
        WTA += weightedTA;
        fprintf(fp, "At time  %d process %d %s arr %d total %d remain %d wait %d TA %d WTA %.2f\n", currentTime, processID, status, arrivalTime, totalTime, remainingTime, waitingTime, turnaroundTime, weightedTA);
        printf("At time  %d process %d %s arr %d total %d remain %d wait %d TA %d WTA %.2f\n", currentTime, processID, status, arrivalTime, totalTime, remainingTime, waitingTime, turnaroundTime, weightedTA);
    }
    else
    {
        fprintf(fp, "At time  %d process %d %s arr %d total %d remain %d wait %d \n", currentTime, processID, status, arrivalTime, totalTime, remainingTime, waitingTime);
        printf("At time  %d process %d %s arr %d total %d remain %d wait %d \n", currentTime, processID, status, arrivalTime, totalTime, remainingTime, waitingTime);
    }
}

void schedulerPrefPrint(FILE *fp)
{ // missing std WTA I am to tired to do it now
    int currentTime = getClk();
    float utilization = (TotalTime / (float)currentTime) * 100;
    float averageWT = (float)WT / countGlobal;
    float averageWTA = (float)WTA / countGlobal;
    averageWT = roundf(averageWT * 100) / 100;
    averageWTA = roundf(averageWTA * 100) / 100;
    fprintf(fp, "CPU utilization = %.0f%%\n", utilization);
    fprintf(fp, "Avg WTA = %.2f\n", averageWTA);
    fprintf(fp, "Avg waiting = %.2f\n", averageWT);
    // fprintf(fp,"Std WTA = %.2f\n", stdWTA);
}

bool checkifEnd(int msg_q)
{
    ProcessMsg msg;
    // printf("Checked if end\n");
    if (msgrcv(msg_q, &msg, sizeof(ProcessMsg) - sizeof(long), 2, IPC_NOWAIT) != -1)
    { // wait for msg of type 2
        // printf("Returned true \n");
        return true;
    }
    // printf("returned false \n");
    return false;
}

void checkforNewProcesses(int msg_q, int algo)
{
    ProcessMsg msg;
    // printf("Checking for new processes...\n");
    while (msgrcv(msg_q, &msg, sizeof(ProcessMsg) - sizeof(long), 1, IPC_NOWAIT) != -1)
    { // wait for msg of type 1 (process)
        printf("Received process message: ID=%d, Arrival=%d, Runtime=%d, Priority=%d\n", msg.id, msg.arrivalTime, msg.runTime, msg.priority);
        int pid = fork();
        if (pid == -1)
        {
            perror("Process fork failed\n");
            return;
        }
        else if (pid == 0)
        {
            // Use absolute path for process.out to avoid path issues
            char RunTime[5];
            sprintf(RunTime, "%d", msg.runTime);
            char ID[5];
            sprintf(ID, "%d", msg.id);
            execl("./Compiled/process.out", "process.out", RunTime, ID, (char *)NULL);
            perror("Process execl failed\n");
            exit(1);
        }
        else
        {
            printf("Forked process with PID=%d\n", pid);
            kill(pid, SIGSTOP);
            pcb *obj = malloc(sizeof(pcb));
            if (obj == NULL)
            {
                perror("Memory allocation for PCB failed\n");
                return;
            }
            pcb tempPcb = pcb_init(&msg, pid);
            memcpy(obj, &tempPcb, sizeof(pcb));

            // add to suitable ds
            switch (algo)
            {
            case HPF:
                countGlobal++;
                PCBPriQ_enqueue(PriQ, obj);
                printf("Enqueued process ID=%d into priority queue\n", obj->givenid);
                break;
            case SRTN:
                printf("SRTN: Entered!!");
                countGlobal++;
                SRTN_PriQueue_insert(SRTN_Queue, obj);
                printf("Enqueued process ID=%d into SRTN queue\n", obj->givenid);
                break;
            case RR:
                countGlobal++;
                RR_insert(queue, obj); // pcbtempRR is the process that is running now
                break;
            }
        }
    }
    if (algo == RR)
    {
        if (pcbtempRR != NULL)
        {
            RR_insert(queue, pcbtempRR); // pcbtempRR is the process that is running now
            pcbtempRR = NULL;            // Reset the pointer to avoid re-adding the same process
        }
    }
}

void SRTN_func(FILE *fp, FILE *fp2)
{
    // 1.Check if any process enqueued in the Queue or not
    if (SRTN_Queue->size == 0)
    {
        printf("SRTN: No process in the Queue yet...\n");
        IdleTime++;
        return;
    }

    // 2. Peek the SRTN Process in the Priority Queue
    pcb *current_process = SRTN_PriQueue_peek(SRTN_Queue);

    // 3. Start The Process if its first time to start
    if (current_process->remainingTime == current_process->executionTime)
    {

        schedulerlogPrint(fp, getClk(), current_process, "started");
    }

    // 4. Increment the waiting time of the other Processes
    for (int i = 0; i < SRTN_Queue->size; i++)
    {
        if (SRTN_Queue->Process[i] != current_process)
            SRTN_Queue->Process[i]->waitingTime++; // Here it will increment the waiting time of any process which now doesn't take the process.
    }

    // 5. Decrement the remaining time of the peeked Process
    current_process->remainingTime--;
    kill(current_process->systemid, SIGCONT); // run the process
    // 6. Check if its fished or not, If true pop it and kill it.
    if (current_process->remainingTime == 0)
    {
        printf("SRTN: Process %d Terminated at %d\n", current_process->givenid, getClk());
        schedulerlogPrint(fp, getClk(), current_process, "finished");
        kill(current_process->systemid, SIGCONT); // run the process one last time to finish it
        int turn_around_time = getClk() - current_process->arrivalTime;

        // kill(current_process->systemid, SIGKILL); // temporary for testing purposes
        SRTN_PriQueue_pop(SRTN_Queue);
    }
    else
    {
        // 7. Print the curent states of the current moment of the process
        // kill(current_process->systemid, SIGSTOP); // Stop the process
        printf("SRTN: Running Process %d at %d, remsaining time: %d\n", current_process->givenid, getClk(), current_process->remainingTime);
    }
}

void HPF_Iter(FILE *fp, FILE *fp2)
{
    pcb *head = PCBPriQ_peek(PriQ);
    int currentTime = getClk();
    if (head == NULL)
    {
        IdleTime++;
        return;
    }

    if (head->remainingTime == 0)
    {
        printf("Process %d Terminated at %d\n", head->givenid, getClk());
        schedulerlogPrint(fp, getClk(), head, "finished");
        kill(head->systemid, SIGCONT); // run for one last time to finish it
        PCBPriQ_dequeue(PriQ);
    }
    else
    {
        // Set waiting time when first executing the process
        if (head->remainingTime == head->executionTime)
        {
            head->waitingTime = getClk() - head->arrivalTime;
        }
        if (head->status == WAITING)
        {
            // Continue the process execution
            head->status = RUNNING;
            schedulerlogPrint(fp, getClk(), head, "started");
        }
        while (currentTime + 1 > getClk())
        {
        }
        kill(head->systemid, SIGCONT); // Continue the process execution
        head->remainingTime--;
        printf("Running Process %d at %d, remaining time: %d\n", head->givenid, getClk(), head->remainingTime);
    }
}

void roundRobin(int quantum, FILE *fp) // assuming I am going to get a array of processes this assumption might not be true
{
    pcbtempRR = NULL;

    // printf("Round Robin started\n"); //remove it later
    pcbtempRR = dequeueCir(queue);
    if (pcbtempRR != NULL)
    {
        int currentTime = getClk(); // Time when it get dequeued (start time)

        if (pcbtempRR->waitingTime == 0 && pcbtempRR->executionTime == pcbtempRR->remainingTime) // I think that execution time is the time need for it to finish from the start not sure
        {
            pcbtempRR->waitingTime = currentTime - pcbtempRR->arrivalTime; // Waiting time of the process
            kill(pcbtempRR->systemid, SIGCONT);
            schedulerlogPrint(fp, currentTime, pcbtempRR, "started");
        }

        else

        {
            kill(pcbtempRR->systemid, SIGCONT);
            schedulerlogPrint(fp, currentTime, pcbtempRR, "resumed");
        }

        if (pcbtempRR->remainingTime > quantum)
        {
            pcbtempRR->remainingTime -= quantum;
            while (currentTime + quantum > getClk())
            {
            }
            kill(pcbtempRR->systemid, SIGSTOP); // Stop the process
            schedulerlogPrint(fp, currentTime + quantum, pcbtempRR, "stopped");
        }
        else
        {
            int finishTime = currentTime + pcbtempRR->remainingTime; // End time of the process
            while (currentTime + pcbtempRR->remainingTime > getClk())
            {
            }
            pcbtempRR->remainingTime = 0;
            kill(pcbtempRR->systemid, SIGKILL);
            schedulerlogPrint(fp, finishTime, pcbtempRR, "finished");
            // the function that will be traded for shawarma
            free(pcbtempRR);
            pcbtempRR = NULL;
        }
    }
}
void recieveMess(int signum)
{
    printf("Received signal to check for new processes\n");
    checkforNewProcesses(MessageQueueId, algo);
}

int main(int argc, char *argv[])
{
    initClk();
    // TODO implement the scheduler :)
    // upon termination release the clock resources.
    printf("Scheduler starting\n");
    signal(SIGUSR1, recieveMess); // Register handler for SIGUSR1 signal

    algo = atoi(argv[1]);
    int quantum = atoi(argv[2]);
    printf("Algo no %d, quantum=%d\n", algo, quantum);
    MessageQueueId = msgget(MSG_KEY, IPC_CREAT | 0666);
    if (MessageQueueId == -1)
    {
        perror("Failed to create/get message queue");
        exit(1);
    }

    printf("Message queue ID (sch): %d\n", MessageQueueId);

    // initialise queues to be used in algorithm iterations
    if (algo == HPF)
        PriQ = PCBPriQ_init();
    if (algo == SRTN)
        SRTN_Queue = SRTN_PriQueue_init(100);
    if (algo == RR)
    {
        queue = malloc(sizeof(CircularQueue));
        queue->front = -1;
        queue->rear = -1;
        pcbtempRR = NULL;
    }

    printf("PCB Q init completed\n");

    FILE *fp = fopen("schedulerlog.txt", "w");
    FILE *fp2 = fopen("schedulerPref.txt", "w");

    bool end = 0;
    while (!end || readyQNotEmpty(algo))
    {

        // Check for new processes only when needed
        end = end ? end : checkifEnd(MessageQueueId);
        // printf("End = %d", end);
        checkforNewProcesses(MessageQueueId, algo);

        // Only run algorithm iterations when the clock ticks
        // run the algorithms (one iteration)
        switch (algo)
        {
        case HPF:
            HPF_Iter(fp, fp2);
            break;
        case SRTN:
            SRTN_func(fp, fp2);
            sleep(1);
            break;
        case RR:
            roundRobin(quantum, fp);
            break;
        }
        if (algo == HPF)
        {
            // PCBPriQ_printGivenIDs(PriQ);
        }
    }

    schedulerPrefPrint(fp2); // print final results in pref file
    printf("done");
    fclose(fp);
    fclose(fp2);
    free(PriQ);
    free(queue);
    SRTN_PriQueue_free(SRTN_Queue);
    destroyClk(true);
    exit(0);
}