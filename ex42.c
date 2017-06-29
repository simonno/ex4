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

#define SHM_SIZE 256
#define SENGET_FLAGS (0644 | IPC_CREAT)
#define FTOK_ERROR "ftok error.\n"
#define SHMGET_ERROR "shmget error.\n"
#define SHMAT_ERROR "shmat error - error in attaching to the shared memory.\n"
#define PTHREAD_CREATE_ERROR "pthread_create error.\n"
#define OPEN_ERROR "open new file error.\n"
#define THREAD_POOL_CAPACITY 5
#define PTHREAD_MUTEX_INIT_ERROR "mutex init failed.\n"
#define NANOSLEEP_ERROR "nanosleep system call failed \n"
#define WRITE_ERROR "writing to the file failed.\n"
#define SEMGET_ERROR "semget error.\n"

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
    int val;
    struct semid_ds *buf;
    ushort *array;
};

pthread_t tid[THREAD_POOL_CAPACITY];
char* data;
Queue* jobQueue;
int internal_count;
pthread_mutex_t jobQueueLock;
int fd;
int shmid, semidRead, semidWrite;
union semun semarg;
pthread_mutex_t counterLock;

void addJob(char value);
void* threadPool(void * args);
void threadsFunction(char mission);
QNode* deQueue(Queue *q);
void enQueue(Queue *q, char value);
Queue* createQueue();
QNode* newNode(char value);

void run();

int main(int argc, char *argv[]) {
    int i, err;
    key_t key;

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

    semidRead = semget(key, 1, IPC_CREAT | IPC_EXCL | 0666);
    semidWrite = semget(key, 1, IPC_CREAT | IPC_EXCL | 0666);
    if (semidRead == -1 ||  semidWrite == -1) {
        perror(SEMGET_ERROR);
        exit(EXIT_FAILURE);
    }
    semarg.val = 0;
    semctl(semidRead, 0, IPC_SET, semarg);
    semarg.val = 0;
    semctl(semidWrite, 0, IPC_SET, semarg);



    if (pthread_mutex_init(&jobQueueLock, NULL) != 0) {
        printf(PTHREAD_MUTEX_INIT_ERROR);
        exit(EXIT_FAILURE);
    }


    if (pthread_mutex_init(&counterLock, NULL) != 0) {
        printf(PTHREAD_MUTEX_INIT_ERROR);
        exit(EXIT_FAILURE);
    }

    /* create the threads */
    for(i = 0; i <  THREAD_POOL_CAPACITY; i++) {
        err = pthread_create(&(tid[i]), NULL, &threadPool, NULL);
        if (err != 0)
            perror(PTHREAD_CREATE_ERROR);
    }

    run();

}

void run() {
    struct sembuf sb;
    char job;
    sb.sem_num = 0;
    sb.sem_flg = SEM_UNDO;
    while (1) {
        sb.sem_op = -1;
        semop(semidRead, &sb, 1);
        job = *data;
        sb.sem_op = 1;
        semop(semidWrite, &sb, 1);
        addJob(job);
    }
}

void addJob(char value){
    pthread_mutex_lock(&jobQueueLock);
    enQueue(jobQueue, value);
    pthread_mutex_unlock(&jobQueueLock);
}

void* threadPool(void * args) {
    while (1) {
        pthread_mutex_lock(&jobQueueLock);
        QNode* job = deQueue(jobQueue);
        pthread_mutex_unlock(&jobQueueLock);
        threadsFunction(job->value);
    }
}

void threadsFunction(char mission) {
    char line[256];
    int tid;

    int x = (rand() % 101) + 10;
    struct timespec tim, tim2;
    tim.tv_sec = 0;
    tim.tv_nsec = x;

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
            tid = (int) pthread_self();
            sprintf(line, "thread identifier is %d and internal_count is %d", tid, internal_count);
            if (write(fd, line, sizeof(line)) < 0) {
                perror(WRITE_ERROR);
                exit(EXIT_FAILURE);
            }
            return;
        default:
            add = 0;
            break;
    }

    pthread_mutex_lock(&jobQueueLock);
    internal_count += add;
    pthread_mutex_unlock(&jobQueueLock);
}




// A utility function to create a new linked list node.
QNode* newNode(char value) {
    QNode *temp = (QNode*)malloc(sizeof(QNode));
    temp->value = value;
    temp->next = NULL;
    return temp;
}

// A utility function to create an empty queue
Queue *createQueue() {
    Queue *q = (Queue*)malloc(sizeof(Queue));
    q->front = q->rear = NULL;
    return q;
}

// The function to add a value k to q
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

// Function to remove a value from given queue q
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