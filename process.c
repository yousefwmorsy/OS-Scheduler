#include "headers.h"

/* Modify this file as needed*/
int remainingtime;

void handler(int signum)
{
    destroyClk(false);
    exit(0); // Exit the process when receiving SIGUSR1

}
int main(int agrc, char *argv[])
{
    initClk();

    // TODO it needs to get the remaining time from somewhere
    // remainingtime = ??;
    signal(SIGUSR1, handler); // Register handler for SIGUSR1 signal
    while (1)
    {
        // remainingtime = ??;
    }

 

    return 0;
}
