#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <stdlib.h>
#include <sstream>
#include <tuple>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <list>

using namespace std;

/*Macros*/
#define RELEASE 1000
#define ADDITIONAL 1001
#define QUIT 1002
#define SCALE 100

/*Input Directory*/
const std::string inpDir = "input1";

/*Global Variables...*/
int n, m;
int numberOfLiveUser;
pthread_barrier_t BOS;
pthread_barrier_t REQB;
vector<pthread_barrier_t> ACKB;
pthread_mutex_t rmtx;
pthread_mutex_t pmtx;
vector<pthread_cond_t> cv;
vector<pthread_mutex_t> cmtx;

/*shared Mem...*/
vector<int> shm;

/*Int to string in 2 Dig Format...*/
string toTwoDigigString(int num)
{
    char buffer[3];
    sprintf(buffer, "%02d", num);
    return string(buffer);
}

/*Function to get corresponding str*/
string toString(int reqType)
{
    if (reqType == RELEASE)
    {
        return "RELEASE";
    }
    else if (reqType == QUIT)
    {
        return "QUIT";
    }
    return "ADDITIONAL";
}

/*Function to get the Max-Need*/
void getMaxNeed(vector<int> &NEED, int userId)
{
    // Open the File and read the Need...

    string inpFile = inpDir + "/thread" + toTwoDigigString(userId) + ".txt";

    ifstream file(inpFile);
    if (!file)
    {
        pthread_mutex_lock(&pmtx);
        cout << "User " << userId << " : Error opening the file" << endl;
        pthread_mutex_unlock(&pmtx);
        exit(1);
    }

    string line;

    /*Read the Need*/
    getline(file, line);
    stringstream ss(line);
    int num;
    while (ss >> num)
    {
        NEED.push_back(num);
    }

    if (NEED.size() != (long unsigned int)m)
    {
        pthread_mutex_lock(&pmtx);
        cout << "User " << userId << " : Error Need size is not m" << endl;
        pthread_mutex_unlock(&pmtx);
        exit(1);
    }
}

/*Function to get the next Request from a file...*/
tuple<int, char, vector<int>> getReq(ifstream &file)
{
    /*Read the line..*/
    string line;
    getline(file, line);
    stringstream ss(line);

    tuple<int, char, vector<int>> res;
    ss >> get<0>(res); // Read the Delay...
    ss >> get<1>(res); // Read the R/Q

    /*Read the request...*/
    int num;
    while (ss >> num)
    {
        get<2>(res).push_back(num);
    }

    return res;
}

/*Function to check type of resources...*/
int typeofReq(vector<int> &req)
{
    if (req.size() == 0)
        return QUIT;

    if (req.size() != (long unsigned int)m)
    {
        pthread_mutex_lock(&pmtx);
        cout << "Error : REQ size is not m" << endl;
        pthread_mutex_unlock(&pmtx);
        exit(1);
    }

    for (int i = 0; i < m; i++)
    {
        if (req[i] > 0)
        {
            return ADDITIONAL;
        }
    }

    return RELEASE;
}

/*Function to compare to m vectors*/
bool lessThan(vector<int> &a, vector<int> &b)
{
    if (a.size() != b.size())
    {
        pthread_mutex_lock(&pmtx);
        cout << "size of a != size of b" << endl;
        pthread_mutex_unlock(&pmtx);
        exit(1);
    }

    if (a.size() != (long unsigned int)m)
    {
        pthread_mutex_lock(&pmtx);
        cout << "size of a and b != m" << endl;
        pthread_mutex_unlock(&pmtx);
        exit(1);
    }

    for (int i = 0; i < m; i++)
    {
        if (a[i] > b[i])
            return false;
    }
    return true;
}

