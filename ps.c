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
    int priority;
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
    int i;
    printf("\n\nProcess Execution Table:\n");
    printf("========================================================================================================\n");
    printf("PID  Name  Type                           AT  BT  Pri  Mem   Swap?  SwapT  CT   TAT  WT   RT\n");
    printf("========================================================================================================\n");
    
    for (i = 0; i < n; i++) {
        printf("%-4d %-5s %-30s %-3d %-3d %-4d %-5d %-6s %-6d %-4d %-4d %-4d %-4d\n",
               proc[i].pid, proc[i].name, proc[i].type, proc[i].arrival_time,
               proc[i].burst_time, proc[i].priority, proc[i].memory_required,
               proc[i].is_swapped ? "Yes" : "No",
               proc[i].total_swap_time, proc[i].completion_time,
               proc[i].turnaround_time, proc[i].waiting_time,
               proc[i].response_time);
    }
    printf("========================================================================================================\n");
}

void calculate_priority(Process proc[], int n, int total_memory) {
    int current_time = 0;
    int available_memory = total_memory;
    int completed = 0;
    int i, j, idx;
    int highest_priority;
    
    printf("\n\nScheduling Execution Log (Priority - Non-Preemptive):\n");
    printf("========================================\n");
    printf("Total Available Memory: %d MB\n", total_memory);
    printf("Priority: Lower number = Higher priority\n\n");
    
    while (completed < n) {
        idx = -1;
        highest_priority = INT_MAX;
        
        /* Find highest priority process among arrived processes */
        for (i = 0; i < n; i++) {
            if (proc[i].arrival_time <= current_time && !proc[i].is_completed) {
                if (proc[i].priority < highest_priority) {
                    highest_priority = proc[i].priority;
                    idx = i;
                }
                /* If priorities are equal, choose shortest burst time */
                else if (proc[i].priority == highest_priority && idx != -1) {
                    if (proc[i].burst_time < proc[idx].burst_time) {
                        idx = i;
                    }
                }
            }
        }
        
        if (idx == -1) {
            current_time++;
            continue;
        }
        
        /* Handle swapping if needed */
        if (proc[idx].memory_required > available_memory) {
            printf("Time %d: Process %s needs %d MB but only %d MB available\n", 
                   current_time, proc[idx].name, proc[idx].memory_required, available_memory);
            
            for (j = 0; j < n; j++) {
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
        
        printf("Time %d: Executing process %s (burst: %d, priority: %d) [Highest Priority]\n",
               current_time, proc[idx].name, proc[idx].burst_time, proc[idx].priority);
        
        current_time += proc[idx].burst_time;
        proc[idx].completion_time = current_time;
        proc[idx].turnaround_time = proc[idx].completion_time - proc[idx].arrival_time;
        proc[idx].waiting_time = proc[idx].turnaround_time - proc[idx].burst_time;
        proc[idx].is_completed = 1;
        
        printf("Time %d: Process %s completed\n\n", current_time, proc[idx].name);
        completed++;
    }
}

void print_performance_metrics(Process proc[], int n, double exec_time) {
    int i;
    int total_wt = 0, total_tat = 0, total_rt = 0;
    int total_burst = 0;
    int makespan = 0;
    int total_swap_time = 0;
    int swapped_count = 0;
    int context_switches = n - 1;
    
    for (i = 0; i < n; i++) {
        total_wt += proc[i].waiting_time;
        total_tat += proc[i].turnaround_time;
        total_rt += proc[i].response_time;
        total_burst += proc[i].burst_time;
        total_swap_time += proc[i].total_swap_time;
        if (proc[i].completion_time > makespan) makespan = proc[i].completion_time;
        if (proc[i].is_swapped) swapped_count++;
    }
    
    double avg_wt = (double)total_wt / n;
    double avg_tat = (double)total_tat / n;
    double avg_rt = (double)total_rt / n;
    double throughput = (double)n / makespan;
    double cpu_util = ((double)total_burst / makespan) * 100;
    
    printf("\n\nPerformance Metrics:\n");
    printf("========================================\n");
    printf("Average Waiting Time       : %.2f units\n", avg_wt);
    printf("Average Turnaround Time    : %.2f units\n", avg_tat);
    printf("Average Response Time      : %.2f units\n", avg_rt);
    printf("Throughput                 : %.4f processes/unit\n", throughput);
    printf("CPU Utilization            : %.2f%%\n", cpu_util);
    printf("Context Switches           : %d\n", context_switches);
    printf("Total Execution Time       : %d units\n", makespan);
    printf("Total Swap Time            : %d units\n", total_swap_time);
    printf("Processes Swapped Out      : %d\n", swapped_count);
    printf("Program Execution Time     : %.6f seconds\n", exec_time);
    printf("========================================\n");
}

void initialize_processes(Process proc[]) {
    /* Priority assignments: Critical web requests = 1, Short tasks = 2, Database = 3, Batch = 4-5 */
    proc[0].pid = 1; strcpy(proc[0].name, "P1"); strcpy(proc[0].type, "Batch Analytics");
    proc[0].arrival_time = 0; proc[0].burst_time = 5; proc[0].priority = 5;
    proc[0].memory_required = 2500; proc[0].is_swapped = 0; proc[0].total_swap_time = 0; proc[0].is_completed = 0;
    
    proc[1].pid = 2; strcpy(proc[1].name, "P2"); strcpy(proc[1].type, "Web Request (critical)");
    proc[1].arrival_time = 1; proc[1].burst_time = 3; proc[1].priority = 1;
    proc[1].memory_required = 512; proc[1].is_swapped = 0; proc[1].total_swap_time = 0; proc[1].is_completed = 0;
    
    proc[2].pid = 3; strcpy(proc[2].name, "P3"); strcpy(proc[2].type, "Batch Analytics (long)");
    proc[2].arrival_time = 2; proc[2].burst_time = 8; proc[2].priority = 5;
    proc[2].memory_required = 3200; proc[2].is_swapped = 0; proc[2].total_swap_time = 0; proc[2].is_completed = 0;
    
    proc[3].pid = 4; strcpy(proc[3].name, "P4"); strcpy(proc[3].type, "Web Request (short)");
    proc[3].arrival_time = 1; proc[3].burst_time = 2; proc[3].priority = 1;
    proc[3].memory_required = 256; proc[3].is_swapped = 0; proc[3].total_swap_time = 0; proc[3].is_completed = 0;
    
    proc[4].pid = 5; strcpy(proc[4].name, "P5"); strcpy(proc[4].type, "Database Query");
    proc[4].arrival_time = 3; proc[4].burst_time = 4; proc[4].priority = 3;
    proc[4].memory_required = 1024; proc[4].is_swapped = 0; proc[4].total_swap_time = 0; proc[4].is_completed = 0;
    
    proc[5].pid = 6; strcpy(proc[5].name, "P6"); strcpy(proc[5].type, "Web Request (quick)");
    proc[5].arrival_time = 5; proc[5].burst_time = 1; proc[5].priority = 1;
    proc[5].memory_required = 128; proc[5].is_swapped = 0; proc[5].total_swap_time = 0; proc[5].is_completed = 0;
    
    proc[6].pid = 7; strcpy(proc[6].name, "P7"); strcpy(proc[6].type, "Web Request");
    proc[6].arrival_time = 4; proc[6].burst_time = 2; proc[6].priority = 2;
    proc[6].memory_required = 384; proc[6].is_swapped = 0; proc[6].total_swap_time = 0; proc[6].is_completed = 0;
    
    proc[7].pid = 8; strcpy(proc[7].name, "P8"); strcpy(proc[7].type, "API Request");
    proc[7].arrival_time = 4; proc[7].burst_time = 2; proc[7].priority = 2;
    proc[7].memory_required = 448; proc[7].is_swapped = 0; proc[7].total_swap_time = 0; proc[7].is_completed = 0;
    
    proc[8].pid = 9; strcpy(proc[8].name, "P9"); strcpy(proc[8].type, "Report Generation");
    proc[8].arrival_time = 7; proc[8].burst_time = 6; proc[8].priority = 4;
    proc[8].memory_required = 1800; proc[8].is_swapped = 0; proc[8].total_swap_time = 0; proc[8].is_completed = 0;
    
    proc[9].pid = 10; strcpy(proc[9].name, "P10"); strcpy(proc[9].type, "Data Processing");
    proc[9].arrival_time = 6; proc[9].burst_time = 5; proc[9].priority = 3;
    proc[9].memory_required = 1200; proc[9].is_swapped = 0; proc[9].total_swap_time = 0; proc[9].is_completed = 0;
}

int main() {
    Process proc[MAX_PROCESSES];
    double start_time, end_time;
    
    printf("========================================\n");
    printf("  Priority Scheduling Algorithm\n");
    printf("  (Non-Preemptive)\n");
    printf("  Cross-Platform (Windows/Linux)\n");
    printf("========================================\n\n");
    
    initialize_processes(proc);
    
    printf("Initialized %d processes\n", MAX_PROCESSES);
    printf("Total Available Memory: %d MB\n\n", TOTAL_MEMORY);
    
    start_time = get_time();
    calculate_priority(proc, MAX_PROCESSES, TOTAL_MEMORY);
    end_time = get_time();
    
    print_process_table(proc, MAX_PROCESSES);
    print_performance_metrics(proc, MAX_PROCESSES, end_time - start_time);
    
    printf("\n\nScenario Analysis:\n");
    printf("========================================\n");
    printf("Priority scheduling executes critical web requests first,\n");
    printf("improving response time for high-priority tasks. However,\n");
    printf("low-priority batch jobs may experience starvation.\n\n");
    printf("Priority Assignment:\n");
    printf("  1 = Critical Web Requests (Highest)\n");
    printf("  2 = Regular Web/API Requests\n");
    printf("  3 = Database Queries\n");
    printf("  4 = Report Generation\n");
    printf("  5 = Batch Analytics (Lowest)\n");
    printf("========================================\n\n");
    
    return 0;
}
