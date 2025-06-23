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
#define USERFRAMES 12288
#define ESSENTIAL 10
#define MAX_PROCESS 500
#define INT_IN_PAGE 1024
#define NFFMIN 1000

// Statistics Metric
int currActive = 0;
int degofMp = MAX_PROCESS + 1;
int pageAccess = 0;
int pageFault = 0;
int swaps = 0;
int NFF = USERFRAMES;

// Kernel Metrics
tuple<int, int, int> FFLIST[USERFRAMES];
bool busyFlag[USERFRAMES];
bool essentialFlag[USERFRAMES];

// Page Table Data-Structure
class PageTable
{
private:
    // Page-Table Entries...
    vector<uint16_t> row;
    vector<uint16_t> his;

public:
    // Constructor
    PageTable()
    {
        for (int i = 0; i < PG_TBL_SIZE; i++)
        {
            row.push_back(0);
            his.push_back(0);
        }
    };

    // Valid page or not
    bool inMem(uint16_t k)
    {
        uint16_t val = row[k];
        uint16_t msb = val >> 15;
        return msb == 1;
    }

    // set the valid-Bit
    void setValid(uint16_t pgNo)
    {
        row[pgNo] |= (0x8000);
    }

    // set the ref-Bit
    void setRef(uint16_t pgNo)
    {
        row[pgNo] |= (0x4000);
        return;
    }

    // set the Frame Number
    void setFrame(uint16_t pgNo, uint16_t frNo)
    {
        row[pgNo] &= (0xC000);
        row[pgNo] |= (frNo);
        return;
    }

    // clear the valid-Bit
    void clearValid(uint16_t pgNo)
    {
        row[pgNo] &= (0x7FFF);
        return;
    }

    // clear the ref-Bit
    void clearRef(uint16_t pgNo)
    {
        row[pgNo] &= (0xBFFF);
        return;
    }

    // Get the ref-Bit
    uint16_t getRef(uint16_t pgNo)
    {
        return (row[pgNo] & (0x4000)) ? 1 : 0;
    }

    // Update the History
    void shiftHis(uint16_t pgNo)
    {
        int bitVal = getRef(pgNo);
        // cout << "Bitval get as " << bitVal << endl;
        his[pgNo] = (his[pgNo] >> 1) ^ (bitVal << 15);
    }

    // set to be Most-Recent
    void setToMostRecent(uint16_t pgNo)
    {
        his[pgNo] = 0XFFFF;
    }

    // Get the Corresponding Frame
    uint16_t getFrame(uint16_t pgNo)
    {
        return row[pgNo] & (0x3FFF);
    }

    // Get the victim q
    tuple<uint16_t, uint16_t, uint16_t> getVictimQ()
    {
        uint16_t ans = 0;
        uint16_t hist = 0;
        bool firstFlag = false;
        for (uint16_t i = ESSENTIAL; i < PG_TBL_SIZE; i++)
        {
            bool flag = inMem(i);
            if (flag)
            {
                if (firstFlag)
                {
                    if (his[i] < hist)
                    {
                        ans = i;
                        hist = his[i];
                    }
                }
                else
                {
                    firstFlag = true;
                    ans = i;
                    hist = his[i];
                }
            }
        }

        if (firstFlag == false)
        {
            cout << "BLUNDER" << endl;
        }

        return {ans, hist, getFrame(ans)};
    }

    // Free-Page and enqueue to freeFrame
    void updateHist()
    {
        for (int i = 0; i < PG_TBL_SIZE; i++)
        {
            bool flag = inMem(i);
            if (flag)
            {
                shiftHis(i);
                clearRef(i);
            }
        }
    }

    // clear the Page Table
    void clear()
    {
        row.resize(0);
        his.resize(0);
    }
};

class Process
{
public:
    int pid;
    int accesses;
    int faults;
    int replacements;
    int a1_attempts;
    int a2_attempts;
    int a3_attempts;
    int a4_attempts;
    int arraySize;
    int numofBinSearch;
    int currBinSearch;
    int active;
    vector<int> bin_search;
    class PageTable PT;
};

// ProcessList
vector<Process> processList;

// Get the Page Number Corresponding to entry k in A
int getPgNum(int k)
{
    return (ESSENTIAL + (k / INT_IN_PAGE));
}

// Function to get the FreeList...
int getFreeFrame()
{
    for (int i = 0; i < USERFRAMES; i++)
    {
        if (busyFlag[i] == false && essentialFlag[i] == false)
        {
            return i;
        }
    }

    return -1;
}

