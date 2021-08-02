#include "structs.h"

int main(int argc, char **argv)
{
    shared_memory *smemory = NULL;
    sem_t *send1 = NULL, *receive1 = NULL;
    sem_t *send2 = NULL, *receive2 = NULL;
    sem_t *mutexCheck = NULL;
    srand(time(0));
    int i, shmid, debug = 1;
    char shmid_str[30];
    char *message1 = NULL;
    char *message2 = NULL;
    unsigned long *checksum1 = NULL;
    unsigned long *checksum2 = NULL;

    //get shared memory id
    for (i = 0; i < argc; i++)
    {
        if (strcmp(argv[i], "-s") == 0)
            shmid = atoi(argv[i + 1]);
    }
    //attach to memory
    smemory = shmat(shmid, NULL, 0);
    if (smemory < 0)
    {
        perror("***Attach Shared Memory Failed***\n");
        exit(1);
    }

    sprintf(shmid_str, "%d", shmid);

    message1 = (smemory->messageENCCh1); //to enc1
    message2 = (smemory->messageENCCh2); //to enc2
    send1 = &(smemory->mutexENC1_Ch);
    receive1 = &(smemory->mutexCh_ENC1);
    send2 = &(smemory->mutexENC2_Ch);
    receive2 = &(smemory->mutexCh_ENC2);
    checksum1 = &(smemory->checksum1);
    checksum2 = &(smemory->checksum2);
    mutexCheck = &(smemory->mutexCheck);
    while (1)
    {
        // chan receives message from enc1 and sends it to enc2
        sem_wait(receive1); //wait enc1 to write
        if (debug)
        {
            printf("CHAN: %s\n", message1);
        }
        sem_wait(send2);
        smemory->isSame = 0;
        do
        {
            double x = (double)rand() / (RAND_MAX);
            if (x < smemory->probability)
            {
                //printf("%f\n",x);
                strcpy(message2, message1); //send message to enc2
                message2[0] = '*';          //change message
                // printf("Message changed\n");
            }
            else
            {
                strcpy(message2, message1); //same
            }
            *checksum2 = *checksum1; //send checksum to enc2
            sem_post(receive2);
            sem_wait(mutexCheck);
        } while (smemory->isSame == 0); //loop until same
        if (strcmp(message1, "TERM") == 0)
        {
            break;
        }
        sleep(1);
        // chan receives message from enc2 and sends it to enc1

        sem_post(send2);
        sem_wait(receive2);
        if (debug)
        {
            printf("CHAN: %s\n", message2);
        }
        sem_wait(send1);

        smemory->isSame = 0;
        do
        {
            double x = (double)rand() / (RAND_MAX);
            if (x < smemory->probability)
            {
                //printf("%f\n",x);
                strcpy(message1, message2); //send message to enc1
                message1[0] = '*';
                // printf("Message changed\n");
            }
            else
            {
                strcpy(message1, message2);
            }
            *checksum1 = *checksum2; //send checksum to enc1
            sem_post(receive1);
            sem_wait(mutexCheck);
        } while (smemory->isSame == 0);

        if (strcmp(message2, "TERM") == 0)
        {
            break;
        }
        sleep(1);
        sem_post(send1);
    }

    //Detach memory
    if (shmdt(smemory) == -1)
    {
        perror("***Detach Memory Failed***");
        exit(1);
    }
    exit(0);
}