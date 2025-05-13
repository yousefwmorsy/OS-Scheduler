#include "headers.h"
#include <math.h>
#define maxMemory 1024
#define maxProcess 1000 // will change and it later and probably remove it from the code
float WTA = 0;          // waiting time of the process
int WT = 0;             // turnaround time of the process
int TotalTime = 0;      // execution time of the process
int countGlobal = 0;    // number of processes in the system
int countfinished = 0;
int newentry = 0;
bool workingOnHandler = false; // flag to check if the signal handler is being executed
bool allsent = false;
PCBPriQ *PriQ;             // ready queue for HPF
SRTN_PriQueue *SRTN_Queue; // Priority Queue for SRTN
CircularQueue *queue;      // queue for RR
pcb *pcbtempRR;            // process that is running now
float *WTAs;               // Array to store WTA values
FILE *fp;                  // file pointer for logging
enum schedulealgo algo;    // algorithm type
int quantum;               // quantum for RR
int MessageQueueId;
Memory_Block *memory;
Blocked_Processes *BP;
FILE *fp3;
bool outsideOfQ = false ;
bool someone_finished = false;

bool readyQNotEmpty(int algo)
{
    switch (algo)
    {
    case HPF:
        return (PCBPriQ_isEmpty(PriQ) == 0);
        break;
    case SRTN:
        return (SRTN_Queue->size != 0);
        break;
    case RR:
       if (isEmptyCir(queue))
       {
        if (outsideOfQ)
        {
           return true;
        }
        return false ;

       }
       return true ;
        break;
    }
}

void schedulerlogPrint(FILE *fip, int currentTime, pcb *pcb, char status[])
{
    if (fip == NULL)
    {
        printf("Error opening file  %s!!!!\n", status);
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
        countfinished++;
        int turnaroundTime = currentTime - arrivalTime;
        float weightedTA = (float)turnaroundTime / totalTime;
        WTA += weightedTA;
        WTAs[countGlobal] = weightedTA; // Store WTA value in the array
        // printf("Count: %d\n", countGlobal);
        fprintf(fip, "At time  %d process %d %s arr %d total %d remain %d wait %d TA %d WTA %.2f\n", currentTime, processID, status, arrivalTime, totalTime, remainingTime, waitingTime, turnaroundTime, weightedTA);
        printf("At time  %d process %d %s arr %d total %d remain %d wait %d TA %d WTA %.2f\n", currentTime, processID, status, arrivalTime, totalTime, remainingTime, waitingTime, turnaroundTime, weightedTA);
    }
    else
    {
        fprintf(fip, "At time  %d process %d %s arr %d total %d remain %d wait %d \n", currentTime, processID, status, arrivalTime, totalTime, remainingTime, waitingTime);
        printf("At time  %d process %d %s arr %d total %d remain %d wait %d \n", currentTime, processID, status, arrivalTime, totalTime, remainingTime, waitingTime);
    }
}

void printMemLog(FILE *fp,int currentTime ,pcb *pcb,char status []){
fprintf(fp,"At time %d %s %d bytes for process %d from %d to %d\n",currentTime ,status ,pcb->memsize,pcb->givenid,pcb->memStart,pcb->memEnd);
}

void schedulerPrefPrint(FILE *fp)
{ // missing std WTA I am to tired to do it now
    int currentTime = getClk();
    float utilization = (TotalTime / (float)currentTime) * 100;
    float averageWT = (float)WT / countGlobal;
    float averageWTA = (float)WTA / countGlobal;

    float stdWTA = 0;
    for (int i = 0; i < countGlobal; i++)
    {
        // printf("%.2f - %.2f\n", averageWTA, WTAs[i]);
        stdWTA += pow((averageWTA) - (WTAs[i]), 2);
    }
    stdWTA = sqrt(stdWTA / countGlobal);
    averageWT = roundf(averageWT * 100) / 100;
    averageWTA = roundf(averageWTA * 100) / 100;
    utilization = roundf(utilization * 100) / 100;
    printf("Total Time = %d, Running Time = %d\n", currentTime, TotalTime);
    fprintf(fp, "CPU utilization = %.2f%%\n", utilization);
    fprintf(fp, "Avg WTA = %.2f\n", averageWTA);
    fprintf(fp, "Avg Waiting = %.2f\n", averageWT);

    stdWTA = roundf(stdWTA * 100) / 100;
    fprintf(fp, "Std WTA = %.2f\n", stdWTA);
}