/*Bankers'Algorithm*/
bool isSafe(vector<vector<int>> &NEED, vector<vector<int>> &ALLOCATION, vector<int> &AVAILABLE)
{
    /*Number of processes*/
    int n = ALLOCATION.size();
    int m = AVAILABLE.size();

    /*Work*/
    vector<int> work(AVAILABLE);

    /*Finish*/
    vector<int> finish(n, false);

    /*MAX_REQ*/
    vector<vector<int>> MAX_REQ(n, vector<int>(m));

    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < m; j++)
        {
            MAX_REQ[i][j] = NEED[i][j] - ALLOCATION[i][j];
        }
    }

    /*Banker's Loop...*/
    while (1)
    {
        // Found flag...
        bool found = false;

        // Find the job that can be satisfied...
        for (int i = 0; i < n; i++)
        {
            // Unfinished Job...
            if (finish[i] == false)
            {
                // MAX_REQ[i]<=work
                bool flag = lessThan(MAX_REQ[i], work);

                if (flag)
                {
                    /*Can be satisfied...*/
                    // work  = work + ALLOCATION[i]

                    for (int j = 0; j < m; j++)
                    {
                        work[j] += ALLOCATION[i][j];
                    }
                    finish[i] = true;
                    found = true;
                    break;
                }
            }
        }

        if (found == false)
            break;
    }

    /*See If all the Job has finished or not...*/
    for (int i = 0; i < n; i++)
    {
        if (finish[i] == false)
            return false;
    }
    return true;
}

/*User Thread...*/
void *user(void *arg)
{
    // Get the userId
    int userId = *(int *)arg;
    delete (int *)arg;

    // print the arrival Message
    pthread_mutex_lock(&pmtx);
    cout << "\tThread " << userId << " born" << endl;
    pthread_mutex_unlock(&pmtx);

    // Open the File and read the Need...
    vector<int> NEED;

    // Get the Max-Need
    getMaxNeed(NEED, userId);

    // Escape the 1st Line for Rest of Simulation...
    string inpFile = inpDir + "/thread" + toTwoDigigString(userId) + ".txt";

    ifstream file(inpFile);
    if (!file)
    {
        pthread_mutex_lock(&pmtx);
        cout << "User " << userId << " : Error opening the file" << endl;
        pthread_mutex_unlock(&pmtx);
        exit(1);
    }

    string line;

    /*Read the Need*/
    getline(file, line);

    /*wait on BOS*/
    pthread_barrier_wait(&BOS);

    /*Enter the working Loop...*/
    while (1)
    {
        /*Read the next Request from file...*/
        tuple<int, char, vector<int>> reqTuple = getReq(file);
        int delay = get<0>(reqTuple);
        vector<int> req = get<2>(reqTuple);

        /*sleep*/
        usleep(SCALE * delay);

        /*Release or Additional*/
        int reqCat = typeofReq(req);

        /*Lock rmtx...*/
        pthread_mutex_lock(&rmtx);

        /*write the request...*/
        shm[0] = userId;
        shm[1] = reqCat;

        if (reqCat != QUIT)
        {
            for (int i = 2; i <= m + 1; i++)
            {
                shm[i] = req[i - 2];
            }
        }

        pthread_mutex_lock(&pmtx);
        cout << "\tThread " << userId << " send resource request: type = " << toString(reqCat) << endl;
        pthread_mutex_unlock(&pmtx);

        /*Join REQB*/
        pthread_barrier_wait(&REQB);

        /*Join ACKBi*/
        pthread_barrier_wait(&ACKB[userId]);

        /*Release mutex*/
        pthread_mutex_unlock(&rmtx);

        /*Release Request*/
        if (reqCat == RELEASE)
        {
            pthread_mutex_lock(&pmtx);
            cout << "\tThread " << userId << " is done with its resource release request" << endl;
            pthread_mutex_unlock(&pmtx);
            continue;
        }
        if (reqCat == QUIT)
        {
            pthread_mutex_lock(&pmtx);
            cout << "\tThread " << userId << " is going to quit" << endl;
            pthread_mutex_unlock(&pmtx);
            return NULL;
        }

        /*Conditional wait...*/
        pthread_mutex_lock(&cmtx[userId]);
        pthread_cond_wait(&cv[userId], &cmtx[userId]);
        pthread_mutex_unlock(&cmtx[userId]);

        pthread_mutex_lock(&pmtx);
        cout << "\tThread " << userId << " is granted its last resource request" << endl;
        pthread_mutex_unlock(&pmtx);
    }
}

