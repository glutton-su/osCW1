#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#ifdef _WIN32
    #include <windows.h>
    #include <time.h>
#else
    #include <unistd.h>
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
    int total_swap_time;
    int is_completed;
} Process;

double get_time() {
#ifdef _WIN32
    return (double)clock() / CLOCKS_PER_SEC;
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1000000.0;
#endif
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

void calculate_sjf(Process proc[], int n, int total_memory, int execution_order[]) {
    int current_time = 0;
    int available_memory = total_memory;
    int completed = 0;
    int order_count = 0;
    
    printf("\n\nScheduling Execution Log (SJF - Non-Preemptive):\n");
    printf("=================================\n");
    printf("Total Available Memory: %d MB\n\n", total_memory);
    
    while (completed < n) {
        int idx = -1;
        int min_burst = INT_MAX;
        
        // Find process with shortest burst time among arrived processes
        for (int i = 0; i < n; i++) {
            if (proc[i].arrival_time <= current_time && !proc[i].is_completed) {
                if (proc[i].burst_time < min_burst) {
                    min_burst = proc[i].burst_time;
                    idx = i;
                }
                // If burst times are equal, choose the one that arrived first
                if (proc[i].burst_time == min_burst && proc[i].arrival_time < proc[idx].arrival_time) {
                    idx = i;
                }
            }
        }
        
        if (idx == -1) {
            current_time++;
            continue;
        }
        
        // Handle swapping if needed
        if (proc[idx].memory_required > available_memory) {
            printf("Time %d: Process %s needs %d MB but only %d MB available\n", 
                   current_time, proc[idx].name, proc[idx].memory_required, available_memory);
            
            for (int j = 0; j < n; j++) {
                if (proc[j].is_completed && !proc[j].is_swapped) {
                    proc[j].is_swapped = 1;
                    available_memory += proc[j].memory_required;
                    current_time += SWAP_TIME;
                    printf("Time %d: Swapped OUT process %s (freed %d MB, available: %d MB)\n",
                           current_time, proc[j].name, proc[j].memory_required, available_memory);
                    
                    if (available_memory >= proc[idx].memory_required) break;
                }
            }
        }
        
        current_time += SWAP_TIME;
        proc[idx].total_swap_time = SWAP_TIME;
        available_memory -= proc[idx].memory_required;
        
        printf("Time %d: Swapped IN process %s (required: %d MB, available: %d MB)\n",
               current_time, proc[idx].name, proc[idx].memory_required, available_memory);
        
        proc[idx].start_time = current_time;
        proc[idx].response_time = proc[idx].start_time - proc[idx].arrival_time;
        
        printf("Time %d: Executing process %s (burst: %d units) [Shortest Job]\n",
               current_time, proc[idx].name, proc[idx].burst_time);
        
        current_time += proc[idx].burst_time;
        proc[idx].completion_time = current_time;
        proc[idx].turnaround_time = proc[idx].completion_time - proc[idx].arrival_time;
        proc[idx].waiting_time = proc[idx].turnaround_time - proc[idx].burst_time;
        proc[idx].is_completed = 1;
        
        execution_order[order_count++] = idx;
        
        printf("Time %d: Process %s completed\n\n", current_time, proc[idx].name);
        completed++;
    }
}

void print_performance_metrics(Process proc[], int n, double exec_time) {
    int total_wt = 0, total_tat = 0;
    int total_burst = 0;
    int makespan = 0;
    int total_swap_time = 0;
    int swapped_count = 0;
    
    for (int i = 0; i < n; i++) {
        total_wt += proc[i].waiting_time;
        total_tat += proc[i].turnaround_time;
        total_burst += proc[i].burst_time;
        total_swap_time += proc[i].total_swap_time;
        if (proc[i].completion_time > makespan) makespan = proc[i].completion_time;
        if (proc[i].is_swapped) swapped_count++;
    }
    
    double avg_wt = (double)total_wt / n;
    double avg_tat = (double)total_tat / n;
    double throughput = (double)n / makespan;
    double cpu_util = ((double)total_burst / makespan) * 100;
    
    printf("\n\nPerformance Metrics:\n");
    printf("=================================\n");
    printf("Average Waiting Time       : %.2f units\n", avg_wt);
    printf("Average Turnaround Time    : %.2f units\n", avg_tat);
    printf("Throughput                 : %.4f processes/unit\n", throughput);
    printf("CPU Utilization            : %.2f%%\n", cpu_util);
    printf("Total Swap Time            : %d units\n", total_swap_time);
    printf("Processes Swapped Out      : %d\n", swapped_count);
    printf("Program Execution Time     : %.6f seconds\n", exec_time);
}

void initialize_processes(Process proc[]) {
    proc[0] = (Process){1, "P1", "Batch Analytics", 0, 5, 0, 0, 0, 0, 0, 2500, 0, 0, 0};
    proc[1] = (Process){2, "P2", "Web Request (critical)", 1, 3, 0, 0, 0, 0, 0, 512, 0, 0, 0};
    proc[2] = (Process){3, "P3", "Batch Analytics (long-running)", 2, 8, 0, 0, 0, 0, 0, 3200, 0, 0, 0};
    proc[3] = (Process){4, "P4", "Web Request (short)", 1, 2, 0, 0, 0, 0, 0, 256, 0, 0, 0};
    proc[4] = (Process){5, "P5", "Database Query", 3, 4, 0, 0, 0, 0, 0, 1024, 0, 0, 0};
    proc[5] = (Process){6, "P6", "Web Request (quick)", 5, 1, 0, 0, 0, 0, 0, 128, 0, 0, 0};
    proc[6] = (Process){7, "P7", "Web Request", 4, 2, 0, 0, 0, 0, 0, 384, 0, 0, 0};
    proc[7] = (Process){8, "P8", "API Request", 4, 2, 0, 0, 0, 0, 0, 448, 0, 0, 0};
    proc[8] = (Process){9, "P9", "Report Generation", 7, 6, 0, 0, 0, 0, 0, 1800, 0, 0, 0};
    proc[9] = (Process){10, "P10", "Data Processing", 6, 5, 0, 0, 0, 0, 0, 1200, 0, 0, 0};
}

int main() {
    Process proc[MAX_PROCESSES];
    int execution_order[MAX_PROCESSES];
    double start_time, end_time;
    
    printf("========================================\n");
    printf("  SJF Scheduling Algorithm\n");
    printf("  (Shortest Job First - Non-Preemptive)\n");
    printf("  Cross-Platform (Windows/Linux)\n");
    printf("========================================\n\n");
    
    initialize_processes(proc);
    
    printf("Initialized 10 processes\n");
    printf("Total Available Memory: %d MB\n\n", TOTAL_MEMORY);
    
    start_time = get_time();
    calculate_sjf(proc, MAX_PROCESSES, TOTAL_MEMORY, execution_order);
    end_time = get_time();
    
    print_process_table(proc, MAX_PROCESSES);
    print_performance_metrics(proc, MAX_PROCESSES, end_time - start_time);
    
    printf("\n\n=================================\n");
    printf("Scenario Analysis:\n");
    printf("=================================\n");
    printf("SJF prioritizes shorter jobs, reducing average waiting\n");
    printf("time compared to FCFS. Critical short web requests are\n");
    printf("served faster, but long-running processes may starve.\n\n");
    
    return 0;
}