// add to freeFrame
void addFreeFrame(int frame, int proc, int page)
{
    FFLIST[frame] = make_tuple(frame, proc, page);
    busyFlag[frame] = false;
}

// Update the freeFrame
void updateFreeFrame(int frame, int owner, int page)
{
    FFLIST[frame] = make_tuple(frame, owner, page);
    busyFlag[frame] = true;
}

// Attempt-1
int attempt1(int proc, int page)
{
    for (int i = 0; i < USERFRAMES; i++)
    {
        if (busyFlag[i] == false && essentialFlag[i] == false)
        {
            int _frNo = get<0>(FFLIST[i]);
            int _proc = get<1>(FFLIST[i]);
            int _pgNo = get<2>(FFLIST[i]);

            if (proc == _proc && page == _pgNo)
            {
                return i;
            }
        }
    }

    return -1;
}

int attempt2(int proc, int page)
{
    for (int i = 0; i < USERFRAMES; i++)
    {
        if (busyFlag[i] == false && essentialFlag[i] == false)
        {
            int _frNo = get<0>(FFLIST[i]);
            int _proc = get<1>(FFLIST[i]);
            int _pgNo = get<2>(FFLIST[i]);

            if (_proc == -1 && _pgNo == -1)
            {
                return i;
            }
        }
    }

    return -1;
}
int attempt3(int proc, int page)
{
    for (int i = 0; i < USERFRAMES; i++)
    {
        if (busyFlag[i] == false && essentialFlag[i] == false)
        {
            int _frNo = get<0>(FFLIST[i]);
            int _proc = get<1>(FFLIST[i]);
            int _pgNo = get<2>(FFLIST[i]);

            if (proc == _proc)
            {
                return i;
            }
        }
    }

    return -1;
}
int attempt4(int proc, int page)
{

    // Randomly select FREE-FRAMES
    do
    {
        int frameNo = rand() % USERFRAMES;
        if (busyFlag[frameNo] == false && essentialFlag[frameNo] == false)
        {
            return frameNo;
        }
    } while (1);

    return -1;
}

// Handle Exiting Process
void exitProcess(int procNum)
{
    // Free all the Frames Allocated to It...
    for (int i = 0; i < USERFRAMES; i++)
    {
        uint16_t frNo = get<0>(FFLIST[i]);
        uint16_t proc = get<1>(FFLIST[i]);
        uint16_t pgNo = get<2>(FFLIST[i]);

        if (proc == procNum)
        {
            if (busyFlag[i] == true)
                NFF++;

            updateFreeFrame(i, -1, -1);
            busyFlag[i] = false;
            essentialFlag[i] = false;
        }
    }

    // clear it's Page-Table
    processList[procNum].PT.clear();
}