bool checkifEnd(int msg_q)
{
    ProcessMsg msg;
    if (msgrcv(msg_q, &msg, sizeof(ProcessMsg) - sizeof(long), 2, IPC_NOWAIT) != -1)
    { // wait for msg of type 2
        return true;
    }
    return false;
}

void checkforNewProcesses(int msg_q, int algo)
{
    workingOnHandler = true;
    ProcessMsg msg;
    bool entered = false;
    while (msgrcv(msg_q, &msg, sizeof(ProcessMsg) - sizeof(long), 1, IPC_NOWAIT) != -1)
    { // wait for msg of type 1 (process)
        entered = true;
        printf("Received process message: ID=%d, Arrival=%d, Runtime=%d, Priority=%d\n", msg.id, msg.arrivalTime, msg.runTime, msg.priority);
        countGlobal++;
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
            char Quantum[5];
            sprintf(Quantum, "%d", quantum);

            execl("./Compiled/process.out", "process.out", RunTime, ID, Quantum, (char *)NULL);
            perror("Process execl failed\n");
            exit(1);
        }
        else
        {
            kill(getpid(), SIGSTOP); // stop the process immediately after forking
            printf("Forked process with PID=%d\n", pid);
            pcb *obj = malloc(sizeof(pcb));
            if (obj == NULL)
            {
                perror("Memory allocation for PCB failed\n");
                return;
            }
            pcb tempPcb = pcb_init(&msg, pid);

            memcpy(obj, &tempPcb, sizeof(pcb));
            if(!allocate_memory(obj, memory, 'H')){
                blockedQueue_enqueue(BP, obj);
                printf("Process (%d, %d) get in blocked memory\n", tempPcb.givenid, tempPcb.systemid);
            }
            else{
                printf("Process (%d, %d) get in memory\n", tempPcb.givenid, tempPcb.systemid);
                printMemLog(fp3,getClk(),obj,"allocated");
                // add to suitable ds
                switch (algo)
                {
                case HPF:
                    PCBPriQ_enqueue(PriQ, obj);
                    printf("Enqueued process ID=%d into priority queue\n", obj->givenid);
                    break;
                case SRTN:
                    SRTN_PriQueue_insert(SRTN_Queue, obj);
                    printf("Enqueued process ID=%d into SRTN queue\n", obj->givenid);
                    break;
                case RR:
                    printf("Enqueued process ID=%d into RR queue\n", obj->givenid);
                    RR_insert(queue, obj); // pcbtempRR is the process that is running now
                    break;
                }
            }
        }
    }
    workingOnHandler = false;
}

void SRTN_func(FILE *fp, FILE *fp2)
{
    int currentTime = getClk();

    // 1.Check if any process enqueued in the Queue or not
    if (SRTN_Queue->size == 0)
    {
        return;
    }

    // 2. Peek the SRTN Process in the Priority Queue
    pcb *current_process = SRTN_PriQueue_peek(SRTN_Queue);
    if (current_process == NULL || current_process->remainingTime <= 0)
    {
        return; // Skip invalid processes
    }

    // 3. Start The Process if its first time to start
    if (current_process->remainingTime == current_process->executionTime)
        schedulerlogPrint(fp, getClk(), current_process, "started");

    // 4. Increment the waiting time of the other Processes
    for (int i = 0; i < SRTN_Queue->size; i++)
    {
        if (SRTN_Queue->Process[i] != current_process)
            SRTN_Queue->Process[i]->waitingTime++; // Here it will increment the waiting time of any process which now doesn't take the process.
    }

    while (currentTime + 1 > getClk())
    {
    }

    // 5. Decrement the remaining time of the peeked Process
    if (current_process && current_process->remainingTime > 0 && kill(current_process->systemid, SIGCONT) != -1)
        current_process->remainingTime--;

    // 7. Print the curent states of the current moment of the process
    if (current_process)
        printf("SRTN: Running Process %d at %d, remsaining time: %d\n", current_process->givenid, getClk(), current_process->remainingTime);
}

