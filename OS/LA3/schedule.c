// -------------------------------------
// Date   : Jan 24 2025
// writer : Aditya Kumar Bharti
// Roll   : 22CS30007
// Topic  : Round Robin Scheduling Algorithm
// -------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

// Macros for the state of Process
#define ARRIVAL 1
#define IOCOMP 2
#define CPU_BURST_END 3
#define INTERRUPT 4

// Macros Related to Length
#define MAX_NUM_BURST 20
#define READY_QUEUE_LEN 200
#define NUMBEROFPROCESS 200

// Process Structure
struct proc
{
    int id;
    int arrival_time;
    int numberOfBurst;
    int curr_time;
    int ending_time;
    int turnAround_time;
    int running_time;
    int wait_time;
    int status;
    int last_burst_Number;
    int Remcurr_burst_time;
    int interrupt_time;
    int burstTimes[MAX_NUM_BURST + 1];
};

// Array of Processes
struct proc pcbArr[NUMBEROFPROCESS + 1];

// Part2: Queue Implementation

// Ready-Queue Implementaton
struct ready_queue
{
    int front;
    int back;
    int size;
    int v[READY_QUEUE_LEN + 1];
};

// Init Function
struct ready_queue init()
{
    struct ready_queue q;
    q.front = 1;
    q.back = 0;
    q.size = 0;
    return q;
}

// Front Method
int front(struct ready_queue q)
{
    return q.v[q.front];
}

// Enqueue Method
struct ready_queue enqueue(struct ready_queue q, int key)
{
    if (q.size == READY_QUEUE_LEN)
    {
        printf("Ready_queue is full\n");
        exit(0);
    }
    if (q.back == READY_QUEUE_LEN)
    {
        q.back = 1;
        q.v[q.back] = key;
    }
    else
    {
        q.v[++q.back] = key;
    }
    q.size++;
    return q;
}

// Dequeue Method
struct ready_queue dequeue(struct ready_queue q)
{
    if (q.size == 0)
    {
        printf("Ready_queue is full\n");
        exit(0);
    }
    if (q.front == READY_QUEUE_LEN)
    {
        q.front = 1;
    }
    else
    {
        q.front++;
    }
    q.size--;
    return q;
}

// Empty Method
int empty(struct ready_queue q)
{
    return q.size == 0;
}

// size Method
int size(struct ready_queue q)
{
    return q.size;
}

// Declare the Ready_Queue
struct ready_queue RD_Q;

// Part 3 : Handling the events[Discrete - event system simulation]

#define CAPACITY 200

void swap(int *a, int *b)
{
    int temp = *a;
    *a = *b;
    *b = temp;
    return;
}

// Global variable to track the size of the Heap
int EventQ_size = 0;

// Event_Queue
int H[CAPACITY + 1];

// Helper Methods for Indices
int Root()
{
    return 1;
}
int Parent(int n)
{
    return n / 2;
}
int LeftChild(int n)
{
    return 2 * n;
}
int RightChild(int n)
{
    return 2 * n + 1;
}

// Helper Methods for Node testing
int HasParent(int n)
{
    return n != Root();
}

int IsNode(int n)
{
    return n <= EventQ_size;
}

// Get Min Function
int getMin()
{
    if (EventQ_size == 0)
    {
        printf("Empty Priority Queue\n");
        exit(0);
    }
    return H[Root()];
}

// Function to shift up the node in the order

// Helper for shiftup(Return 1 if a Priority is less)
int helpUp(int a, int b)
{
    if (pcbArr[a].curr_time > pcbArr[b].curr_time)
        return 1;
    else if (pcbArr[a].curr_time < pcbArr[b].curr_time)
        return 0;
    else
    {
        if ((pcbArr[b].status == ARRIVAL || pcbArr[b].status == IOCOMP) && (pcbArr[a].status == ARRIVAL || pcbArr[a].status == IOCOMP) && (pcbArr[a].id > pcbArr[b].id))
            return 1;
        if ((pcbArr[a].status == INTERRUPT || pcbArr[a].status == CPU_BURST_END) && (pcbArr[b].status == ARRIVAL || pcbArr[b].status == IOCOMP))
            return 1;
        if (pcbArr[b].status == CPU_BURST_END && pcbArr[a].status == INTERRUPT)
            return 1;
        return 0;
    }
}