// Perform Binary Search
int performBinSearch(int procNum)
{
    /*Perform the Binary Search*/
    int left = 0;
    int right = processList[procNum].arraySize - 1;
    int k = processList[procNum].bin_search[processList[procNum].currBinSearch];
    while (left < right)
    {
        /*Calculate mid-value*/
        int mid = (left + right) / 2;

        // Get the Page Number to access the mid
        uint16_t pageNum = (uint16_t)getPgNum(mid);

        // Request the PageNum
        bool flag = processList[procNum].PT.inMem(pageNum);
        if (flag)
        {
            processList[procNum].PT.setRef(pageNum);
            processList[procNum].accesses++;
        }
        else
        {
            if (NFF > NFFMIN)
            {
                int freeFrameIdx = getFreeFrame();
                if (freeFrameIdx == -1)
                {
                    cout << "No free Frames Available2" << endl;
                    exit(EXIT_FAILURE);
                }

                // Update the FreeFrame
                updateFreeFrame(freeFrameIdx, procNum, pageNum);

                processList[procNum].PT.setValid(pageNum);
                processList[procNum].PT.setRef(pageNum);
                processList[procNum].PT.setFrame(pageNum, freeFrameIdx);

                // set it to most recently Used
                processList[procNum].PT.setToMostRecent(pageNum);

                processList[procNum].faults++;
                processList[procNum].accesses++;

                NFF--;

#ifdef VERBOSE
                cout << "    Fault on Page" << setw(5) << pageNum << ": Free frame " << freeFrameIdx << " found" << endl;
#endif
            }
            else
            {
                // Get the Victim-Q
                auto it = processList[procNum].PT.getVictimQ();
                uint16_t q = get<0>(it);
                uint16_t hist = get<1>(it);
                uint16_t frame = get<2>(it);

                // Mark-q  to be cleared Out
                processList[procNum].PT.clearValid(q);
                processList[procNum].PT.clearRef(q);

#ifdef VERBOSE
                cout << "    Fault on Page" << setw(5) << pageNum << ": To replace Page " << q << " at Frame " << frame << " [history = " << hist << "]" << endl;
#endif
                // Attempt-1
                int freeFrameIdx = attempt1(procNum, pageNum);
                if (freeFrameIdx == -1)
                {
                    // Attempt-2
                    freeFrameIdx = attempt2(procNum, pageNum);
                    if (freeFrameIdx == -1)
                    {
                        // Attempt-3
                        freeFrameIdx = attempt3(procNum, pageNum);
                        if (freeFrameIdx == -1)
                        {
                            // Attempt-4(has to succeed)
                            freeFrameIdx = attempt4(procNum, pageNum);
                            processList[procNum].a4_attempts++;
#ifdef VERBOSE
                            cout << "        Attempt 4: Free frame " << freeFrameIdx << " owned by Process " << get<1>(FFLIST[freeFrameIdx]) << " chosen" << endl;
#endif
                        }
                        else
                        {
                            processList[procNum].a3_attempts++;
#ifdef VERBOSE
                            cout << "        Attempt 3: Own page " << get<2>(FFLIST[freeFrameIdx]) << " found in free frame " << freeFrameIdx << endl;
#endif
                        }
                    }
                    else
                    {
                        processList[procNum].a2_attempts++;
#ifdef VERBOSE
                        cout << "        Attempt 2: Free frame " << freeFrameIdx << " owned by no process found" << endl;
#endif
                    }
                }
                else
                {
                    processList[procNum].a1_attempts++;
#ifdef VERBOSE
                    cout << "        Attempt 1: Page found in free frame " << freeFrameIdx << endl;
#endif
                }

                // Free-Frame Index can not be 1 here...
                if (freeFrameIdx == -1)
                {
                    cout << "No free Frames Available1" << endl;
                    exit(EXIT_FAILURE);
                }

                // Update the FreeFrame
                updateFreeFrame(freeFrameIdx, procNum, pageNum);

                processList[procNum].PT.setValid(pageNum);
                processList[procNum].PT.setRef(pageNum);
                processList[procNum].PT.setFrame(pageNum, freeFrameIdx);

                processList[procNum].faults++;
                processList[procNum].accesses++;
                processList[procNum].replacements++;

                // set it to most recently Used
                processList[procNum].PT.setToMostRecent(pageNum);

                // Add(g,i,q) to the FreeFrame List
                addFreeFrame(frame, procNum, q);
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

    // Shift the History
    processList[procNum].PT.updateHist();

    // Incrment Bin_serach NUmber
    processList[procNum].currBinSearch++;

    if (processList[procNum].currBinSearch >= processList[procNum].numofBinSearch)
    {
        processList[procNum].active = 0;
        exitProcess(procNum);
    }

    return right;
}

// Load to MM(Essential)
void loadMM(PageTable &pt, int i)
{
    /*Allocate ESSESNTIAL pages to Process - i*/
    for (int k = 0; k < ESSENTIAL; k++)
    {
        uint16_t pgNo = uint16_t(k);
        uint16_t frNo = uint16_t(i * ESSENTIAL + k);

        // Update the page Table  Entry
        pt.setValid(pgNo);
        pt.setRef(pgNo);
        pt.setFrame(pgNo, frNo);

        // set it to be the Most Recent
        pt.setToMostRecent(pgNo);

        FFLIST[frNo] = make_tuple(i * ESSENTIAL + k, i, k);
        essentialFlag[frNo] = true;
        busyFlag[frNo] = true;

        // Decrement NFF
        NFF--;
    }
}

// init all process
void initProcess()
{
    int n = processList.size();
    for (int i = 0; i < n; i++)
    {
        processList[i].pid = i;
        processList[i].a1_attempts = 0;
        processList[i].a2_attempts = 0;
        processList[i].a3_attempts = 0;
        processList[i].a4_attempts = 0;
        processList[i].accesses = 0;
        processList[i].faults = 0;
        processList[i].replacements = 0;
        processList[i].active = 1;
        processList[i].currBinSearch = 0;
    }

    /*Load the Processes into MM*/
    for (int i = 0; i < n; i++)
    {
        loadMM(processList[i].PT, i);
    }
}

// Init freeFrame list
void initFFLIST()
{
    for (int i = 0; i < USERFRAMES; i++)
    {
        FFLIST[i] = make_tuple(i, -1, -1);
        busyFlag[i] = false;
        essentialFlag[i] = false;
    }
}

/*Driver Code...*/
int main()
{
    // seed the Random Function
    srand(time(NULL));

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

    // Init the FREE frame List
    initFFLIST();

    // Vector of Processes
    processList.resize(n);

    // Init all Process
    initProcess();

    // Init the bin_serach Struct
    for (int i = 0; i < n; i++)
    {
        processList[i].bin_search.resize(m);
        processList[i].numofBinSearch = m;
    }

    // Process counter
    int pcnt = 0;

    while (getline(fptr, line))
    {
        // Reset the stream
        ss.str("");
        ss.clear();

        // Write to the stream
        ss << line;

        // Read the size of the Array A
        ss >> processList[pcnt].arraySize;

        // Read the process Info..
        for (int i = 0; i < m; i++)
        {
            ss >> processList[pcnt].bin_search[i];
        }

        // Increment pcnt
        pcnt++;
    }

    // Init ReadyQ
    queue<int> readyQ;
    for (int i = 0; i < n; i++)
    {
        readyQ.push(i);
    }

    // Simulation Begin
    while (!readyQ.empty())
    {
        auto it = readyQ.front();
        readyQ.pop();
        if (processList[it].active)
        {
#ifdef VERBOSE
            cout << "+++ Process " << it << ": Search " << processList[it].currBinSearch + 1 << endl;
#endif
            performBinSearch(it);
            readyQ.push(it);
        }
    }

    // Print the Metrics...
    // Print Page Access Summary
    cout << "+++ Page access summary" << endl;
    cout << setw(5) << "PID" << setw(12) << "Accesses" << setw(16) << "Faults" << setw(23) << "Replacements" << setw(40) << "Attempts" << endl;

    for (const auto &p : processList)
    {
        double faultRate = (p.accesses > 0) ? (100.0 * p.faults / p.accesses) : 0.0;
        double replacementRate = (p.accesses > 0) ? (100.0 * p.replacements / p.accesses) : 0.0;

        cout << setw(5) << p.pid
             << setw(12) << p.accesses
             << setw(9) << p.faults << "   (" << fixed << setprecision(2) << faultRate << "%)"
             << setw(9) << p.replacements << "   (" << replacementRate << "%)"
             << setw(9) << p.a1_attempts << " +"
             << setw(5) << p.a2_attempts << " +"
             << setw(5) << p.a3_attempts << " +"
             << setw(5) << p.a4_attempts << "  ("
             << fixed << setprecision(2)
             << (p.accesses > 0 ? 100.0 * p.a1_attempts / p.replacements : 0.0) << "% + "
             << (p.accesses > 0 ? 100.0 * p.a2_attempts / p.replacements : 0.0) << "% + "
             << (p.accesses > 0 ? 100.0 * p.a3_attempts / p.replacements : 0.0) << "% + "
             << (p.accesses > 0 ? 100.0 * p.a4_attempts / p.replacements : 0.0) << "%)" << endl;
    }

    // Calculate totals
    int totalAccesses = 0;
    int totalFaults = 0;
    int totalReplacements = 0;
    int totalA1 = 0;
    int totalA2 = 0;
    int totalA3 = 0;
    int totalA4 = 0;

    for (const auto &p : processList)
    {
        totalAccesses += p.accesses;
        totalFaults += p.faults;
        totalReplacements += p.replacements;
        totalA1 += p.a1_attempts;
        totalA2 += p.a2_attempts;
        totalA3 += p.a3_attempts;
        totalA4 += p.a4_attempts;
    }

    double totalFaultRate = (totalAccesses > 0) ? (100.0 * totalFaults / totalAccesses) : 0.0;
    double totalReplacementRate = (totalAccesses > 0) ? (100.0 * totalReplacements / totalAccesses) : 0.0;

    // Print total line
    cout << endl;
    cout << setw(5) << "Total"
         << setw(12) << totalAccesses
         << setw(9) << totalFaults << "   (" << fixed << setprecision(2) << totalFaultRate << "%)"
         << setw(9) << totalReplacements << "   (" << totalReplacementRate << "%)"
         << setw(9) << totalA1 << " +"
         << setw(5) << totalA2 << " +"
         << setw(5) << totalA3 << " +"
         << setw(5) << totalA4 << "  ("
         << fixed << setprecision(2)
         << (totalReplacements > 0 ? 100.0 * totalA1 / totalReplacements : 0.0) << "% + "
         << (totalReplacements > 0 ? 100.0 * totalA2 / totalReplacements : 0.0) << "% + "
         << (totalReplacements > 0 ? 100.0 * totalA3 / totalReplacements : 0.0) << "% + "
         << (totalReplacements > 0 ? 100.0 * totalA4 / totalReplacements : 0.0) << "%)" << endl;
}