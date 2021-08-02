#include "structs.h"

int main(int argc, char **argv)
{
    shared_memory *smemory = NULL;
    pid_t CHAN;
    sem_t *receiveP = NULL, *receiveCh = NULL;
    sem_t *sendP = NULL, *sendCh = NULL;
    int i, shmid = -1, isSecond = 0, debug = 1; //turn it to 1 to debug
    unsigned long *checksum = NULL;
    char shmid_str[30];
    char *messageP = NULL, *messageCh = NULL;

    if (argc < 3)
        exit(1);

    for (i = 0; i < argc; i++)
    {
        if (strcmp(argv[i], "-s") == 0) //get shared memory id
            shmid = atoi(argv[i + 1]);
        if (strcmp(argv[i], "-f") == 0) //who sends to whom
            isSecond = atoi(argv[i + 1]);
    }
    smemory = shmat(shmid, NULL, 0); //attach to memory
    if (smemory < 0)
    {
        perror("***Attach Failed***\n");
        exit(1);
    }
    if (!isSecond) //first time/p1 to p2
    {
        sprintf(shmid_str, "%d", shmid);

        if ((CHAN = fork()) < 0)
            perror("***Fork Failed***\n");
        else if (CHAN == 0)
        {
            execl("chan", "chan", "-s", shmid_str, (char *)NULL); //exec , chan.c file
            perror("***Execl Failed***\n");
            exit(1);
        }
        receiveP = &(smemory->mutexP1_ENC1); //1
        sendP = &(smemory->mutexENC1_P1);    //0
        sendCh = &(smemory->mutexENC1_Ch);
        receiveCh = &(smemory->mutexCh_ENC1);
        messageP = (smemory->messagePENC1);
        messageCh = (smemory->messageENCCh1);
        checksum = &(smemory->checksum1);
    }
    else
    {
        receiveP = &(smemory->mutexP2_ENC2);
        sendP = &(smemory->mutexENC2_P2);
        sendCh = &(smemory->mutexENC2_Ch);
        receiveCh = &(smemory->mutexCh_ENC2);
        messageP = (smemory->messagePENC2);
        messageCh = (smemory->messageENCCh2);
        checksum = &(smemory->checksum2);
    }
    if (!isSecond || strcmp(messageCh, "TERM") != 0)
    {
        while (1)
        {
            sem_wait(sendP);
            if (debug)
            {
                printf("ENC%d:%s\n", isSecond + 1, messageP);
            }
            sem_wait(sendCh);
            strcpy(messageCh, messageP); //send message to chan
            *checksum = hash(messageCh); //calculate first checksum
            sem_post(receiveCh);
            sem_post(sendP);
            if (strcmp(messageP, "TERM") == 0)
            {
                break;
            }
            sleep(1);
            sem_post(sendCh);
            sem_wait(receiveCh);
            sem_wait(sendP);
        }
    }

    //Detach memory
    if (shmdt(smemory) == -1)
    {
        perror("***Detach Memory Failed***");
        exit(1);
    }
    exit(0);
}