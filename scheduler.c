#include "headers.h"
#define CirQ_SIZE 100
PCBPriQ* PriQ; //ready queue for HPF

bool readyQNotEmpty(int algo){
    switch (algo){
        case HPF:
            return !(PCBPriQ_isEmpty(PriQ));
            break;
        case SRTN:
            
            break;
        case RR:
            
            break;
    }
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
                    
                    break;
                case RR:

                    break;
            }
        }
    }
}

void HPF_Iter(){
    pcb* head = PCBPriQ_peek(PriQ);
    if (head == NULL) {
        //printf("No process in the queue to run\n");
        return;
    }
    
    if(head->remainingTime == 0){
        printf("Process %d Terminated at %d\n", head->givenid, getClk());
        kill(head->sysstemid, SIGKILL); //temporary for testing purposes
        PCBPriQ_dequeue(PriQ);
    }
    else{
        // Set waiting time when first executing the process
        if(head->waitingTime == -1){
            head->waitingTime = getClk() - head->arrivalTime;
        }
        // Continue the process execution
        kill(head->sysstemid, SIGCONT);
        
        // Only decrement the remaining time once per clock tick
        head->remainingTime--;
        head->status = RUNNING;
        printf("Running Process %d at %d, remaining time: %d\n", head->givenid, getClk(), head->remainingTime);
    }
}

#define maxProcess 1000 // will change and it later and probably remove it from the code 
float WTA[maxProcess]; // waiting time of the process
int WT[maxProcess]; // turnaround time of the process
int execTime[maxProcess]; // execution time of the process
int countGlobal = 0; // number of processes in the system
int indexGlobal = 0; // index of the process in the system
int indexGlobal2 = 0; // index of the process in the system

// void roundRobin(int quantum,int numProc, pcb *process[],FILE *fp) //assuming I am going to get a array of processes this assumption might not be true 
// {
//     initClk();
//     CircularQueue *queue = malloc(sizeof(CircularQueue));
//     queue->front = -1;
//     queue->rear = -1;
//     int alldone = 0;
//     int countDone = 0;
//     pcb *pcbtemp = malloc(numProc * sizeof(pcb));
//     while (1)
//     {
//         for (int i = 0; i < numProc; i++)
//         {
//             if(process[i]!=NULL && process[i]->arrivalTime <= getClk() && process[i]->remainingTime > 0)
//             {
//                 execTime[countGlobal]=process[i]->executionTime;
//                 enqueueCir(queue,process[i]);
//                 process[i]=NULL;
//             }
//         }
//         pcbtemp = dequeueCir(queue);
//         if (pcbtemp != NULL)
//         {
//         int currentTime = getClk(); // Time when it get dequeued (start time)

//         if(pcbtemp->waitingTime == 0 && pcbtemp->process->runTime == pcbtemp->remainingTime) //I think that execution time is the time need for it to finish from the start not sure
//         {
//             pcbtemp->waitingTime = currentTime - pcbtemp->arrivalTime; // Waiting time of the process
//             schedulerlogPrint(fp, currentTime, pcbtemp->process->processID, "started", pcbtemp->arrivalTime, pcbtemp->process->runTime,pcbtemp->remainingTime, pcbtemp->waitingTime, false); 
//             WT[indexGlobal2] = pcbtemp->waitingTime;
//         }
        
//         else

//         {
//             schedulerlogPrint(fp, currentTime, pcbtemp->process->processID, "resumed", pcbtemp->arrivalTime, pcbtemp->process->runTime,pcbtemp->remainingTime, pcbtemp->waitingTime, false);
//         }
        
//             if (pcbtemp->remainingTime > quantum)
//             {
//                 pcbtemp->remainingTime -= quantum;
//                 while(currentTime + quantum > getClk()){}
//                 enqueueCir(queue, pcbtemp);
//                 schedulerlogPrint(fp, currentTime + quantum, pcbtemp->process->processID, "stopped", pcbtemp->arrivalTime, pcbtemp->process->runTime,pcbtemp->remainingTime, pcbtemp->waitingTime, false); 
//             }
//             else
//             {
//                 int finishTime = currentTime + pcbtemp->remainingTime; // End time of the process
//                 pcbtemp->remainingTime = 0;
//                 schedulerlogPrint(fp, currentTime + quantum, pcbtemp->process->processID, "finished", pcbtemp->arrivalTime, pcbtemp->process->runTime,pcbtemp->remainingTime, pcbtemp->waitingTime, true);
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
// void schedulerlogPrint (FILE *fp, int currentTime, int processID, char status [],int arrivalTime,int totalTime ,int remainingTime, int waitingTime ,bool isfinished)
// //AHMED ABD-ELjALeel!!!!!!!! please use this and don't waste your time and copy the arguments from the use of the function in roundRobin so you don't waste time filling the arguments
// {
//     if (fp == NULL)
//     {
//         printf("Error opening file!\n");
//         return;
//     }
//     if (isfinished ==true)
//     {
//         int turnaroundTime = currentTime - arrivalTime;
//         float weightedTA = (float)(turnaroundTime / totalTime);
//         WTA[indexGlobal] = weightedTA;
//         indexGlobal++;
//         fprintf(fp, "At time  %d process %d %s arr %d total %d remain %d wait %d TA %d WTA %d\n", currentTime , processID ,status ,arrivalTime ,totalTime , remainingTime , waitingTime , turnaroundTime ,weightedTA );
//     }
//     else
//     {
//         fprintf(fp, "At time  %d process %d %s arr %d total %d remain %d wait %d \n", currentTime, processID,status, arrivalTime,totalTime, remainingTime, waitingTime);
//     }
//     return;
// }
    
// void schedulerPrefPrint(FILE *fp){ //missing std WTA I am to tired to do it now
//     int currentTime = getClk();
//     int totalWT = 0;
//     float totalWTA = 0;
//     int executionTimetotal = 0;
//     for (int i = 0; i < countGlobal; i++)
//     {
//         totalWT += WT[i];
//         totalWTA += WTA[i];
//         executionTimetotal += execTime[i];
//     }
//     float utilization = (executionTimetotal / (float)currentTime)*100 ;
//     float averageWT = (float)totalWT / countGlobal;
//     float averageWTA = (float)totalWTA / countGlobal;
//     float averageWT = round2(averageWT, 2);
//     float averageWTA = round2(averageWTA, 2);
//     fprintf(fp,"CPU utilization = %d%%\n", utilization);
//     fprintf(fp,"Avg WTA = %.2f\n", averageWTA);
//     fprintf(fp,"Avg waiting = %.2f\n", averageWT);
//     //fprintf(fp,"Std WTA = %.2f\n", stdWTA);

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
    PriQ = PCBPriQ_init(); 
    printf("PCB init completed\n");
    
    
    int lastClk = -1;
    while(!checkifEnd(MessageQueueId) || readyQNotEmpty(algo)) { 
        // Check for new processes only when needed
        if(!checkifEnd(MessageQueueId)) {
            checkforNewProcesses(MessageQueueId, algo);
        }

        // Only run algorithm iterations when the clock ticks
        int currentClk = getClk();
        if (currentClk > lastClk) {
            lastClk = currentClk;
            
            // Run the appropriate scheduling algorithm
            switch (algo) {
                case HPF:
                    HPF_Iter();
                    break;
                case SRTN:
                    // SRTN implementation would go here
                    break;
                case RR:
                    // RR implementation would go here
                    break;
            }
        }
        
    }

    destroyClk(true);
    exit(0);
}
