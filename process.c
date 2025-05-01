#include "headers.h"

/* Modify this file as needed*/
int remainingtime;
int PID;
int quantum;
void handler(int signum)
{
    if (remainingtime <= quantum)
    {
        printf("Processssssssssss %d terminated at time %d\n", PID, getClk());
        kill(getppid(), SIGUSR2); // Send SIGUSR2 to the Scheduer
        exit(0);
    }
    else
    {
        remainingtime -= quantum;
        printf("Process %d received signal at time %d, remaining time: %d\n", PID, getClk(), remainingtime);
    } // Set remaining time to 0 when signal is received
    signal(SIGCONT, handler);
}
int main(int agrc, char *argv[])
{
    initClk();
    remainingtime = atoi(argv[1]); // get the remaining time from the command line arguments
    PID = atoi(argv[2]);           // get the PID from the command line arguments
    quantum = atoi(argv[3]);       // get the quantum from the command line arguments
    printf("Process %d forked at %d and recieved %d remaining time \n", PID, getClk(), remainingtime);

    // TODO it needs to get the remaining time from somewhere
    // remainingtime = ??;
    signal(SIGCONT, handler); // Register handler for SIGUSR1 signal
    while (1)
    {
        // printf("Waiting...\n");
        kill(getpid(), SIGSTOP); // remainingtime = ??;
    }

    return 0;
}
