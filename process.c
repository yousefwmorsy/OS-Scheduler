#include "headers.h"

/* Modify this file as needed*/
int remainingtime;
int PID;
void handler(int signum)
{
    if (remainingtime <= 1)
    {
        printf("Processssssssssss %d terminated\n", PID);
        exit(0);
    }
    else
    {
        remainingtime--;
        printf("Process %d received signal, remaining time: %d\n", PID, remainingtime);
    } // Set remaining time to 0 when signal is received
    signal(SIGCONT, handler);
}
void clearResources(int signum)
{
    // TODO Clears all resources in case of interruption

    // signal(SIGUSR1, clearResources);
    // destroyClk(true);
    exit(0);
}
int main(int agrc, char *argv[])
{
    initClk();
    remainingtime = atoi(argv[1]); // get the remaining time from the command line arguments
    PID = atoi(argv[2]);           // get the PID from the command line arguments

    printf("Process %d started and recieved %d remaining time \n", getpid(), remainingtime);

    // TODO it needs to get the remaining time from somewhere
    // remainingtime = ??;
    signal(SIGCONT, handler); // Register handler for SIGUSR1 signal
    // signal(SIGUSR1, clearResources); // Register handler for SIGINT signal
    while (1)
    {
        // printf("Waiting...\n");
        // pause(); // remainingtime = ??;
    }

    return 0;
}
