/********************************
* Student name: Noam Simon      *
* Student ID: 208388850         *
* Course Exercise Group: 04     *
* Exercise name: Exercise 41    *
********************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>


#define SHM_SIZE 1024  /* make it a 1K shared memory segment */
#define FTOK_ERROR "ftok error"
#define SHMGET_ERROR "shmget error"
#define SENGET_FLAGS (0644 | IPC_CREAT)
#define SHMAT_ERROR "shmat error - error in attaching to the shared memory."

char toLower(char chr);



int main(int argc, char *argv[]) {
    char requestCode;
    int shmid;
    char* data;
    key_t key;

    /* create a value for sheared memory */
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


    while (1) {
        printf("Please enter request code\n");
        scanf("%c", &requestCode);

        if  (requestCode == 'i' && requestCode == 'I') {
            break;
        }
        requestCode = toLower(requestCode);

        *data = requestCode;
    }
}

char toLower(char chr) {
    if (chr >='A' && chr<='Z') {
        return (char) (chr + 32);
    }
    return chr;
}