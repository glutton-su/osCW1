#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Platform-specific includes and sleep function
#ifdef _WIN32
    #include <windows.h>
    #define SLEEP_MS(x) Sleep(x)
    #define CLEAR_SCREEN "cls"
#else
    #include <unistd.h>
    #include <sys/time.h>
    #define SLEEP_MS(x) usleep((x) * 1000)
    #define CLEAR_SCREEN "clear"
#endif

// For timing
#ifdef _WIN32
    #include <time.h>
#else
    #include <sys/time.h>
#endif

#define MAX_PROCESSES 10
#define SWAP_TIME 2
#define TOTAL_MEMORY 6000

typedef struct {
    int pid;
    char name[20];
    char type[50];
    int arrival_time;
    int burst_time;
    int completion_time;
    int turnaround_time;
    int waiting_time;
    int response_time;
    int start_time;
    int memory_required;
    int is_swapped;
    int swap_out_time;
    int swap_in_time;
    int total_swap_time;
} Process;

// ─────────────────────────────────────────────────────────
// Function prototypes (this fixes the compilation errors)
// ─────────────────────────────────────────────────────────
double get_time(void);
void print_gantt_chart(Process proc[], int n);
void print_process_table(Process proc[], int n);
void calculate_fcfs(Process proc[], int n, int total_memory);
void print_performance_metrics(Process proc[], int n, double exec_time);
void initialize_processes(Process proc[]);

// Cross-platform timing function
double get_time() {
#ifdef _WIN32
    return (double)clock() / CLOCKS_PER_SEC;
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1000000.0;
#endif
}

void print_gantt_chart(Process proc[], int n) {
    int i, j;
   
    printf("\n\nGantt Chart:\n");
    printf("=================================\n");
   
    // Top border
    for (i = 0; i < n; i++) {
        printf("+");
        for (j = 0; j < proc[i].burst_time; j++) printf("--");
    }
    printf("+\n");
   
    // Process names
    for (i = 0; i < n; i++) {
        printf("|");
        int padding = proc[i].burst_time * 2 - strlen(proc[i].name);
        for (j = 0; j < padding / 2; j++) printf(" ");
        printf("%s", proc[i].name);
        for (j = 0; j < (padding + 1) / 2; j++) printf(" ");
    }
    printf("|\n");
   
    // Bottom border
    for (i = 0; i < n; i++) {
        printf("+");
        for (j = 0; j < proc[i].burst_time; j++) printf("--");
    }
    printf("+\n");
   
    // Timeline
    printf("0");
    for (i = 0; i < n; i++) {
        for (j = 0; j < proc[i].burst_time * 2 - 1; j++) printf(" ");
        printf("%d", proc[i].completion_time);
    }
    printf("\n");
}

void print_process_table(Process proc[], int n) {
    printf("\n\nProcess Execution Table:\n");
    printf("=========================================================================================================================\n");
    printf("PID\tName\tType\t\t\t\tAT\tBT\tMem\tSwapped\tSwap Time\tCT\tTAT\tWT\tRT\n");
    printf("=========================================================================================================================\n");
   
    for (int i = 0; i < n; i++) {
        printf("%d\t%s\t%-30s\t%d\t%d\t%d\t%s\t%d\t\t%d\t%d\t%d\t%d\n",
               proc[i].pid, proc[i].name, proc[i].type, proc[i].arrival_time,
               proc[i].burst_time, proc[i].memory_required,
               proc[i].is_swapped ? "Yes" : "No",
               proc[i].total_swap_time, proc[i].completion_time,
               proc[i].turnaround_time, proc[i].waiting_time,
               proc[i].response_time);
    }
    printf("=========================================================================================================================\n");
}

