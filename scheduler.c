#include "headers.h"
#include <math.h>
#define maxProcess 1000 // will change and it later and probably remove it from the code 
float WTA; // waiting time of the process
int WT; // turnaround time of the process
int execTime; // execution time of the process
int countGlobal = 0; // number of processes in the system
int IdleTime = 0;
PCBPriQ* PriQ; //ready queue for HPF
SRTN_PriQueue* SRTN_Queue;

bool readyQNotEmpty(int algo){
    switch (algo){
        case HPF:
            //printf("Q Empty %d", PCBPriQ_isEmpty(PriQ));
            return (PCBPriQ_isEmpty(PriQ) == 0);
            break;
        case SRTN:
            return (SRTN_Queue->size == 0);
            break;
        case RR:
            
            break;
    }
}

void schedulerlogPrint (FILE *fp, int currentTime,pcb *pcb, char status [])
//AHMED ABD-ELjALeel!!!!!!!! please use this and don't waste your time and copy the arguments from the use of the function in roundRobin so you don't waste time filling the arguments
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
    
    if (strcmp(status, "finished") == 0)
    {
        int turnaroundTime = currentTime - arrivalTime;
        float weightedTA = (float)turnaroundTime / totalTime;
        WTA+= weightedTA;
        fprintf(fp, "At time  %d process %d %s arr %d total %d remain %d wait %d TA %d WTA %.2f\n", currentTime , processID ,status ,arrivalTime ,totalTime , remainingTime , waitingTime , turnaroundTime ,weightedTA );
        printf("At time  %d process %d %s arr %d total %d remain %d wait %d TA %d WTA %.2f\n", currentTime , processID ,status ,arrivalTime ,totalTime , remainingTime , waitingTime , turnaroundTime ,weightedTA );
    }
    else
    {
        fprintf(fp, "At time  %d process %d %s arr %d total %d remain %d wait %d \n", currentTime, processID,status, arrivalTime,totalTime, remainingTime, waitingTime);
        printf( "At time  %d process %d %s arr %d total %d remain %d wait %d \n", currentTime, processID,status, arrivalTime,totalTime, remainingTime, waitingTime);
    }
}

void schedulerPrefPrint(FILE *fp){ //missing std WTA I am to tired to do it now
    int currentTime = getClk();
    float utilization = (execTime / (float)currentTime)*100 ;
    float averageWT = (float)WT / countGlobal;
    float averageWTA = (float)WTA / countGlobal;
    averageWT = roundf(averageWT * 100) / 100;
    averageWTA = roundf(averageWTA* 100)/100;
    fprintf(fp,"CPU utilization = %.0f%%\n", utilization);
    fprintf(fp,"Avg WTA = %.2f\n", averageWTA);
    fprintf(fp,"Avg waiting = %.2f\n", averageWT);
    //fprintf(fp,"Std WTA = %.2f\n", stdWTA);
}

bool checkifEnd(int msg_q){
    ProcessMsg msg;
    //printf("Checked if end\n");
    if(msgrcv(msg_q, &msg, sizeof(ProcessMsg) - sizeof(long), 2, IPC_NOWAIT)!=-1){ //wait for msg of type 2
        //printf("Returned true \n");
        return true;
    }
    //printf("returned false \n");
    return false;
}

void checkforNewProcesses(int msg_q, int algo){
    ProcessMsg msg;
    //printf("Checking for new processes...\n");
    while(msgrcv(msg_q, &msg, sizeof(ProcessMsg) - sizeof(long), 1, IPC_NOWAIT)!=-1){ //wait for msg of type 1 (process)
        printf("Received process message: ID=%d, Arrival=%d, Runtime=%d, Priority=%d\n", msg.id, msg.arrivalTime, msg.runTime, msg.priority);
        int pid = fork();
        if(pid==-1){
            perror("Process fork failed\n");
            return ;
        }
        else if(pid == 0){
            // Use absolute path for process.out to avoid path issues
            execl("./Compiled/process.out","process.out", (char*)NULL);
            perror("Process execl failed\n");
            exit(1);
        }
        else {
            printf("Forked process with PID=%d\n", pid);
            kill(pid, SIGSTOP);
            pcb *obj = malloc(sizeof(pcb)); 
            if (obj == NULL) {
                perror("Memory allocation for PCB failed\n");
                return;
            }
            pcb tempPcb = pcb_init(&msg, pid);
            memcpy(obj, &tempPcb, sizeof(pcb)); 

            //add to suitable ds
            switch(algo){
                case HPF:
                    PCBPriQ_enqueue(PriQ, obj);
                    printf("Enqueued process ID=%d into priority queue\n", obj->givenid);
                    break;
                case SRTN:
                    printf("SRTN: Entered!!");
                    SRTN_PriQueue_insert(SRTN_Queue, obj);
                    printf("Enqueued process ID=%d into SRTN queue\n", obj->givenid);
                    break;
                case RR:

                    break;
            }
        }
    }
}

