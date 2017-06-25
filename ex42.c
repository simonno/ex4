/********************************
* Student name: Noam Simon      *
* Student ID: 208388850         *
* Course Exercise Group: 04     *
* Exercise name: Exercise 42    *
********************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <pthread.h>

#define SHM_SIZE 1024  /* make it a 1K shared memory segment */
#define FTOK_ERROR "ftok error"
#define SHMGET_ERROR "shmget error"
#define SENGET_FLAGS (0644 | IPC_CREAT)
#define SHMAT_ERROR "shmat error - error in attaching to the shared memory."
#define THREADS_CAPCITIY 5
#define PTHREAD_CREATE_ERROR "pthread_create error.\n"

pthread_t tid[10];
char* data;

int main(int argc, char *argv[]) {
    int shmid, i, err;
    key_t key;

    for(i = 0; i <  THREADS_CAPCITIY; i++) {
        err = pthread_create(&(tid[i]), NULL, &doSomeThing, NULL);
        if (err != 0)
            perror(PTHREAD_CREATE_ERROR);
        i++;
    }
        /* create a key for sheared memory */
    // /home/noam/ClionProjects/OperationSystem/ex3/cmake-build-debug/
    key = ftok("208388850.txt", 'N');
    if (key == (key_t) -1) {
        perror(FTOK_ERROR);
        exit(EXIT_FAILURE);
    }

    /* connect to the segment */
    shmid = shmget(key, SHM_SIZE, SENGET_FLAGS);
    if (shmid == -1) {
        perror(SHMGET_ERROR);
        exit(EXIT_FAILURE);
    }

    /* attach the segment to our data space */
    data = shmat(shmid, NULL, 0);
    if (data == (char *) -1) {
        perror(SHMAT_ERROR);
        exit(EXIT_FAILURE);
    }

}