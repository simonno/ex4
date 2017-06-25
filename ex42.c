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
// A linked list (LL) node to store a queue entry
typedef struct QNodeStruct QNode;
struct QNodeStruct{
    int value;
    QNode *next;
};

// The queue, front stores the front node of LL and rear stores ths
// last node of LL
typedef struct {
    QNode *front, *rear;
} Queue;

pthread_t tid[10];
char* data;

void *threadsFunction(void *);
QNode *deQueue(Queue *q);
void enQueue(Queue *q, int value);
Queue *createQueue();
QNode* newNode(int k);

int main(int argc, char *argv[]) {
    int shmid, i, err;
    key_t key;



    for(i = 0; i <  THREADS_CAPCITIY; i++) {
        err = pthread_create(&(tid[i]), NULL, &threadsFunction, NULL);
        if (err != 0)
            perror(PTHREAD_CREATE_ERROR);
        i++;
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

}

void *threadsFunction(void * args) {
    return NULL;
}




// A utility function to create a new linked list node.
QNode* newNode(int k) {
    QNode *temp = (QNode*)malloc(sizeof(QNode));
    temp->value = k;
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
void enQueue(Queue *q, int value) {
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