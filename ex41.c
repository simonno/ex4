/********************************
* Student name: Noam Simon      *
* Student ID: 208388850         *
* Course Exercise Group: 04     *
* Exercise name: Exercise 41    *
********************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <errno.h>


#define SHM_SIZE 256
#define SENGET_FLAGS (0644 | IPC_CREAT)
#define FTOK_ERROR "ftok error.\n"
#define SHMGET_ERROR "shmget error.\n"
#define SHMAT_ERROR "shmat error - error in attaching to the shared memory.\n"
#define SEMGET_ERROR "semget error.\n"
#define SEMOP_ERROR "semop error.\n"

char toLower(char chr);

int semidRead, semidWrite;
union semun {
    int val;
    struct semid_ds *buf;
    ushort *array;
};
union semun semarg;

/****************************************************************************
* function name : main                                                      *
* input :                                                                   *
* output :                                                                  *
* explanation :                                                             *
****************************************************************************/
int main(int argc, char *argv[]) {
    char requestCode;
    int shmid;
    char* data;
    key_t key;
    struct sembuf sb;

    /* create a value for sheared memory */
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

    /* create a value for semaphore */
    key = ftok("208388850.txt", 'a');
    if (key == (key_t) -1) {
        perror(FTOK_ERROR);
        exit(EXIT_FAILURE);
    }
    semidRead = semget(key, 0, 0666);
    if (semidRead == -1) {
        perror(SEMGET_ERROR);
        exit(EXIT_FAILURE);
    }

    /* create a value for semaphore */
    key = ftok("208388850.txt", '6');
    if (key == (key_t) -1) {
        perror(FTOK_ERROR);
        exit(EXIT_FAILURE);
    }
    semidWrite = semget(key, 0, 0666);
    if (semidWrite == -1) {
        perror(SEMGET_ERROR);
        exit(EXIT_FAILURE);
    }

    sb.sem_num = 0;
    sb.sem_flg = SEM_UNDO;
    char dummay;
    while (1) {

        printf("Please enter request code\n");
        scanf("%c%c", &requestCode, &dummay);

        if  (requestCode == 'i' || requestCode == 'I') {
            break;
        }
        requestCode = toLower(requestCode);

        sb.sem_op = -1;
        if (semop(semidWrite, &sb, 1) == -1) {
            if (errno == 22 || errno == 43) {
                exit(EXIT_SUCCESS);
            }
            perror(SEMOP_ERROR);
            exit(EXIT_FAILURE);
        }
        //write the job to shared memory
        *data = requestCode;
        sb.sem_op = 1;
        if (semop(semidRead, &sb, 1) == -1) {
            if (errno == 22 || errno == 43) {
                exit(EXIT_SUCCESS);
            }
            perror(SEMOP_ERROR);
            exit(EXIT_FAILURE);
        }
    }
}

/****************************************************************************
* function name : toLower                                                   *
* input : char to convert to lower case                                     *
* output : the lower case char                                              *
* explanation : covert upper case char to lower case                        *
****************************************************************************/
char toLower(char chr) {
    if (chr >='A' && chr<='Z') {
        return (char) (chr + 32);
    }
    return chr;
}