#include "headers.h"

/* Modify this file as needed*/
int remainingtime;
int PID;
int quantum;
void handler(int signum)
{
    if (remainingtime <= quantum)
    {
        kill(getppid(), SIGUSR1); // Send SIGUSR2 to the Scheduer
        printf("Process %d terminated at time %d\n", PID, getClk());
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
    printf("Process %d forked at %d and received %d remaining time \n", PID, getClk(), remainingtime);
    // TODO it needs to get the remaining time from somewhere
    signal(SIGCONT, handler); // Register handler for SIGUSR1 signal
    kill(getppid(), SIGCONT); // Send SIGUSR1 to the Scheduer
    while (1)
    {
        kill(getpid(), SIGTSTP); // remainingtime = ??;
    }

    return 0;
}