/*Main Thread...*/
int main()
{
    string sysFile = inpDir + "/system.txt";

    ifstream file(sysFile);
    if (!file)
    {
        pthread_mutex_lock(&pmtx);
        cout << "Error opening the " << sysFile << endl;
        pthread_mutex_unlock(&pmtx);
        exit(1);
    }

    string line;

    /*Read m*/
    getline(file, line);
    m = stoi(line);

    /*Read n*/
    getline(file, line);
    n = stoi(line);
    numberOfLiveUser = n;

    /*Read AVAILABLE*/
    vector<int> AVAILABLE;

    getline(file, line);
    stringstream ss(line);
    int num;

    while (ss >> num)
    {
        AVAILABLE.push_back(num);
    }

    if (AVAILABLE.size() != (long unsigned int)m)
    {
        pthread_mutex_lock(&pmtx);
        cout << "master: number of Resources is not m!!" << endl;
        pthread_mutex_unlock(&pmtx);
        exit(1);
    }

    /*ALLOCATION*/
    vector<vector<int>> ALLOCATION(n, vector<int>(m, 0));

    /*NEED*/
    vector<vector<int>> NEED(n);
    for (int i = 0; i < n; i++)
    {
        getMaxNeed(NEED[i], i);
    }

    /*Barriers and mutex...*/
    pthread_barrier_init(&BOS, NULL, n + 1); // Initialize barrier
    pthread_barrier_init(&REQB, NULL, 2);    // Initialize barrier
    ACKB.resize(n);
    for (int i = 0; i < n; i++)
    {
        pthread_barrier_init(&ACKB[i], NULL, 2);
    }
    // Initializing rmtx
    pthread_mutex_init(&rmtx, NULL);
    pthread_mutex_trylock(&rmtx);
    pthread_mutex_unlock(&rmtx);

    // Initializing pmtx
    pthread_mutex_init(&pmtx, NULL);
    pthread_mutex_trylock(&pmtx);
    pthread_mutex_unlock(&pmtx);

    // Initializing cv,cmtx
    cv.resize(n);
    cmtx.resize(n);
    for (int i = 0; i < n; i++)
    {
        pthread_cond_init(&cv[i], NULL);
        pthread_mutex_init(&cmtx[i], NULL);
        pthread_mutex_trylock(&cmtx[i]);
        pthread_mutex_unlock(&cmtx[i]);
    }

    /*Init the shm*/
    shm.resize(m + 2);

    /*Finish array to track which of threads have finished*/
    vector<bool> finish(m, false);

    /*Create the Queue Data-Structure...*/
    list<pair<int, vector<int>>> request;

    /*Create Threads...*/
    pthread_t threads[n];
    for (int i = 0; i < n; i++)
    {
        int *userId = new int(i);
        pthread_create(&threads[i], NULL, user, userId);
    }

    /*wait on BOS*/
    pthread_barrier_wait(&BOS);

    /*Working Loop...*/
    while (numberOfLiveUser)
    {
        /*Joins REQB*/
        pthread_barrier_wait(&REQB);

        /*Read the Request...*/
        int userId = shm[0];
        int typeOfReq = shm[1];
        vector<int> currReq;
        for (int i = 2; i <= m + 1; i++)
        {
            currReq.push_back(shm[i]);
        }

        /*Joins ACKBi*/
        pthread_barrier_wait(&ACKB[userId]);

        pthread_mutex_lock(&pmtx);
        if (typeOfReq != QUIT)
            cout << "Master thread stores resource request of thread " << userId << endl;
        if (typeOfReq == RELEASE)
        {
            /*Update the available...*/
            for (int i = 0; i < m; i++)
            {
                AVAILABLE[i] += (-currReq[i]);
                ALLOCATION[userId][i] += (currReq[i]);
            }
        }
        else if (typeOfReq == QUIT)
        {
            /*Update the available...*/
            for (int i = 0; i < m; i++)
            {
                AVAILABLE[i] += ALLOCATION[userId][i];
                ALLOCATION[userId][i] = 0;
            }
            cout << "Master thread releases resources of thread " << userId << endl;
            numberOfLiveUser--;

            /*Mark it as finished*/
            finish[userId] = true;

            /*Print left threads...*/
            cout << numberOfLiveUser << " threads left: ";
            for (int i = 0; i < m; i++)
            {
                if (!finish[i])
                    cout << i << " ";
            }
            cout << endl;

            /*print available resources...*/
            cout << "AVAILABLE Resources: ";
            for (int i = 0; i < m; i++)
                cout << AVAILABLE[i] << " ";
            cout << endl;
        }
        else
        {
            /*ADDITIONAL*/

            /*Update the available...*/
            for (int i = 0; i < m; i++)
            {
                if (currReq[i] < 0)
                {
                    AVAILABLE[i] += (-currReq[i]);
                    ALLOCATION[userId][i] += (currReq[i]);
                    currReq[i] = 0;
                }
            }

            /*Push it at the back of the list*/
            request.push_back({userId, currReq});
        }

        /*Print the list of waiting Threads...*/
        cout << "\t\tWaiting threads : ";
        for (auto it : request)
        {
            cout << it.first << " ";
        }
        cout << endl;

        cout << "Master thread tries to grant pending requests" << endl;

        /*Possibility of serving Pending Req...*/
        for (auto it = request.begin(); it != request.end();)
        {
            bool flag = lessThan((*it).second, AVAILABLE);

            if (flag)
            {
                /*Can be served...*/
                for (int i = 0; i < m; i++)
                {
                    AVAILABLE[i] -= ((*it).second)[i];
                    ALLOCATION[(*it).first][i] += ((*it).second)[i];
                }

                // Bankers'check...
                bool bankerFlag = true;

#ifdef _DLAVOID
                bankerFlag = isSafe(NEED, ALLOCATION, AVAILABLE);
#endif

                if (bankerFlag == false)
                {
                    /*Revert back to prev-state*/
                    for (int i = 0; i < m; i++)
                    {
                        AVAILABLE[i] += ((*it).second)[i];
                        ALLOCATION[(*it).first][i] -= ((*it).second)[i];
                    }

                    cout << "\t+++ Unsafe to grant request of thread " << (*it).first << endl;
                }

                if (bankerFlag == true)
                {
                    cout << "Master thread grants resource requests for thread " << (*it).first << endl;
                    pthread_mutex_unlock(&pmtx);
                    /*Signal It...*/
                    sleep(1);
                    pthread_cond_signal(&cv[(*it).first]);
                    pthread_mutex_lock(&pmtx);
                    /*Delete this Request*/
                    it = request.erase(it);
                }
                else
                {
                    /*Next Request*/
                    ++it;
                }
            }
            else
            {
                cout << "\t+++ Insufficient resources to grant request of thread " << (*it).first << endl;
                ++it;
            }
        }

        /*Print the list of waiting Threads...*/
        cout << "\t\tWaiting threads : ";
        for (auto it : request)
        {
            cout << it.first << " ";
        }
        cout << endl;
        pthread_mutex_unlock(&pmtx);
    }

    /*Clearing the Mutex and all...*/
    pthread_barrier_destroy(&BOS);
    pthread_barrier_destroy(&REQB);
    for (int i = 0; i < n; i++)
    {
        pthread_barrier_destroy(&ACKB[i]);
    }
    pthread_mutex_destroy(&rmtx);
    pthread_mutex_destroy(&pmtx);
    for (int i = 0; i < n; i++)
    {
        pthread_cond_destroy(&cv[i]);
        pthread_mutex_destroy(&cmtx[i]);
    }

    for (int i = 0; i < n; i++)
    {
        pthread_join(threads[i], NULL);
    }
}