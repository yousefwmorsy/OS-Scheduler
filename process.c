#include "headers.h"

/* Modify this file as needed*/
int remainingtime;
int PID;
void handler(int signum)
{
    if (remainingtime <= 1)
    {
        printf("Processssssssssss %d terminated at time %d\n", PID, getClk());
        exit(0);
    }
    else
    {
        remainingtime--;
        printf("Process %d received signal at time %d, remaining time: %d\n", PID, getClk(), remainingtime);
    } // Set remaining time to 0 when signal is received
    signal(SIGCONT, handler);
}
void stopping(int signum)
{
    // TODO Clears all resources in case of interruption
    pause();
}

int main(int agrc, char *argv[])
{
    initClk();
    remainingtime = atoi(argv[1]); // get the remaining time from the command line arguments
    PID = atoi(argv[2]);           // get the PID from the command line arguments

    printf("Process %d started and recieved %d remaining time \n", PID, remainingtime);

    // TODO it needs to get the remaining time from somewhere
    // remainingtime = ??;
    signal(SIGCONT, handler);  // Register handler for SIGUSR1 signal
    signal(SIGTSTP, stopping); // Register handler for SIGINT signal
    // signal(SIGUSR1, clearResources); // Register handler for SIGINT signal
    while (1)
    {
        // printf("Waiting...\n");
        // pause(); // remainingtime = ??;
    }

    return 0;
}