void SRTN_func (FILE *fp, FILE *fp2){
    //Check if any process enqueued in the Queue or not
    printf("Iam in SRTN");
    if(SRTN_Queue->size == 0){
        printf("SRTN: No process in the Queue yet...");
        IdleTime++;
        return;
    }

    pcb *current_process = SRTN_PriQueue_peek(SRTN_Queue);
    if(current_process->waitingTime == -1){
        current_process->waitingTime = 0;
        schedulerlogPrint(fp, getClk(), current_process, "started");
    }

    for (int i = 1; i < SRTN_Queue->size; i++){
        if(current_process->waitingTime == -1) current_process->waitingTime = 0;
        else SRTN_Queue->Process[i]->waitingTime++;
    }

    current_process->remainingTime--;
    if (current_process->remainingTime == 0){
        printf("SRTN: Process %d Terminated at %d\n", current_process->givenid, getClk());
        schedulerlogPrint(fp, getClk(), current_process, "finished"); 

        int turn_around_time = getClk() - current_process->arrivalTime;
        WT += current_process->waitingTime;
        WTA += (float)turn_around_time / current_process->executionTime;

        kill(current_process->systemid, SIGKILL); //temporary for testing purposes
        SRTN_PriQueue_pop(SRTN_Queue);
    }

    printf("SRTN: Running Process %d at %d, remaining time: %d\n", current_process->givenid, getClk(), current_process->remainingTime);
}

void HPF_Iter(FILE *fp, FILE *fp2){
    pcb* head = PCBPriQ_peek(PriQ);
        if (head == NULL) {
        printf("No process in the queue to run\n");
        IdleTime++;
        return;
    }
    
    if(head->remainingTime == 0){
        printf("Process %d Terminated at %d\n", head->givenid, getClk());
        schedulerlogPrint(fp, getClk(), head, "finished"); 
        kill(head->systemid, SIGKILL); //temporary for testing purposes
        PCBPriQ_dequeue(PriQ);
    }
    else{
        // Set waiting time when first executing the process
        if(head->waitingTime == -1){
            head->waitingTime = getClk() - head->arrivalTime;
        }
        if(head->status == WAITING){
            // Continue the process execution
            kill(head->systemid, SIGCONT);
            head->status = RUNNING;
            schedulerlogPrint(fp, getClk(), head, "started"); 
        }
        //int curr = getClk();
        //printf("ok");
        //while(curr == getClk()){}
        //printf("whoo");
        // Only decrement the remaining time once per clock tick
        head->remainingTime--;
        printf("Running Process %d at %d, remaining time: %d\n", head->givenid, getClk(), head->remainingTime);
    }

}

// void roundRobin(int quantum,int numProc, pcb *process[],FILE *fp) //assuming I am going to get a array of processes this assumption might not be true 
// {
//     CircularQueue *queue = malloc(sizeof(CircularQueue));
//     queue->front = -1;
//     queue->rear = -1;
//     int alldone = 0;
//     int countDone = 0;
//     pcb *pcbtemp = malloc(numProc * sizeof(pcb));
//     printf("Round Robin started\n");
//     while (1)
//     {
//         for (int i = 0; i < numProc; i++)
//         {
//             if(process[i]!=NULL && process[i]->arrivalTime <= getClk() && process[i]->remainingTime > 0)
//             {
//                 execTime+=process[i]->executionTime;
//                 enqueueCir(queue,process[i]);
//                 countGlobal++;
//                 process[i]=NULL;
//                 printf("Process %d added to the queue\n", i);
//             }
//         }
//         pcbtemp = dequeueCir(queue);
//         if (pcbtemp != NULL)
//         {
//         int currentTime = getClk(); // Time when it get dequeued (start time)

//         if(pcbtemp->waitingTime == 0 && pcbtemp->executionTime == pcbtemp->remainingTime) //I think that execution time is the time need for it to finish from the start not sure
//         {
//             pcbtemp->waitingTime = currentTime - pcbtemp->arrivalTime; // Waiting time of the process
//             schedulerlogPrint(fp, currentTime, pcbtemp, "started"); 
//             WT+= pcbtemp->waitingTime;
//         }
        
//         else

//         {
//             schedulerlogPrint(fp, currentTime, pcbtemp, "resumed");
//         }
        
