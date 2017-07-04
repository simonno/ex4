/********************************
* Student name: Noam Simon      *
* Student ID: 208388850         *
* Course Exercise Group: 04     *
* Exercise name: Exercise 42    *
********************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define SHM_SIZE 256
#define SENGET_FLAGS (0644 | IPC_CREAT)
#define THREAD_POOL_CAPACITY 5
#define FTOK_ERROR "ftok error.\n"
#define SHMGET_ERROR "shmget error.\n"
#define SHMAT_ERROR "shmat error - error in attaching to the shared memory.\n"
#define PTHREAD_CREATE_ERROR "pthread_create error.\n"
#define OPEN_ERROR "open new file error.\n"
#define PTHREAD_MUTEX_INIT_ERROR "mutex init failed.\n"
#define NANOSLEEP_ERROR "nanosleep system call failed \n"
#define WRITE_ERROR "writing to the file failed.\n"
#define SEMGET_ERROR "semget error.\n"
#define PTHREAD_CANCEL_ERROR "pthread_cancel error.\n"
#define MUTEX_DESTROY_ERROR "mutex destroy failed.\n"
#define SEMAPHORE_REMOVE_ERROR "error - remove semaphore.\n"
#define SHMCTL_ERROR "shmctl error.\n"
#define SEMCTL_ERROR "semctl error.\n"
#define SEMOP_ERROR "semop error.\n"


// A linked list (LL) node to store a queue entry
typedef struct QNodeStruct QNode;
struct QNodeStruct{
    char value;
    QNode *next;
};

// The queue, front stores the front node of LL and rear stores ths
// last node of LL
typedef struct {
    QNode *front, *rear;
} Queue;

union semun {
    int val;	// value for SETVAL
    struct semid_ds *buf;	//buffer for IPC_STAT, IPC_SET
    unsigned short *array;	// array for GETALL, SETALL
};

pthread_t tid[THREAD_POOL_CAPACITY];
char* data;
Queue* jobQueue;
int internal_count;
pthread_mutex_t jobQueueLock;
int fd;
int shmid, semidRead, semidWrite;
union semun semarg;
pthread_mutex_t counterLock1;
pthread_mutex_t counterLock2;
pthread_mutex_t printLock;

void atExit();
void addJob(char value);
void* threadPool(void * args);
void threadsFunction(char mission);
QNode* deQueue(Queue *q);
void enQueue(Queue *q, char value);
Queue* createQueue();
QNode* newNode(char value);
void run();
void g();
void h();
void printCountToFile(unsigned long int tid);
void clearQueue(Queue *q) ;


int main(int argc, char *argv[]) {
    int i, err;
    key_t key;

    internal_count = 0;

    /*  create a job queue */
    jobQueue = createQueue();

    /* create a file with read and write permissions */
    fd = open("208388850.txt", O_CREAT | O_RDWR, 0666);
    if (fd < 0) {
        perror(OPEN_ERROR);
        exit(EXIT_FAILURE);
    }

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
    /* create a key for semaphore */
    // /home/noam/ClionProjects/OperationSystem/ex4---os/cmake-build-debug/
    key = ftok("208388850.txt", 'a');
    if (key == (key_t) -1) {
        perror(FTOK_ERROR);
        exit(EXIT_FAILURE);
    }
    semidRead = semget(key, 1, IPC_CREAT | IPC_EXCL | 0666);
    if (semidRead == -1) {
        perror(SEMGET_ERROR);
        exit(EXIT_FAILURE);
    }
    semarg.val = 0;
    if (semctl(semidRead, 0, SETVAL, semarg) == -1) {
        perror(SEMCTL_ERROR);
        exit(EXIT_FAILURE);
    }

    /* create a key for semaphore */
    // /home/noam/ClionProjects/OperationSystem/ex4---os/cmake-build-debug/
    key = ftok("208388850.txt", '6');
    if (key == (key_t) -1) {
        perror(FTOK_ERROR);
        exit(EXIT_FAILURE);
    }
    semidWrite = semget(key, 1, IPC_CREAT | IPC_EXCL | 0666);
    if (semidWrite == -1) {
        perror(SEMGET_ERROR);
        exit(EXIT_FAILURE);
    }
    semarg.val = 1;
    if (semctl(semidWrite, 0, SETVAL, semarg) == -1) {
        perror(SEMCTL_ERROR);
        exit(EXIT_FAILURE);
    }



    if (pthread_mutex_init(&jobQueueLock, NULL) != 0) {
        printf(PTHREAD_MUTEX_INIT_ERROR);
        exit(EXIT_FAILURE);
    }
    if (pthread_mutex_init(&counterLock1, NULL) != 0) {
        printf(PTHREAD_MUTEX_INIT_ERROR);
        exit(EXIT_FAILURE);
    }
    if (pthread_mutex_init(&counterLock2, NULL) != 0) {
        printf(PTHREAD_MUTEX_INIT_ERROR);
        exit(EXIT_FAILURE);
    }
    if (pthread_mutex_init(&printLock, NULL) != 0) {
        printf(PTHREAD_MUTEX_INIT_ERROR);
        exit(EXIT_FAILURE);
    }

    /* create the threads */
    for(i = 0; i <  THREAD_POOL_CAPACITY; i++) {
        err = pthread_create(&(tid[i]), NULL, &threadPool, NULL);
        if (err != 0) {
            perror(PTHREAD_CREATE_ERROR);
            exit(EXIT_FAILURE);
        }

    }
    run();
}