// To maintain the Heap Priority
void shiftUp(int n)
{
    while (HasParent(n) && helpUp(H[Parent(n)], H[n]))
    {
        swap(&H[Parent(n)], &H[n]);
        n = Parent(n);
    }
}

void push(int newNum)
{
    // Check if heap is full
    if (EventQ_size == CAPACITY)
    {
        printf("Priority Queue Full!\n");
        exit(0);
    }
    // Insert at the End
    H[EventQ_size + 1] = newNum;
    EventQ_size++;

    // Shift Up
    shiftUp(EventQ_size);
}

void shiftDown(int n)
{
    while (IsNode(LeftChild(n)))
    {
        int child = LeftChild(n);
        if (IsNode(RightChild(n)) && helpUp(H[LeftChild(n)], H[RightChild(n)]))
            child = RightChild(n);
        if (helpUp(H[n], H[child]))
            swap(&H[n], &H[child]);
        else
            break;

        n = child;
    }
}

void Pop()
{
    if (EventQ_size == 0)
    {
        printf("Empty priority queue!\n");
        exit(0);
    }
    H[Root()] = H[EventQ_size];
    EventQ_size--;
    shiftDown(Root());
}

int isEmpty()
{
    return EventQ_size == 0;
}

// Round-Robing Scheduling...
void roundRobin(int q, int numberofProcess)
{

    // Perform the  Initialization
    for (int i = 1; i <= numberofProcess; i++)
    {

        pcbArr[i].curr_time = pcbArr[i].arrival_time;
        pcbArr[i].status = ARRIVAL;

        // Init the curr_burst Number to zero
        pcbArr[i].last_burst_Number = 0;
        pcbArr[i].Remcurr_burst_time = pcbArr[i].burstTimes[1]; // Last CPU-Burst to be executed
        pcbArr[i].turnAround_time = 0;
        pcbArr[i].wait_time = 0;
        pcbArr[i].running_time = 0;
        pcbArr[i].interrupt_time = 0;
    }

    // Part 3 : Handling the events[Discrete - event system simulation]

    // Init the Ready - Queue
    RD_Q = init();

    // Time till which CPU is Engaged
    int cpu_time = -1;

    // Init Global Time
    int time = 0;

    // Total Ideal CPU-Time
    int ideal_time = 0;

    // Total Turn-Around time
    int totalTurnaround_time = 0;

    // Start Inserting the Processes in Event-Queue
    for (int i = 1; i <= numberofProcess; i++)
    {
        push(i);
    }

#ifdef VERBOSE
    printf("%-10d: Starting\n", 0);
#endif
    // Start Processing
    while (!isEmpty())
    {

        // Get the Process
        int curr_Proc = getMin();

        // Pop this Current Job
        Pop();

        // Set the time
        time = pcbArr[curr_Proc].curr_time;

        // Arrival after IO or for first Time
        if (pcbArr[curr_Proc].status == ARRIVAL || pcbArr[curr_Proc].status == IOCOMP)
        {
            if (time > cpu_time && cpu_time != -1)
            {
#ifdef VERBOSE
                printf("%-10d: CPU goes IDLE\n", cpu_time);
#endif
                ideal_time += (time - cpu_time);
                cpu_time = time;
            }

#ifdef VERBOSE
            printf("%-10d: Process %d Joins the Ready Queue upon %s\n", pcbArr[curr_Proc].curr_time, curr_Proc, (pcbArr[curr_Proc].status == ARRIVAL ? "Arrival" : "IO completion"));
#endif
            // If the time of Current-Event is earlier than CPU-Engaged time, It will get schedule later in the Ready-Q
            if (time < cpu_time)
            {
                pcbArr[curr_Proc].curr_time = cpu_time;
            }

            // Insert it in the RD_Q
            RD_Q = enqueue(RD_Q, curr_Proc);
        }
        else if (pcbArr[curr_Proc].last_burst_Number == pcbArr[curr_Proc].numberOfBurst && pcbArr[curr_Proc].status == CPU_BURST_END)
        {
            // Process is Completed(All set to Leave this world)
            totalTurnaround_time = pcbArr[curr_Proc].ending_time = pcbArr[curr_Proc].curr_time;
            pcbArr[curr_Proc].turnAround_time = pcbArr[curr_Proc].ending_time - pcbArr[curr_Proc].arrival_time;
            pcbArr[curr_Proc].wait_time = pcbArr[curr_Proc].turnAround_time - pcbArr[curr_Proc].running_time;

            // Print the Leaving Message
            int percentage = (int)round(((double)pcbArr[curr_Proc].turnAround_time / (double)pcbArr[curr_Proc].running_time) * 100);
            printf("%-10d: Process%5d exits. Turnaround time = %5d (%d%%), Wait time = %-5d\n", pcbArr[curr_Proc].ending_time, pcbArr[curr_Proc].id, pcbArr[curr_Proc].turnAround_time, percentage, pcbArr[curr_Proc].wait_time);
        }
        else if (pcbArr[curr_Proc].status == CPU_BURST_END)
        {
            // Complete the IO-BURST(It has to complete the IO-Burst)
            pcbArr[curr_Proc].last_burst_Number += 1;                                                                     // One For IO
            pcbArr[curr_Proc].Remcurr_burst_time = pcbArr[curr_Proc].burstTimes[pcbArr[curr_Proc].last_burst_Number + 1]; // Last CPU burst to be executed
            pcbArr[curr_Proc].running_time += pcbArr[curr_Proc].burstTimes[pcbArr[curr_Proc].last_burst_Number];
            pcbArr[curr_Proc].status = IOCOMP;
            pcbArr[curr_Proc].curr_time += pcbArr[curr_Proc].burstTimes[pcbArr[curr_Proc].last_burst_Number];

            // After CPU-Burst Joing the Event-Queue
            push(curr_Proc);
        }
        else
        {
#ifdef VERBOSE
            printf("%-10d: Process %d Joins the Ready Queue after Timeout\n", pcbArr[curr_Proc].curr_time, curr_Proc);
#endif
            // If the time of Current-Event is earlier than CPU-Engaged time, It will get schedule later in the Ready-Q
            if (time < cpu_time)
            {
                pcbArr[curr_Proc].curr_time = cpu_time;
            }

            // Interrupt(Join the RD_Q)
            RD_Q = enqueue(RD_Q, curr_Proc);
        }

        // Check the Status of the CPU(IF the Job at the front is after the cpu_Engaged Time , let him do the Execution)
        if (!empty(RD_Q) && (pcbArr[(front(RD_Q))].curr_time >= cpu_time))
        {
            // Process the Ready Jobs
            int ready_job = front(RD_Q);

            // Dq it
            RD_Q = dequeue(RD_Q);

            // Check if It will complete its IO-Burst
            if (pcbArr[ready_job].Remcurr_burst_time <= q)
            {
#ifdef VERBOSE
                printf("%-10d: Process %d is Scheduled to run for time %d\n", pcbArr[ready_job].curr_time, ready_job, pcbArr[ready_job].Remcurr_burst_time);
#endif
                // Increment the last_burst Number
                if (pcbArr[ready_job].status == ARRIVAL)
                    pcbArr[ready_job].last_burst_Number++;
                if (pcbArr[ready_job].status == IOCOMP)
                    pcbArr[ready_job].last_burst_Number++;

                // printf("%d(%d) : [%d - %d]\t\t", ready_job, pcbArr[ready_job].last_burst_Number, pcbArr[ready_job].curr_time, pcbArr[ready_job].curr_time + pcbArr[ready_job].Remcurr_burst_time);
                // CPU-Burst Complete
                pcbArr[ready_job].running_time += pcbArr[ready_job].Remcurr_burst_time;
                cpu_time = (pcbArr[ready_job].curr_time += pcbArr[ready_job].Remcurr_burst_time); // set CPU time engaged time here as well
                pcbArr[ready_job].Remcurr_burst_time = 0;

                // Set status to CPU_BURST_END
                pcbArr[ready_job].status = CPU_BURST_END;

                // Push it to the Event_Q
                push(ready_job);
            }
            else
            {
#ifdef VERBOSE
                printf("%-10d: Process %d is Scheduled to run for time %d\n", pcbArr[ready_job].curr_time, ready_job, q);
#endif
                // Increment the last_burst Number
                if (pcbArr[ready_job].status == ARRIVAL)
                    pcbArr[ready_job].last_burst_Number++;
                if (pcbArr[ready_job].status == IOCOMP)
                    pcbArr[ready_job].last_burst_Number++;
                // Run Untill Interrupt Comes
                // printf("%d(%d) : [%d - %d]\t\t", ready_job, pcbArr[ready_job].last_burst_Number, pcbArr[ready_job].curr_time, pcbArr[ready_job].curr_time + q);
                pcbArr[ready_job].running_time += q;
                cpu_time = (pcbArr[ready_job].curr_time += q); // set cpu_engaged Time here as well
                pcbArr[ready_job].Remcurr_burst_time -= q;

                // set status to Interrupt
                pcbArr[ready_job].status = INTERRUPT;
                pcbArr[ready_job].interrupt_time = 0;

                // Push it to the Event_Q
                push(ready_job);
            }
        }
    }

    printf("\n");

    // Performance Figures

    double avg_wait_time = 0;
    for (int i = 1; i <= numberofProcess; i++)
        avg_wait_time += pcbArr[i].wait_time;
    avg_wait_time /= numberofProcess;

    // Cpu utilization
    double cpu_util = 100.00 - ((double)ideal_time / (double)totalTurnaround_time) * 100;

    printf("Average wait time = %.2f\n", avg_wait_time);
    printf("Total turnaround time = %d\n", totalTurnaround_time);
    printf("Cpu Ideal time = %d\n", ideal_time);
    printf("Cpu utilization = %.2f%%\n", cpu_util);

    printf("\n");
}

