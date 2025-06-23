/*
Author : Aditya Kumar Bharti
Roll   : 22CS30007
*/

// Necessary Headers...
#include <iostream>
#include <fstream>
#include <sstream>
#include <queue>
#include <vector>
#include <cstdint>
#include <tuple>
#include <iomanip>

using namespace std;

// Macros
#define PG_TBL_SIZE 2048
#define OSPAGE 4096
#define USERPAGE 12288
#define ESSENTIAL 10
#define MAX_PROCESS 500
#define INT_IN_PAGE 1024

// Statistics Metric
int currActive = 0;
int degofMp = MAX_PROCESS + 1;
int pageAccess = 0;
int pageFault = 0;
int swaps = 0;

// Page Table Data-Structure
class PageTable
{
private:
    // Page-Table Entries...
    vector<pair<uint16_t, uint16_t>> row;

public:
    // Constructor
    PageTable()
    {
        for (int i = 0; i < PG_TBL_SIZE; i++)
        {
            row.push_back({0, 0});
        }
    };

    // Valid page or not
    bool inMem(uint16_t k)
    {
        uint16_t val = row[k].second;
        uint16_t msb = val >> 15;
        return msb == 1;
    }

    // set the Page Number
    void set(uint16_t pgNo, uint16_t frNo)
    {
        row[pgNo].second = (frNo) ^ (1 << 15);
        return;
    }

    // Get the Corresponding Frame
    uint16_t get(uint16_t pgNo)
    {
        return row[pgNo].second ^ (1 << 15);
    }

    // Free-Page and enqueue to freeFrame
    void freePages(queue<int> &q)
    {
        for (int i = 0; i < PG_TBL_SIZE; i++)
        {
            uint16_t val = row[i].second;
            uint16_t msb = val >> 15;

            if (msb == 1)
            {
                /*Valid*/
                q.push(val ^ (1 << 15));
            }

            // Mark Page as  Invalid ...
            row[i].second = 0;
        }
    }
};

// Get the Page Number Corresponding to entry k in A
int getPgNum(int k)
{
    return (ESSENTIAL + (k / INT_IN_PAGE));
}

// Perform Binary Search
int binSearch(int left, int right, int s, int k, int prNum, int srchNum, PageTable &ds, queue<int> &freeFrame, queue<pair<int, int>> &swappedOut)
{
    /*Perform the Binary Search*/
    while (left < right)
    {
        /*Calculate mid-value*/
        int mid = (left + right) / 2;

        // Get the Page Number to access the mid
        uint16_t pageNum = (uint16_t)getPgNum(mid);

        // Request the PageNum
        ++pageAccess; // Increment Page Num
        bool flag = ds.inMem(pageNum);

        if (!flag)
        {
            pageFault++; // Incrment Page Fault

            // Allocate the Frame..
            if (freeFrame.size() >= 1)
            {
                uint16_t avFrame = (uint16_t)freeFrame.front(); // Available frame
                freeFrame.pop();
                ds.set(pageNum, avFrame); // put the entry in pageTable
            }
            else
            {
                /*Return -1*/
                ds.freePages(freeFrame); // Free the pageTable
                currActive--;
                degofMp = min(degofMp, currActive); // Update the degofMp
                cout << "+++ Swapping out process " << setw(4) << prNum << "  [" << currActive << " active processes]" << endl;
                swaps++; // Increment swaps
                swappedOut.push({prNum, srchNum});
                return -1;
            }
        }

        // Binary search update
        if (k <= mid)
        {
            right = mid;
        }
        else
        {
            left = mid + 1;
        }
    }

    return right;
}

// Load a process into MM
void load(PageTable &ds, queue<int> &freeFrame)
{
    // If less than ESSENTIAL frames are there..
    if (freeFrame.size() < ESSENTIAL)
    {
        cout << "Couldn't Load a Swapped Out Process" << endl;
        return;
    }

    // Allocate the frames..
    for (uint16_t i = 0; i < ESSENTIAL; i++)
    {
        uint16_t frame = freeFrame.front();
        freeFrame.pop();
        ds.set(i, frame);
    }
}

