#include "headers.h"
#define CirQ_SIZE 100
bool checkifEnd(int msg_q){
    ProcessMsg msg;
    if(msgrcv(msg_q, &msg, sizeof(ProcessMsg), 2, IPC_NOWAIT)){ //wait for msg of type 2
        return true;
    }
    return false;
}
void roundRobin(int quantum,int numProc, pcb *process[]) //assuming I am going to get a array of processes this assumption might not be true 
{
    initClk();
    CircularQueue *queue = malloc(sizeof(CircularQueue));
    queue->front = -1;
    queue->rear = -1;
    int alldone = 0;
    int countDone = 0;
    pcb *pcbtemp = malloc(numProc * sizeof(pcb));
    while (!alldone)
    {
        for (int i = 0; i < numProc; i++)
        {
            if(process[i]!=NULL && process[i]->arrivalTime <= getClk() && process[i]->remainingTime > 0)
            {
                enqueueCir(queue,process[i]);
                process[i]=NULL;
            }
        }
        pcbtemp = dequeueCir(queue);
        int startTime = getClk(); // Time when it get dequeued (start time)
        if(pcbtemp->waitingTime == 0 && pcbtemp->process->runTime == pcbtemp->remainingTime) //I think that execution time is the time need for it to finish from the start not sure
        {
            pcbtemp->waitingTime = startTime - pcbtemp->arrivalTime; // Waiting time of the process
        }

        if (pcbtemp != NULL)
        {
            if (pcbtemp->remainingTime > quantum)
            {
                pcbtemp->remainingTime -= quantum;
                while(startTime + quantum > getClk())
                {
                }
                enqueueCir(queue, pcbtemp);
            }
            else
            {
                
                pcbtemp->remainingTime = 0;
                int finishTime = startTime + quantum; // End time of the process
                countDone++;
            }
        }
        if (numProc == countDone)
        {
            alldone = 1;
        }
        
 
    }
    

    
}
int main(int argc, char *argv[])
{
    initClk();
    // TODO implement the scheduler :)
    // upon termination release the clock resources.
    printf("Scheduler starting\n");
    enum schedulealgo algo = atoi(argv[1]);
    int quantum = atoi(argv[2]);
    int MessageQueueId = msgget(MSG_KEY, IPC_CREAT | 0666);
    while(1){ //to be changed to something that indicates that the readyqueue is not empty (not all processes terminated)
        if(!checkifEnd(MessageQueueId)){ //checks for ending msg from generator indicating no more processes
            checkforNewProcesses(MessageQueueId); //checks for new processes sent by gen and adds them to ready queue
        }

        //run the algorithms
        switch (algo){
            case HPF:

                break;
            case SRTN:

                break;
            case RR:

                break;
        }
    }

    destroyClk(true);
    exit(0);
}
