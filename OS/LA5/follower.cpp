#include <iostream>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <time.h>

#define SHM_SIZE 104

using namespace std;

int main(int argc, char *argv[])
{
    int nf = argc > 1 ? atoi(argv[1]) : 1;

    for (int i = 1; i < nf; i++)
    {
        if (fork() == 0)
        {
            execlp("./follower", "./follower", NULL);
        }
    }

    // Get the shm
    sleep(1);
    key_t key = ftok("/", 'A');
    if (key == -1)
    {
        perror("Follower fails to get the key\n");
        exit(1);
    }

    int shmid = shmget(key, SHM_SIZE, 0777);
    if (shmid == -1)
    {
        perror("Follower fails to get the shm\n");
        exit(1);
    }

    // Attach the Memory
    int *M = (int *)shmat(shmid, 0, 0);
    if (M == (void *)-1)
    {
        perror("Follower fails to attach to shm\n");
        exit(1);
    }

    // Get the number i
    if (M[1] == M[0])
    {
        cout << "Follower error : " << M[0] << " Followers have already Joined" << endl;
        exit(1);
    }

    int i = ++M[1];
    cout << "Follower " << i << " joins" << endl;

    // Init the program
    while (1)
    {
        // Grap the +ve turn
        if (M[2] == i)
        {
            // Generate random number
            int rand_num = rand() % 9 + 1;
            // write it
            M[3 + i] = rand_num;
            // incremnet it
            M[2] = M[2] + 1;
            // reset to zero
            if (M[2] > M[0])
                M[2] = 0;
        }
        if (M[2] == -i)
        {
            // Decrement it
            M[2] = M[2] - 1;
            // Reset to zero
            if (M[2] < -M[0])
                M[2] = 0;
            // wait for childrens to exit
            for (int i = 1; i < nf; i++)
                wait(NULL);

            // Dettach before you leave
            shmdt(M);
            cout << "Follower " << i << " leaves" << endl;
            return 0;
        }
    }
}