/*Driver Code...*/
int main()
{
    // Number of processes and number of Binary search...
    int n, m;

    // Input file...
    ifstream fptr("search.txt");

    // String
    string line;
    getline(fptr, line);

    // String stream
    stringstream ss(line);

    // Read n, m
    ss >> n >> m;

    // Process Info...
    vector<vector<int>> proc(n, vector<int>(m + 1));

    // Process counter
    int pcnt = 0;

    while (getline(fptr, line))
    {
        // Reset the stream
        ss.str("");
        ss.clear();

        // Write to the stream
        ss << line;

        // Read the process Info..
        for (int i = 0; i <= m; i++)
        {
            ss >> proc[pcnt][i];
        }

        // Increment pcnt
        pcnt++;
    }

    cout << "+++ Simulation data read from file" << endl;

    // vector of PageTable for everyProcess
    vector<PageTable> _pageTable(n);

    // Kernel Lists...
    queue<tuple<int, int, bool>> readyQ;
    queue<int> freeFrame;
    queue<pair<int, int>> swappedOut;

    // Fill in the Lists...
    for (int i = 0; i < n; i++)
    {
        readyQ.push(make_tuple(i, 1, false)); // EveryOne Ready to make their First Bin-Search
    }

    // freeFrame
    for (int i = 0; i < USERPAGE; i++)
    {
        freeFrame.push(OSPAGE + i);
    }

    cout << "+++ Kernel data initialized" << endl;

    // Simulation Begin
    while (!readyQ.empty())
    {
        // Get the Process
        auto it = readyQ.front();
        readyQ.pop();

        // Process It...
        int processNum = get<0>(it);
        int searchNum = get<1>(it);
        bool loadFlag = get<2>(it);

        if (loadFlag == false)
        {
            /*Try to load the Process*/
            load(_pageTable[processNum], freeFrame);
            currActive++;
            loadFlag = true;
        }

        // Perform the curr-bin search
        int s = proc[processNum][0];
        int k = proc[processNum][searchNum];

        // Simulate Binary search
#ifdef VERBOSE
        cout << "\tSearch " << searchNum << " by Process " << processNum << endl;
#endif
        int idx = binSearch(0, s - 1, s, k, processNum, searchNum, _pageTable[processNum], freeFrame, swappedOut);

        /*SuccessFul Binary search*/
        if (idx != -1)
        {
            /*Init a flag to track if this process should be pushed to ReadyQ or not...*/
            bool flag = true;

            /*Breaks if process is not to exit*/
            while (searchNum >= m)
            {
                /*Process Exits*/
                _pageTable[processNum].freePages(freeFrame);
                currActive--;

                /*Look for swappedOut Queue*/
                if (swappedOut.size() >= 1)
                {
                    /*Should Always Work...*/
                    processNum = swappedOut.front().first;
                    searchNum = swappedOut.front().second;

                    /*Try to load in MEM*/
                    load(_pageTable[processNum], freeFrame);
                    currActive++;
                    loadFlag = true;

                    cout << "+++ Swapping in process " << setw(5) << processNum << "  [" << currActive << " active processes]" << endl;

                    swappedOut.pop();
#ifdef VERBOSE
                    cout << "\tSearch " << searchNum << " by Process " << processNum << endl;
#endif
                    /*Perform Binary search*/
                    int newIdx = binSearch(0, proc[processNum][0] - 1, proc[processNum][0], proc[processNum][searchNum], processNum, searchNum, _pageTable[processNum], freeFrame, swappedOut);

                    if (newIdx == -1)
                    {
                        /*Flag set to false as process is in swappedQ*/
                        flag = false;
                        break;
                    }
                }
                else
                {
                    /*Break as no swapped entry process*/
                    break;
                }
            }

            /*Binary Search SuccessFull*/
            if ((searchNum < m) && flag)
            {
                readyQ.push(make_tuple(processNum, searchNum + 1, loadFlag));
            }
        }
    }

    // Swapped Out queue
    while (!swappedOut.empty())
    {
        /*ProcessNum and it's search Num*/
        int processNum = swappedOut.front().first;
        int searchNum = swappedOut.front().second;

        /*Try to load in MEM*/
        load(_pageTable[processNum], freeFrame);

        swappedOut.pop();

#ifdef VERBOSE
        cout << "\tSearch " << searchNum << " by Process " << processNum << endl;
#endif
        /*Simulate Binary Search*/
        int newIdx = binSearch(0, proc[processNum][0] - 1, proc[processNum][0], proc[processNum][searchNum], processNum, searchNum, _pageTable[processNum], freeFrame, swappedOut);

        /*Something Bad has Happenned...*/
        if (newIdx == -1)
        {
            cout << "Something Went Wrong!!" << endl;
            break;
        }
    }

    // Print the Metrics...
    cout << "+++ Page access summary" << endl;
    cout << "\tTotal number of page accesses  =  " << pageAccess << endl;
    cout << "\tTotal number of page faults    =  " << pageFault << endl;
    cout << "\tTotal number of swaps          =  " << swaps << endl;
    cout << "\tDegree of multiprogramming     =  " << degofMp << endl;
}