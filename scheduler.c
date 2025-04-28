#include "headers.h"
#include <math.h>
#define maxProcess 1000 // will change and it later and probably remove it from the code 
float WTA; // waiting time of the process
int WT; // turnaround time of the process
int execTime; // execution time of the process
int countGlobal = 0; // number of processes in the system

bool readyQNotEmpty(int algo){
    switch (algo){
        case HPF:

            break;
        case SRTN:

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
    
    if (status == "finished")
    {
        int turnaroundTime = currentTime - arrivalTime;
        float weightedTA = (float)turnaroundTime / totalTime;
        WTA+= weightedTA;
        fprintf(fp, "At time  %d process %d %s arr %d total %d remain %d wait %d TA %d WTA %f\n", currentTime , processID ,status ,arrivalTime ,totalTime , remainingTime , waitingTime , turnaroundTime ,weightedTA );
        printf("At time  %d process %d %s arr %d total %d remain %d wait %d TA %d WTA %f\n", currentTime , processID ,status ,arrivalTime ,totalTime , remainingTime , waitingTime , turnaroundTime ,weightedTA );
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
    if(msgrcv(msg_q, &msg, sizeof(ProcessMsg), 2, IPC_NOWAIT)){ //wait for msg of type 2
        return true;
    }
    return false;
}
void checkforNewProcesses(int msg_q, int algo){
    ProcessMsg msg;
    while(msgrcv(msg_q, &msg, sizeof(ProcessMsg), 1, IPC_NOWAIT)){ //wait for msg of type 1 (process)
        int pid = fork();
        if(pid==-1){
            perror("Process fork failed\n");
            return ;
        }
        else if(pid == 0){
            execl("Compiled/process.out","./Compiled/process.out", (char*)NULL);
            perror("Process execl failed\n");
            return;
        }
        else {
            pcb *obj = malloc(sizeof(pcb)); 
            if (obj == NULL) {
                perror("Memory allocation for PCB failed\n");
                return;
            }
            *obj = pcb_init(&msg, pid); // Initialize the PCB

            //add to suitable ds
            switch(algo){
                case HPF:

                    break;
                case SRTN: 
                    
                    break;
                case RR:

                    break;
            }
        }
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
    int MessageQueueId = msgget(MSG_KEY, IPC_CREAT | 0666);


     while(readyQNotEmpty(algo)){ //checks that the readyqueue is not empty (not all processes terminated)
        if(!checkifEnd(MessageQueueId)){ //checks for ending msg from generator indicating no more processes
            checkforNewProcesses(MessageQueueId, algo); //checks for new processes sent by gen and adds them to ready queue
        }

        //run the algorithms (one iteration)
        switch (algo){
            case HPF:

                break;
            case SRTN:

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

    destroyClk(true);
    exit(0);
}