void HPF_Iter(FILE *fp, FILE *fp2)
{
    pcb *head = PCBPriQ_peek(PriQ);
    int currentTime = getClk();
    if (head == NULL)
    {
        return;
    }

    if (head->remainingTime != 0){
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
        kill(head->systemid, SIGCONT);
        head->remainingTime--;
    }
}

void roundRobin(int quantum, FILE *fp)
{
    if (pcbtempRR != NULL && outsideOfQ == true )
    {
        RR_insert(queue,pcbtempRR);
        outsideOfQ = false;
    }
    pcbtempRR = NULL;

    pcbtempRR = peekCir(queue);

    if (pcbtempRR != NULL)
    {
        int currentTime = getClk(); // Time when it get dequeued (start time)

        if (pcbtempRR->waitingTime == 0 && pcbtempRR->executionTime == pcbtempRR->remainingTime) // I think that execution time is the time need for it to finish from the start not sure
        {
            pcbtempRR->waitingTime = currentTime - pcbtempRR->arrivalTime; // Waiting time of the process

            schedulerlogPrint(fp, currentTime, pcbtempRR, "started");
        }

        else if (pcbtempRR && pcbtempRR->remainingTime > 0)

        {
            schedulerlogPrint(fp, currentTime, pcbtempRR, "resumed");
        }

        if (pcbtempRR && pcbtempRR->remainingTime > quantum)
        {
            pcbtempRR->remainingTime -= quantum;
            while (currentTime + quantum > getClk())
            {
            }
            pcbtempRR = dequeueCir(queue);
            kill(pcbtempRR->systemid, SIGCONT);
            schedulerlogPrint(fp, currentTime + quantum, pcbtempRR, "stopped");
            outsideOfQ = true;
        }
        else if (pcbtempRR)
        {
            int finishTime = currentTime + pcbtempRR->remainingTime; // End time of the process
            while (currentTime + pcbtempRR->remainingTime > getClk())
            {
            }
            pcbtempRR->remainingTime = 0;
            kill(pcbtempRR->systemid, SIGCONT); // run the process
        }
    }
}

bool findProcessByPid(pid_t pid)
{
    printf("Finding process with PID %d\n", pid);
    free_memory(memory, pid); // Free memory
    someone_finished = true;
    pcb *freed_process;
    printf("------------------------------ \n");
    switch (algo)
    {
    case HPF:
        pcb current  = PCBPriQ_dequeue(PriQ);
        printMemLog(fp3,getClk(),&current,"freed");
        schedulerlogPrint(fp, getClk(), &current, "finished");
        break;
    case SRTN:
        freed_process = SRTN_PriQueue_pop(SRTN_Queue);
        schedulerlogPrint(fp, getClk(), freed_process, "finished");
        printMemLog(fp3,getClk(),freed_process,"freed");
        free(freed_process);
        break;
    case RR:
        freed_process = dequeueCir(queue);
        schedulerlogPrint(fp, getClk(), freed_process, "finished");
        printMemLog(fp3,getClk(),freed_process,"freed");
        free(freed_process);
        break;
    }
}