//             if (pcbtemp->remainingTime > quantum)
//             {
//                 pcbtemp->remainingTime -= quantum;
//                 while(currentTime + quantum > getClk()){}
//                 enqueueCir(queue, pcbtemp);
//                 schedulerlogPrint(fp, currentTime + quantum, pcbtemp, "stopped"); 
//             }
//             else
//             {
//                 int finishTime = currentTime + pcbtemp->remainingTime; // End time of the process
//                 while(currentTime + pcbtemp->remainingTime  > getClk()){}
//                 schedulerlogPrint(fp,currentTime + pcbtemp->remainingTime , pcbtemp,"finished");
//                 pcbtemp->remainingTime = 0;
//                 countDone++;
//             }

//             if (numProc == countDone)
//             {
//                 break;
//             }
//         }
//     }
//     free(pcbtemp);
//     free(queue);

// }
    


int main(int argc, char *argv[])
{
    initClk();  
    // TODO implement the scheduler :)
    // upon termination release the clock resources.
    printf("Scheduler starting\n");
    
    enum schedulealgo algo = atoi(argv[1]);
    int quantum = atoi(argv[2]);
    printf("Algo no %d, quantum=%d\n", algo, quantum);
    int MessageQueueId = msgget(MSG_KEY, IPC_CREAT | 0666);
    if (MessageQueueId == -1) {
        perror("Failed to create/get message queue");
        exit(1);
    }

    printf("Message queue ID (sch): %d\n", MessageQueueId);

    //initialise queues to be used in algorithm iterations
    if(algo == HPF) PriQ = PCBPriQ_init(); 
    if(algo == SRTN) SRTN_Queue = SRTN_PriQueue_init(100);
    printf("PCB init completed\n");

    FILE *fp = fopen("schedulerlog.txt", "w");
    FILE *fp2 = fopen("schedulerPref.txt", "w");

    bool end = 0;
    while(!end || readyQNotEmpty(algo)) { 
        // Check for new processes only when needed
        end = end ? end : checkifEnd(MessageQueueId);
        //printf("End = %d", end);
        checkforNewProcesses(MessageQueueId, algo);

        // Only run algorithm iterations when the clock ticks
        //run the algorithms (one iteration)
        switch (algo){
            case HPF:
                HPF_Iter(fp, fp2);
                break;
            case SRTN:
                printf("\nSRTN: \n");
                SRTN_func(fp, fp2);
                break;
            case RR:
                printf("Round Robin\n");
                FILE *fp = fopen("schedulerlog.txt", "w");
                //roundRobin(quantum, 6, process, fp); //call the round robin function with the processes and the quantum time
                CircularQueue *queue = malloc(sizeof(CircularQueue));
                queue->front = -1;
                queue->rear = -1;
                int alldone = 0;
                int countDone = 0;
                pcb *pcbtemp = malloc(sizeof(pcb));
                //todo list
                //should take the processes with arrival time equals to the current time here
                //should add to countglobal here too
                //should add to circular queue here too
                //should add to the execution time gloabl variable here too
                pcbtemp = dequeueCir(queue);
                if (pcbtemp != NULL)
                {
                int currentTime = getClk(); // Time when it get dequeued (start time)

                if(pcbtemp->waitingTime == 0 && pcbtemp->executionTime == pcbtemp->remainingTime) //I think that execution time is the time need for it to finish from the start not sure
                {
                    pcbtemp->waitingTime = currentTime - pcbtemp->arrivalTime; // Waiting time of the process
                    schedulerlogPrint(fp, currentTime, pcbtemp, "started"); 
                    WT+= pcbtemp->waitingTime;
                }
                
                else

                {
                    schedulerlogPrint(fp, currentTime, pcbtemp, "resumed");
                }
                
                    if (pcbtemp->remainingTime > quantum)
                    {
                        pcbtemp->remainingTime -= quantum;
                        while(currentTime + quantum > getClk()){}
                        enqueueCir(queue, pcbtemp);
                        schedulerlogPrint(fp, currentTime + quantum, pcbtemp, "stopped"); 
                    }
                    else
                    {
                        int finishTime = currentTime + pcbtemp->remainingTime; // End time of the process
                        while(currentTime + pcbtemp->remainingTime  > getClk()){}
                        schedulerlogPrint(fp,currentTime + pcbtemp->remainingTime , pcbtemp,"finished");
                        pcbtemp->remainingTime = 0;
                        countDone++;
                    }

                FILE *fp2 = fopen("schedulerPref.txt", "w");
                schedulerPrefPrint(fp2); //print final results in pref file
                free(pcbtemp);
                free(queue);
                fclose(fp);
                fclose(fp2);
                    break;
            }
        }
        PCBPriQ_printGivenIDs(PriQ);
        sleep(1);
    }
    schedulerPrefPrint(fp2); //print final results in pref file
    printf("done");
    fclose(fp);
    fclose(fp2);
    free(PriQ);
    SRTN_PriQueue_free(SRTN_Queue);
    destroyClk(true);
    exit(0);
}