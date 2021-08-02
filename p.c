#include "structs.h"

int main(int argc, char **argv)
{
    sem_t *send = NULL, *receive = NULL;
    pid_t ENC;
    int isSecond = 1, shmid = -1, status;
    float prob = 0;
    char *message = NULL;
    FILE *fp = NULL;
    shared_memory *smemory = NULL;
    char shmid_str[30];

    for (int i = 0; i < argc; i++)
    {
        if (strcmp(argv[i], "-p") == 0)
        {
            prob = atof(argv[i + 1]); //probability of mistake
            isSecond = 0;
        }
    }

    if (!isSecond) //first message / shared memory is not yet made
    {
        shmid = shmget(IPC_PRIVATE, sizeof(shared_memory), IPC_CREAT | 0666); //allocate shared memory segment
        printf("Shared memory id = %d\n", shmid);
        if (shmid < 0)
        {
            perror("***Shared Memory Failed***\n");
            exit(1);
        }
        fp = fopen("shm_id", "w+"); //save shmid in file shm_id.txt so that p2 reads it
        if (fp == NULL)
        {
            exit(1);
        }
        fprintf(fp, "%d", shmid);
        fclose(fp);
    }
    else
    {
        fp = fopen("shm_id", "r"); //p2 reads from file shm_id.txt the shmid
        if (fp == NULL)
        {
            exit(1);
        }
        fscanf(fp, "%d", &shmid);
        printf("Shared memory id = %d\n", shmid);
        fclose(fp);
    }

    smemory = shmat(shmid, NULL, 0); //attach to memory
    if (smemory < 0)
    {
        perror("***Attach Shared Memory Failed***\n");
        exit(1);
    }
    if (!isSecond)
    {
        //initialize posix semaphores
        if (sem_init(&smemory->mutexP1_ENC1, 1, 1) != 0)
        {
            perror("***Init Mutex Failed***\n");
            exit(1);
        }
        if (sem_init(&smemory->mutexENC1_Ch, 1, 1) != 0)
        {
            perror("***Init Mutex Failed***\n");
            exit(1);
        }
        if (sem_init(&smemory->mutexCh_ENC2, 1, 0) != 0) // blocked because i have stuff to do
        {
            perror("***Init Mutex Failed***\n");
            exit(1);
        }
        if (sem_init(&smemory->mutexENC2_P2, 1, 1) != 0)
        {
            perror("***Init Mutex Failed***\n");
            exit(1);
        }

        if (sem_init(&smemory->mutexP2_ENC2, 1, 0) != 0)
        {
            perror("***Init Mutex Failed***\n");
            exit(1);
        }
        if (sem_init(&smemory->mutexENC2_Ch, 1, 1) != 0)
        {
            perror("***Init Mutex Failed***\n");
            exit(1);
        }
        if (sem_init(&smemory->mutexCh_ENC1, 1, 0) != 0)
        {
            perror("***Init Mutex Failed***\n");
            exit(1);
        }
        if (sem_init(&smemory->mutexENC1_P1, 1, 0) != 0)
        {
            perror("***Init Mutex Failed***\n");
            exit(1);
        }
        if (sem_init(&smemory->mutexCheck, 1, 0) != 0)
        {
            perror("***Init Mutex Failed***\n");
            exit(1);
        }
    }

    if (!isSecond) //p1 sends first and then receives
    {
        send = &(smemory->mutexP1_ENC1);
        receive = &(smemory->mutexENC1_P1);
        message = (smemory->messagePENC1);
    }
    else //p2 receives first and then sends
    {
        send = &(smemory->mutexP2_ENC2);
        receive = &(smemory->mutexENC2_P2);
        message = (smemory->messagePENC2);
    }

    if ((ENC = fork()) < 0)
    {
        perror("***Fork Failed***\n");
        exit(1);
    }
    else if (ENC == 0) //child process
    {
        sprintf(shmid_str, "%d", shmid); //shared memory id to string
        if (!isSecond)                   //p1
        {
            execl("enc", "enc", "-s", shmid_str, "-f", "0", (char *)NULL); //exec , enc.c file
        }
        else //p2
        {
            execl("enc", "enc", "-s", shmid_str, "-f", "1", (char *)NULL);
        }
        perror("***Execl Failed***\n");
        exit(1);
    }
    if (isSecond) // p2
    {
        // p2 receives message
        sem_wait(send);
        printf("-P2: %s\n", message); //received message
        sem_post(send);
    }
    if (!isSecond || strcmp(message, "TERM") != 0)
    {
        while (1)
        {
            // p sends message
            sem_wait(send);
            printf("Type a message:\n");
            fgets(message, MAX_MSG_SZ, stdin); //write message
            if (message[strlen(message) - 1] == '\n')
            {
                message[strlen(message) - 1] = '\0'; //avoid comparing "TERM" with "TERM\n"
            }

            sem_post(receive);
            if (strcmp(message, "TERM") == 0)
            {
                break; //stop loop
            }

            // p receives message
            sleep(1);
            sem_wait(send);
            printf("-P%d: %s\n", isSecond + 1, message); //received message
            if (strcmp(message, "TERM") == 0)
            {
                break;
            }
            sem_post(send);
        }
    }

    do
    {
        if (waitpid(ENC, &status, WUNTRACED | WCONTINUED) == -1) //wait for child to finish
        {
            return -1;
        }
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));

    //destroy semaphores
    if (sem_destroy(&smemory->mutexP1_ENC1) != 0)
    {
        perror("***Destroy Mutex Failed***\n");
        exit(1);
    }
    if (sem_destroy(&smemory->mutexENC1_P1) != 0)
    {
        perror("***Destroy Mutex Failed***\n");
        exit(1);
    }
    if (sem_destroy(&smemory->mutexCh_ENC1) != 0)
    {
        perror("***Destroy Mutex Failed***\n");
        exit(1);
    }
    if (sem_destroy(&smemory->mutexENC1_Ch) != 0)
    {
        perror("***Destroy Mutex Failed***\n");
        exit(1);
    }
    if (sem_destroy(&smemory->mutexP2_ENC2) != 0)
    {
        perror("***Destroy Mutex Failed***\n");
        exit(1);
    }
    if (sem_destroy(&smemory->mutexENC2_P2) != 0)
    {
        perror("***Destroy Mutex Failed***\n");
        exit(1);
    }
    if (sem_destroy(&smemory->mutexCh_ENC2) != 0)
    {
        perror("***Destroy Mutex Failed***\n");
        exit(1);
    }
    if (sem_destroy(&smemory->mutexENC2_Ch) != 0)
    {
        perror("***Destroy Mutex Failed***\n");
        exit(1);
    }
    if (sem_destroy(&smemory->mutexCheck) != 0)
    {
        perror("***Destroy Mutex Failed***\n");
        exit(1);
    }
    //destroy shared memory segment
    if (shmctl(shmid, IPC_RMID, 0) == -1)
    {
        perror("***Delete Shared Memory Failed***\n");
        exit(1);
    }
    exit(0);
}