void calculate_fcfs(Process proc[], int n, int total_memory) {
    int current_time = 0;
    int available_memory = total_memory;
    int total_swap_operations = 0;
   
    // Sort by arrival time (FCFS)
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (proc[j].arrival_time > proc[j + 1].arrival_time) {
                Process temp = proc[j];
                proc[j] = proc[j + 1];
                proc[j + 1] = temp;
            }
        }
    }
   
    printf("\n\nScheduling Execution Log:\n");
    printf("=================================\n");
    printf("Total Available Memory: %d MB\n\n", total_memory);
   
    for (int i = 0; i < n; i++) {
        // If CPU is idle, jump to arrival time
        if (current_time < proc[i].arrival_time) {
            current_time = proc[i].arrival_time;
        }
       
        // Check if memory is sufficient
        if (proc[i].memory_required > available_memory) {
            printf("Time %d: Process %s needs %d MB but only %d MB available\n",
                   current_time, proc[i].name, proc[i].memory_required, available_memory);
           
            // Find completed processes to swap out
            int memory_freed = 0;
            for (int j = 0; j < i; j++) {
                if (proc[j].completion_time > 0 && proc[j].completion_time <= current_time && !proc[j].is_swapped) {
                    proc[j].is_swapped = 1;
                    proc[j].swap_out_time = current_time;
                    memory_freed += proc[j].memory_required;
                    available_memory += proc[j].memory_required;
                    current_time += SWAP_TIME;
                    total_swap_operations++;
                   
                    printf("Time %d: Swapped OUT process %s (freed %d MB, available: %d MB)\n",
                           current_time, proc[j].name, proc[j].memory_required, available_memory);
                   
                    if (available_memory >= proc[i].memory_required) {
                        break;
                    }
                }
            }
           
            if (available_memory < proc[i].memory_required) {
                printf("WARNING: Insufficient memory even after swapping!\n");
            }
        }
       
        // Swap in the current process
        current_time += SWAP_TIME;
        proc[i].swap_in_time = current_time;
        proc[i].total_swap_time = SWAP_TIME;
        available_memory -= proc[i].memory_required;
        total_swap_operations++;
       
        printf("Time %d: Swapped IN process %s (required: %d MB, available: %d MB)\n",
               current_time, proc[i].name, proc[i].memory_required, available_memory);
       
        proc[i].start_time = current_time;
        proc[i].response_time = proc[i].start_time - proc[i].arrival_time;
       
        printf("Time %d: Executing process %s (burst: %d units)\n",
               current_time, proc[i].name, proc[i].burst_time);
       
        current_time += proc[i].burst_time;
        proc[i].completion_time = current_time;
        proc[i].turnaround_time = proc[i].completion_time - proc[i].arrival_time;
        proc[i].waiting_time = proc[i].turnaround_time - proc[i].burst_time;
       
        printf("Time %d: Process %s completed\n\n", current_time, proc[i].name);
    }
   
    printf("Total Swap Operations: %d\n", total_swap_operations);
    printf("Total Swap Overhead: %d time units\n", total_swap_operations * SWAP_TIME);
}

void print_performance_metrics(Process proc[], int n, double exec_time) {
    int total_wt = 0, total_tat = 0;
    int total_burst = 0;
    int makespan = proc[n-1].completion_time;
    int total_swap_time = 0;
    int swapped_count = 0;
   
    for (int i = 0; i < n; i++) {
        total_wt += proc[i].waiting_time;
        total_tat += proc[i].turnaround_time;
        total_burst += proc[i].burst_time;
        total_swap_time += proc[i].total_swap_time;
       
        if (proc[i].is_swapped) swapped_count++;
    }
   
    double avg_wt = (double)total_wt / n;
    double avg_tat = (double)total_tat / n;
    double throughput = (double)n / makespan;
    double cpu_util = ((double)total_burst / makespan) * 100;
   
    printf("\n\nPerformance Metrics:\n");
    printf("=================================\n");
    printf("Average Waiting Time : %.2f units\n", avg_wt);
    printf("Average Turnaround Time : %.2f units\n", avg_tat);
    printf("Throughput : %.4f processes/unit\n", throughput);
    printf("CPU Utilization : %.2f%%\n", cpu_util);
    printf("Total Swap Time : %d units\n", total_swap_time);
    printf("Processes Swapped Out : %d\n", swapped_count);
    printf("Program Execution Time : %.6f seconds\n", exec_time);
}

