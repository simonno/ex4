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


#define SHM_SIZE 256
#define SENGET_FLAGS (0644 | IPC_CREAT)
#define FTOK_ERROR "ftok error"
#define SHMGET_ERROR "shmget error"
#define SHMAT_ERROR "shmat error - error in attaching to the shared memory."
#define SEMGET_ERROR "semget error.\n"


char toLower(char chr);

int semidRead, semidWrite;
union semun {
    int val;
    struct semid_ds *buf;
    ushort *array;
};
union semun semarg;

int main(int argc, char *argv[]) {
    char requestCode;
    int shmid;
    char* data;
    key_t key;
    struct sembuf sb;

    /* create a value for sheared memory */
    // /home/noam/ClionProjects/OperationSystem/ex3/cmake-build-debug/
    key = ftok("/home/noam/ClionProjects/OperationSystem/ex4---os/cmake-build-debug/208388850.txt", 'N');
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

    /* create a value for semaphor */
    // /home/noam/ClionProjects/OperationSystem/ex3/cmake-build-debug/
    key = ftok("/home/noam/ClionProjects/OperationSystem/ex4---os/cmake-build-debug/208388850.txt", 'M');
    if (key == (key_t) -1) {
        perror(FTOK_ERROR);
        exit(EXIT_FAILURE);
    }
    semidRead = semget(key, 0, 0666);
    if (semidRead == -1) {
        perror(SEMGET_ERROR);
        exit(EXIT_FAILURE);
    }

    /* create a value for semaphor */
    // /home/noam/ClionProjects/OperationSystem/ex3/cmake-build-debug/
    key = ftok("/home/noam/ClionProjects/OperationSystem/ex4---os/cmake-build-debug/208388850.txt", 'U');
    if (key == (key_t) -1) {
        perror(FTOK_ERROR);
        exit(EXIT_FAILURE);
    }
    semidWrite = semget(key, 0, 0666);
    if (semidWrite == -1) {
        perror(SEMGET_ERROR);
        exit(EXIT_FAILURE);
    }

//    semarg.val = 0;
//    semctl(semidRead, 0, IPC_SET, semarg);
//    semarg.val = 0;
//    semctl(semidWrite, 0, IPC_SET, semarg);
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
        semop(semidWrite, &sb, 1);
        *data = requestCode;
        sb.sem_op = 1;
        semop(semidRead, &sb, 1);
    }
}

char toLower(char chr) {
    if (chr >='A' && chr<='Z') {
        return (char) (chr + 32);
    }
    return chr;
}