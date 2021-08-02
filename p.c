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