#ifndef STRUCTS_H
#define STRUCTS_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>

#define MAX_MSG_SZ 100

typedef struct shared_memory
{
    sem_t mutexP1_ENC1; //p1 to enc1
    sem_t mutexENC1_Ch; //enc1 to chan
    sem_t mutexCh_ENC2; //chan to enc2
    sem_t mutexENC2_P2; //enc2 to p2

    sem_t mutexP2_ENC2; //p2 to enc2
    sem_t mutexENC2_Ch; //enc2 to chan
    sem_t mutexCh_ENC1; //chan to enc1
    sem_t mutexENC1_P1; //enc1 to p1

    sem_t mutexCheck; //checksum

    char messagePENC1[MAX_MSG_SZ];  //p1-enc1
    char messageENCCh1[MAX_MSG_SZ]; //enc1-chan
    char messageENCCh2[MAX_MSG_SZ]; //enc2-chan
    char messagePENC2[MAX_MSG_SZ];  //p2-enc2
    float probability;              //probability of mistake
    unsigned long checksum1;        //first
    unsigned long checksum2;        //second
    int isSame;                     //check if checksums are same

} shared_memory;

unsigned long hash(unsigned char *s) //function to calculate hash
{
    unsigned long hash = 7151;
    int temp, i;

    for (i = 0; i < strlen(s); i++)
    {
        temp = *s++;
        hash = ((hash << 5) + hash) + temp;
    }
    return hash;
}

#endif