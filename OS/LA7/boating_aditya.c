#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

/*Semaphore Structs*/
/*{ init_value, PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER }*/
typedef struct
{
    int value;
    pthread_mutex_t mtx;
    pthread_cond_t cv;
} semaphore;

/*P(wait Operation)*/
void P(semaphore *s)
{
    pthread_mutex_lock(&s->mtx);
    while (s->value <= 0)
    {
        // Wait if no available resource
        pthread_cond_wait(&s->cv, &s->mtx);
    }
    s->value--; // Take the resource
    pthread_mutex_unlock(&s->mtx);
}

/*V(signal Operation)*/
void V(semaphore *s)
{
    pthread_mutex_lock(&s->mtx);
    s->value++;                  // Release the resource
    pthread_cond_signal(&s->cv); // Wake up a waiting thread
    pthread_mutex_unlock(&s->mtx);
}

/*Number of boats and Riders*/
int m, n;
int numberofVisitorsAlreadyLeft = 0;
int numberofAlreadyServerd = 0;

/*Semaphores*/
semaphore boatSem = {0, PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER};
semaphore riderSem = {0, PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER};

/*Mutex*/
pthread_mutex_t bmtx;
pthread_barrier_t EOS;

/*Global Variables and Arrays*/
int BA[11]; // At max 10 Boats
int BC[11];
pthread_barrier_t BB[11];
int BT[11];

/*Boat-Threads*/
void *boat(void *arg)
{
    /* Local variables */
    int boatId = *(int *)arg;
    printf("Boat    %2d : Ready\n", boatId);
    while (1)
    {
        /*lock the mutex*/
        pthread_mutex_lock(&bmtx);
        if (numberofAlreadyServerd == n)
        {
            pthread_mutex_unlock(&bmtx);
            pthread_exit(NULL);
        }
        pthread_mutex_unlock(&bmtx);

        // Boat is Already Marked to be availabe...

        // Send a signal to the Rider Semaphore
        V(&riderSem);

        // Wait on the boat Semaphore...
        P(&boatSem);

        BA[boatId] = 1;
        BC[boatId] = -1;
        BT[boatId] = 0;

        // Wait on the Barrier Untill rider confirms the boat
        pthread_barrier_wait(&BB[boatId]);

        /*Lock the Mutex*/
        pthread_mutex_lock(&bmtx);

        BA[boatId] = 0;
        int rtime = BT[boatId];
        numberofAlreadyServerd++;

        /*Unlock the Mutex*/
        pthread_mutex_unlock(&bmtx);

        printf("Boat    %2d : Starts of ride for visitor %d\n", boatId, BC[boatId]);

        /*Enjoy boating with your Customer*/
        usleep(rtime * 100000);

        printf("Boat    %2d : End of ride for visitor %d (rtime = %d)\n", boatId, BC[boatId], BT[boatId]);

        /*Lock Mutex to Increment the numberofAlreadyLeft by 1*/
        pthread_mutex_lock(&bmtx);

        if (numberofVisitorsAlreadyLeft == n)
        {
            pthread_mutex_unlock(&bmtx);
            pthread_barrier_wait(&EOS);
            exit(0);
        }

        /*Else Unlock the Mutex*/
        pthread_mutex_unlock(&bmtx);
    }
}

/*Rider-Threads*/
void *rider(void *arg)
{
    /* Local variables */
    int riderId = *(int *)arg;

    /*Print Ready Message*/
    printf("Visitor %2d : Ready\n", riderId);

    /*Generate a random vtime and rtime*/
    int vtime = 30 + rand() % (120 - 30 + 1);
    int rtime = 15 + rand() % (60 - 15 + 1);

    /*print visiting Message*/
    printf("Visitor %2d : starts sightseeing for %d mins\n", riderId, vtime);

    /*Visit Other Attractions for vtime*/
    usleep(vtime * 100000);

    /*print-Ready to ride Message*/
    printf("Visitor %2d : Ready to ride (rtime = %d)\n", riderId, rtime);

    /*Send a signal to the boat semaphore*/
    V(&boatSem);

    /*Wait on the Rider semaphore*/
    P(&riderSem);

    /*Get an available Boat:Search*/
    int idx_flag = -1;
    while (idx_flag == -1)
    {
        /*Get the Mutex*/
        pthread_mutex_lock(&bmtx);

        for (int i = 1; i <= m; i++)
        {
            if (BA[i] == 1 && BC[i] == -1)
            {
                /*Acquired this free-boat*/
                idx_flag = i;
                BC[idx_flag] = riderId;
                BT[idx_flag] = rtime;
                break;
            }
        }

        /*Release the Mutex*/
        pthread_mutex_unlock(&bmtx);
    }

    printf("Visitor %2d : finds boat %d\n", riderId, idx_flag);

    /*Release the Mutex*/
    pthread_mutex_unlock(&bmtx);

    /*Join the Barrier*/
    pthread_barrier_wait(&BB[idx_flag]);

    /*Enjoy Boating With your Boater...*/
    usleep(rtime * 100000);

    /*Leave the Zoo*/
    printf("Visitor %2d : Leaves the Zoo\n", riderId);

    /*Lock the mutex*/
    pthread_mutex_lock(&bmtx);
    numberofVisitorsAlreadyLeft++;
    if (numberofVisitorsAlreadyLeft == n)
    {
        sleep(1);
        exit(0);
    }
    pthread_mutex_unlock(&bmtx);
}

int main(int argc, char *argv[])
{
    /*Seed the time*/
    srand(time(NULL));

    if (argc < 3)
    {
        printf("Error : Usage => ./boating m n\n");
        exit(1);
    }

    /*Read m and n*/
    m = atoi(argv[1]);
    n = atoi(argv[2]);

    if ((m < 5 || m > 10) || (n < 20 || n > 100))
    {
        printf("Error : Usage => m[5-10] n[20-100]\n");
        exit(1);
    }

    /*Initializing bmtx*/
    pthread_mutex_init(&bmtx, NULL);

    /*Init bmtx to 1: i.e unlock*/
    pthread_mutex_trylock(&bmtx);
    pthread_mutex_unlock(&bmtx);

    /*Init barrier(EOS) to 2*/
    pthread_barrier_init(&EOS, NULL, 2);

    /*Create m boat Threads and n rider Threads*/
    pthread_t boatThread[m + 1];
    int boatargs[m + 1];
    pthread_t riderThread[n + 1];
    int riderargs[n + 1];

    /*Init the args*/
    for (int i = 1; i <= m; i++)
    {
        boatargs[i] = i;
        pthread_barrier_init(&BB[i], NULL, 2);
    }
    for (int i = 1; i <= n; i++)
    {
        riderargs[i] = i;
    }

    /*Create Boat Threads*/
    for (int i = 1; i <= m; i++)
    {
        pthread_create(&boatThread[i], NULL, boat, &boatargs[i]);
    }

    /*Create Rider Threads*/
    for (int i = 1; i <= n; i++)
    {
        pthread_create(&riderThread[i], NULL, rider, &riderargs[i]);
    }

    /*Wait on EOS*/
    pthread_barrier_wait(&EOS);
}