/****************************************************************************
* function name : run                                                       *
* input :                                                                   *
* output : void                                                             *
* explanation : run the server operation                                    *
****************************************************************************/
void run() {
    struct sembuf sb;
    char job;
    sb.sem_num = 0;
    sb.sem_flg = SEM_UNDO;
    while (1) {


        sb.sem_op = -1;
        if (semop(semidRead, &sb, 1) == -1) {
            perror(SEMOP_ERROR);
            exit(EXIT_FAILURE);
        }
        //read from the shared memory
        job = *data;
        sb.sem_op = 1;
        if (semop(semidWrite, &sb, 1) == -1) {
            perror(SEMOP_ERROR);
            exit(EXIT_FAILURE);
        }
        // check the job type.
        if(job == 'g' || job == 'h') {
            break;
        }
        // add the job to the job queue.
        addJob(job);
    }
    if (job == 'g') {
        g();
    } else {
        h();
    }
}

/****************************************************************************
* function name : h                                                         *
* input :                                                                   *
* output : void                                                             *
* explanation : run the h job operation                                     *
****************************************************************************/
void h() {
    int i, err;
    // cancel the threads operation.
    for(i = 0; i <  THREAD_POOL_CAPACITY; i++) {
        addJob('h');
    }
    for(i = 0; i <  THREAD_POOL_CAPACITY; i++) {
        //wait them (join)
        err = pthread_join(tid[i], NULL);
        if (err != 0) {
            perror(PTHREAD_CANCEL_ERROR);
            exit(EXIT_FAILURE);
        }
        printCountToFile(tid[i]);
    }
    printCountToFile(pthread_self());
    atExit();
    exit(EXIT_SUCCESS);
}

/****************************************************************************
* function name : g                                                         *
* input :                                                                   *
* output : void                                                             *
* explanation : run the g job operation                                     *
****************************************************************************/
void g() {
    int i, err;
    // cancel the threads operation.
    for(i = 0; i <  THREAD_POOL_CAPACITY; i++) {
        err = pthread_cancel(tid[i]);
        if (err != 0) {
            perror(PTHREAD_CANCEL_ERROR);
            exit(EXIT_FAILURE);
        }
    }
    printCountToFile(pthread_self());
    atExit();
    exit(EXIT_SUCCESS);
}

/****************************************************************************
* function name : addJob                                                    *
* input :                                                                   *
* output : void                                                             *
* explanation : add job to the jobs queue                                   *
****************************************************************************/
void addJob(char value){
    pthread_mutex_lock(&jobQueueLock);
    enQueue(jobQueue, value);
    pthread_mutex_unlock(&jobQueueLock);
}

/****************************************************************************
* function name : threadPool                                                *
* input :                                                                   *
* output : void                                                             *
* explanation : run the thread operation                                    *
****************************************************************************/
void* threadPool(void * args) {
    while (1) {
        // get the job
        pthread_mutex_lock(&jobQueueLock);
        QNode* job = deQueue(jobQueue);
        pthread_mutex_unlock(&jobQueueLock);
        if (job != NULL) {
            if (job->value == 'h') {
                break;
            }
            threadsFunction(job->value);
        }
        sleep(5);
    }
    return NULL;
}

/****************************************************************************
* function name : threadsFunction                                           *
* input : mission - type of the job                                         *
* output : void                                                             *
* explanation : run the thread operation according to the mission.          *
****************************************************************************/
void threadsFunction(char mission) {
    // random number between 10 to 100
    int x = (rand() % 101) + 10;
    struct timespec tim, tim2;
    tim.tv_sec = 0;
    tim.tv_nsec = x;

    // sleep random nanosecond
    if (nanosleep(&tim, &tim2) < 0) {
        perror(NANOSLEEP_ERROR);
        exit(EXIT_FAILURE);
    }

    int add = 0;
    switch (mission) {
        case 'a':
            add = 1;
            break;
        case 'b':
            add = 2;
            break;
        case 'c':
            add = 3;
            break;
        case 'd':
            add = 4;
            break;
        case 'e':
            add = 5;
            break;
        case 'f':
            printCountToFile(pthread_self());
            return;
        default:
            add = 0;
            break;
    }

    // add to internal count according to the job type.
    pthread_mutex_lock(&jobQueueLock);
    internal_count += add;
    pthread_mutex_unlock(&jobQueueLock);
}