void initialize_processes(Process proc[]) {
    // P1
    proc[0].pid = 1;
    strcpy(proc[0].name, "P1");
    strcpy(proc[0].type, "Batch Analytics");
    proc[0].arrival_time = 0;
    proc[0].burst_time = 5;
    proc[0].memory_required = 2500;
    proc[0].is_swapped = 0;
    proc[0].total_swap_time = 0;
   
    // P2
    proc[1].pid = 2;
    strcpy(proc[1].name, "P2");
    strcpy(proc[1].type, "Web Request (critical)");
    proc[1].arrival_time = 1;
    proc[1].burst_time = 3;
    proc[1].memory_required = 512;
    proc[1].is_swapped = 0;
    proc[1].total_swap_time = 0;
   
    // P3
    proc[2].pid = 3;
    strcpy(proc[2].name, "P3");
    strcpy(proc[2].type, "Batch Analytics (long-running)");
    proc[2].arrival_time = 2;
    proc[2].burst_time = 8;
    proc[2].memory_required = 3200;
    proc[2].is_swapped = 0;
    proc[2].total_swap_time = 0;
   
    // P4
    proc[3].pid = 4;
    strcpy(proc[3].name, "P4");
    strcpy(proc[3].type, "Web Request (short)");
    proc[3].arrival_time = 1;
    proc[3].burst_time = 2;
    proc[3].memory_required = 256;
    proc[3].is_swapped = 0;
    proc[3].total_swap_time = 0;
   
    // P5
    proc[4].pid = 5;
    strcpy(proc[4].name, "P5");
    strcpy(proc[4].type, "Database Query");
    proc[4].arrival_time = 3;
    proc[4].burst_time = 4;
    proc[4].memory_required = 1024;
    proc[4].is_swapped = 0;
    proc[4].total_swap_time = 0;
   
    // P6
    proc[5].pid = 6;
    strcpy(proc[5].name, "P6");
    strcpy(proc[5].type, "Web Request (quick)");
    proc[5].arrival_time = 5;
    proc[5].burst_time = 1;
    proc[5].memory_required = 128;
    proc[5].is_swapped = 0;
    proc[5].total_swap_time = 0;
   
    // P7
    proc[6].pid = 7;
    strcpy(proc[6].name, "P7");
    strcpy(proc[6].type, "Web Request");
    proc[6].arrival_time = 4;
    proc[6].burst_time = 2;
    proc[6].memory_required = 384;
    proc[6].is_swapped = 0;
    proc[6].total_swap_time = 0;
   
    // P8
    proc[7].pid = 8;
    strcpy(proc[7].name, "P8");
    strcpy(proc[7].type, "API Request");
    proc[7].arrival_time = 4;
    proc[7].burst_time = 2;
    proc[7].memory_required = 448;
    proc[7].is_swapped = 0;
    proc[7].total_swap_time = 0;
   
    // P9
    proc[8].pid = 9;
    strcpy(proc[8].name, "P9");
    strcpy(proc[8].type, "Report Generation");
    proc[8].arrival_time = 7;
    proc[8].burst_time = 6;
    proc[8].memory_required = 1800;
    proc[8].is_swapped = 0;
    proc[8].total_swap_time = 0;
   
    // P10
    proc[9].pid = 10;
    strcpy(proc[9].name, "P10");
    strcpy(proc[9].type, "Data Processing");
    proc[9].arrival_time = 6;
    proc[9].burst_time = 5;
    proc[9].memory_required = 1200;
    proc[9].is_swapped = 0;
    proc[9].total_swap_time = 0;
}

int main() {
    Process proc[MAX_PROCESSES];
    double start_time, end_time;
   
    printf("========================================\n");
    printf(" FCFS Scheduling Algorithm\n");
    printf(" Cross-Platform (Windows/Linux)\n");
    printf(" With Hardcoded Data\n");
    printf("========================================\n\n");
   
    // Initialize processes with hardcoded data
    initialize_processes(proc);
   
    printf("Initialized 10 processes with the following data:\n");
    printf("Total Available Memory: %d MB\n\n", TOTAL_MEMORY);
   
    // Start timing
    start_time = get_time();
   
    // Calculate FCFS scheduling with swapping
    calculate_fcfs(proc, MAX_PROCESSES, TOTAL_MEMORY);
   
    // End timing
    end_time = get_time();
   
    // Display results
    print_process_table(proc, MAX_PROCESSES);
    print_gantt_chart(proc, MAX_PROCESSES);
    print_performance_metrics(proc, MAX_PROCESSES, end_time - start_time);
   
    printf("\n\n=================================\n");
    printf("Scenario Analysis:\n");
    printf("=================================\n");
    printf("This demonstrates the convoy effect in FCFS scheduling\n");
    printf("with memory swapping. Long-running batch analytics\n");
    printf("processes block short critical web requests, causing\n");
    printf("poor response times for interactive workloads.\n\n");
   
    return 0;
}