// Driver Code Begins Here...
int main()
{

    // Part 1 : Read the Input File
    int numberofProcess;

    FILE *fptr;
    char *fileName = "input.txt";

    fptr = fopen(fileName, "r");
    fscanf(fptr, "%d", &numberofProcess);

    // printf("The numbr of process is %d\n",numberofProcess);
    for (int i = 1; i <= numberofProcess; i++)
    {
        // Read the IP
        int id;
        fscanf(fptr, "%d", &id);
        pcbArr[i].id = id;

        // Read the Arrival time
        int arr_time;
        fscanf(fptr, "%d", &arr_time);
        pcbArr[i].arrival_time = arr_time;
        pcbArr[i].curr_time = arr_time;
        pcbArr[i].status = ARRIVAL;

        // Read the list of CPU and IO Bursts
        int curr = 0;
        int temp;

        while (1)
        {
            // Read the Burst
            fscanf(fptr, "%d", &temp);
            if (temp == -1)
            {
                break;
            }
            else
            {
                pcbArr[i].burstTimes[++curr] = temp;
            }
        }
        pcbArr[i].numberOfBurst = curr;

        // Init the curr_burst Number to zero
        pcbArr[i].last_burst_Number = 0;
        pcbArr[i].Remcurr_burst_time = pcbArr[i].burstTimes[1]; // Last CPU-Burst to be executed
        pcbArr[i].turnAround_time = 0;
        pcbArr[i].wait_time = 0;
        pcbArr[i].running_time = 0;
        pcbArr[i].interrupt_time = 0;
    }

    // FCFS Scheduling
    printf("\n**** FCFS Scheduling ****\n");
    roundRobin(1e9, numberofProcess);

    // RR Scheduing with q=10
    printf("\n**** RR Scheduling with q = 10 ****\n");
    roundRobin(10, numberofProcess);

    // RR Scheduing with q=5
    printf("\n**** RR Scheduling with q = 5 ****\n");
    roundRobin(5, numberofProcess);

    return 0;
}