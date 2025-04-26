#include "headers.h"

int main(int argc, char *argv[])
{
    // initClk();
    // TODO implement the scheduler :)
    // upon termination release the clock resources.
    printf("Scheduler starting\n");
    enum schedulealgo algo = argv[1];
    int quantum = argv[2];
    int MessageQueueId = msgget(MSG_KEY, IPC_CREAT | 0666);
    ProcessMsg msg;


    // destroyClk(true);
    exit(0);
}