void checkforBlockPro(int algo) {
    someone_finished = false;
    // Process all blocked processes until memory full
    printMemoryTree(memory);
    pcb * blockedProcess = NULL;
    while (blockedQueue_peek(BP) && allocate_memory( blockedProcess = blockedQueue_peek(BP), memory, 'H') )
    {
        blockedQueue_dequeue(BP);
        printf("Blocked process entered the ready queue (ID: %d, SystemID: %d)\n", blockedProcess->givenid, blockedProcess->systemid);
        printMemLog(fp3, getClk(), blockedProcess, "allocated");
        printMemoryTree(memory);
        switch (algo)
        {
            case HPF:
                PCBPriQ_enqueue(PriQ, blockedProcess);
                break;
            case SRTN:
                SRTN_PriQueue_insert(SRTN_Queue, blockedProcess);
                break;
            case RR:
                enqueueCir(queue, blockedProcess);
                break;
        }
    }
}

void recieveMess(int signum)
{
    printf("Received signal to check for new processes\n");
    newentry++;
    return;
}

void signalHandler(int signum)
{
    printf("Received signal that a child terminated \n");
    int status;
    int terminatedPid = 0;
    do
    {
        terminatedPid = waitpid(-1, &status, WNOHANG);
    }while (terminatedPid == 0);

    if (terminatedPid > 0)
    {
        printf("Child process with PID %d has terminated\n", terminatedPid);

        // Handle the termination of the process
        // For example, remove it from the appropriate queue or update its status
        printf("\nbefore findProByID\n\n");
        if (findProcessByPid(terminatedPid))
        {
            printf("Process with PID %d removed from the queue\n", terminatedPid);
        }
        else
        {
            printf("Process with PID %d not found in the queue\n", terminatedPid);
        } // Implement this function to locate the process in your data structures
        printf("\nafter findProByID\n\n");

    }
    else if (terminatedPid == 0)
    {
        printf("No child process has terminated yet\n");
    }
    else
    {
        perror("Error in waitpid");
    }
}

int main(int argc, char *argv[])
{

    printf("Scheduler starting\n");
    WTAs = malloc(sizeof(float) * maxProcess);
    initClk();
    signal(SIGUSR2, recieveMess);   // Register handler for SIGUSR1 signal
    signal(SIGUSR1, signalHandler); // Register handler for SIGCHLD signal

    algo = atoi(argv[1]);
    quantum = atoi(argv[2]);
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

    memory = memory_init(0, maxMemory);
    if(!memory){
        perror("Problem in initializing memory");
        exit(1);
    }

    BP = initialize_blockedQueue();
    if(!BP){
        perror("Problem in initializing BP");
        exit(1);
    }

    fp = fopen("schedulerlog.txt", "w");
    fprintf(fp, "#At time  x process y state arr w total z remain y wait k\n");
    FILE *fp2 = fopen("schedulerPref.txt", "w");
    fp3 = fopen("memory.log","w");
    fprintf(fp3,"#At time x allocated y bytes for process z from i to j\n");

    bool end = 0;
    while (!end || readyQNotEmpty(algo) || !blockedQueue_isEmpty(BP))
    {

        // Check for new processes only when needed
        int lastClk = getClk();
        end = end ? end : checkifEnd(MessageQueueId);
        while (newentry > 0)
        {
            checkforNewProcesses(MessageQueueId, algo);
            newentry--;
        }
        if(someone_finished)
        checkforBlockPro(algo);

        // Only run algorithm iterations when the clock ticks
        // run the algorithms (one iteration)
        usleep(2000);

        switch (algo)
        {
        case HPF:
            PCBPriQ_printGivenIDs(PriQ);
            HPF_Iter(fp, fp2);
            break;
        case SRTN:
            SRTN_func(fp, fp2);
            usleep(1000); // Sleep for a short duration to avoid busy waiting /*Ambiguous*/
            break;
        case RR:
            roundRobin(quantum, fp);
            break;
        }
    }
    readyQNotEmpty(algo);

    schedulerPrefPrint(fp2); // print final results in pref file
    printf("\ndone\n\n");
    fclose(fp);
    fclose(fp2);
    fclose(fp3);
    free(PriQ);
    free(queue);
    SRTN_PriQueue_free(SRTN_Queue);
    free(memory);
    blockedQueue_clear(BP);
    destroyClk(true);
    exit(0);
}