/****************************************************************************
* function name : printCountToFile                                          *
* input : the tid number of the called thread                               *
* output : void                                                             *
* explanation : print the internal count value to the file.                 *
****************************************************************************/
void printCountToFile(unsigned long int tid) {
    char line[256];
    memset(line,0, sizeof(line));
    ssize_t err;
    // get the internal val.
    pthread_mutex_lock(&counterLock2);
    int counter = internal_count;
    pthread_mutex_unlock(&counterLock2);

    // print to the file.
    sprintf(line, "thread identifier is %lu and internal_count is %d\n", tid, counter);
    pthread_mutex_lock(&printLock);
    err = write(fd, line, sizeof(line));
    pthread_mutex_unlock(&printLock);

    // check for errors.
    if (err < 0) {
        perror(WRITE_ERROR);
        exit(EXIT_FAILURE);
    }
}



/****************************************************************************
* function name : newNode                                                   *
* input : value of the new node                                             *
* output : pointer to the new node.                                         *
* explanation : create a new linked list node.                              *
****************************************************************************/
QNode* newNode(char value) {
    QNode *temp = (QNode*)malloc(sizeof(QNode));
    temp->value = value;
    temp->next = NULL;
    return temp;
}

/****************************************************************************
* function name : createQueue                                               *
* input :                                                                   *
* output : pointer to the new queue.                                        *
* explanation : create an empty queue.                                      *
****************************************************************************/
Queue *createQueue() {
    Queue *q = (Queue*)malloc(sizeof(Queue));
    q->front = q->rear = NULL;
    return q;
}

/****************************************************************************
* function name : enQueue                                                   *
* input : q - pointer to queue, value                                       *
* output : void                                                             *
* explanation : add a value k to q                                          *
****************************************************************************/
void enQueue(Queue *q, char value) {
    // Create a new LL node
    QNode *temp = newNode(value);

    // If queue is empty, then new node is front and rear both
    if (q->rear == NULL)
    {
        q->front = q->rear = temp;
        return;
    }

    // Add the new node at the end of queue and change rear
    q->rear->next = temp;
    q->rear = temp;
}

/****************************************************************************
* function name : enQueue                                                   *
* input : q - pointer to queue                                              *
* output : pointer to node                                                  *
* explanation : remove a value from given queue q                           *
****************************************************************************/
QNode *deQueue(Queue *q) {
    // If queue is empty, return NULL.
    if (q->front == NULL)
        return NULL;

    // Store previous front and move front one node ahead
    QNode *temp = q->front;
    q->front = q->front->next;

    // If front becomes NULL, then change rear also as NULL
    if (q->front == NULL)
        q->rear = NULL;
    return temp;
}

/****************************************************************************
* function name : onExit                                                    *
* input :                                                                   *
* output : void                                                             *
* explanation : free all resources that were allocated at exit.             *
****************************************************************************/
void atExit() {
    if (pthread_mutex_destroy(&counterLock1) != 0) {
        perror(MUTEX_DESTROY_ERROR);
        exit(EXIT_FAILURE);
    }
    if (pthread_mutex_destroy(&jobQueueLock) != 0) {
        perror(MUTEX_DESTROY_ERROR);
        exit(EXIT_FAILURE);
    }
    if (semctl(semidRead, 0, IPC_RMID, semarg) != 0) {
        perror(SEMAPHORE_REMOVE_ERROR);
        exit(EXIT_FAILURE);
    }
    if ( semctl(semidWrite, 0, IPC_RMID, semarg) != 0) {
        perror(SEMAPHORE_REMOVE_ERROR);
        exit(EXIT_FAILURE);
    }
    /* remove the shared memory */
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror(SHMCTL_ERROR);
        exit(EXIT_FAILURE);
    }
    close(fd);
    clearQueue(jobQueue);
}

/****************************************************************************
* function name : clearQueue                                                *
* input : pointer to the queue                                              *
* output : void                                                             *
* explanation : free all nodes fo the queue.                                *
****************************************************************************/
void clearQueue(Queue *q) {
    while (q->front != NULL) {
        // Store previous front and move front one node ahead
        QNode *temp = q->front;
        q->front = q->front->next;
        free(temp);
    }
}