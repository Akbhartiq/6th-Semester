#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>

// Shm size and max_size of the hash array
#define SHM_SIZE 104
#define MAX_SIZE 1000

using namespace std;

int main(int argc, char *argv[])
{
    int n = argc > 1 ? atoi(argv[1]) : 10;

    // Get the Key
    key_t key = ftok("/", 'A');
    if (key == -1)
    {
        perror("Key is not Generated!!\n");
        exit(1);
    }

    // Create the shm
    int shmid;
    shmid = shmget(key, (SHM_SIZE) * sizeof(int), 0777 | IPC_CREAT | IPC_EXCL);

    // Attach the Memory
    int *M = (int *)shmat(shmid, 0, 0);
    if (M == (void *)-1)
    {
        perror("Memory can't be Attached\n");
        exit(1);
    }

    // Init
    M[0] = n;
    M[1] = 0;
    M[2] = 0;

    // Seeding...
    srand(time(0));

    // Hash-Array
    int hash[MAX_SIZE] = {0};

    // Busy Wait
    while (M[1] != n)
        ;

    // Flags to check if we have to exit and begin(printting)
    int begin_flag = 0;
    int exit_flag = 0;
    while (1)
    {
        // Grap the turn
        if (M[2] == 0)
        {
            // If exit turn
            if (exit_flag)
            {
                // Erase the M and exit
                if (shmctl(shmid, IPC_RMID, NULL) == -1)
                {
                    perror("shmctl IPC_RMID");
                    shmdt(M);
                    exit(1);
                }
                else
                {
                    cout << endl
                         << "****Completed****" << endl;
                    shmdt(M);
                    return 0;
                }
            }
            if (begin_flag)
            {
                //  Print the sum
                int sum = M[3];
                cout << M[3];
                for (int i = 4; i <= n + 3; i++)
                {
                    sum += M[i];
                    cout << " + " << M[i];
                }
                cout << " = " << sum << endl;

                // Check if sum is Already Calculated...
                if (hash[sum] == 1)
                {
                    exit_flag = 1;
                    M[2] = -1; // Statt Exitting process
                    continue;
                }
                hash[sum] = 1; // Marked sum as 1
            }
            int rand_num = rand() % 99 + 1;
            M[3] = rand_num;
            ++M[2];
            begin_flag = 1;
        }
    }

    return 0;
}