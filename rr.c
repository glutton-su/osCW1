#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
#define TIME_QUANTUM 3

typedef struct {
    int pid;
    char name[20];
    char type[50];
    int arrival_time;
    int burst_time;
    int remaining_time;
    int completion_time;
    int turnaround_time;
    int waiting_time;
    int response_time;
    int start_time;
    int memory_required;
    int is_swapped;
    int total_swap_time;
    int first_execution;
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

void calculate_rr(Process proc[], int n, int total_memory, int time_quantum) {
    int current_time = 0;
    int available_memory = total_memory;
    int completed = 0;
    int queue[100], front = 0, rear = 0;
    int visited[MAX_PROCESSES] = {0};
    
    printf("\n\nScheduling Execution Log (Round Robin - TQ=%d):\n", time_quantum);
    printf("=================================\n");
    printf("Total Available Memory: %d MB\n\n", total_memory);
    
    // Add first arrived process to queue
    for (int i = 0; i < n; i++) {
        if (proc[i].arrival_time <= current_time && !visited[i]) {
            queue[rear++] = i;
            visited[i] = 1;
        }
    }
    
    while (completed < n) {
        if (front == rear) {
            current_time++;
            for (int i = 0; i < n; i++) {
                if (proc[i].arrival_time <= current_time && !visited[i]) {
                    queue[rear++] = i;
                    visited[i] = 1;
                }
            }
            continue;
        }
        
        int idx = queue[front++];
        
        // Handle swapping if needed
        if (proc[idx].remaining_time == proc[idx].burst_time) {
            if (proc[idx].memory_required > available_memory) {
                printf("Time %d: Process %s needs %d MB but only %d MB available\n", 
                       current_time, proc[idx].name, proc[idx].memory_required, available_memory);
                
                for (int j = 0; j < n; j++) {
                    if (proc[j].remaining_time == 0 && !proc[j].is_swapped) {
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
            proc[idx].total_swap_time += SWAP_TIME;
            available_memory -= proc[idx].memory_required;
            
            printf("Time %d: Swapped IN process %s (required: %d MB, available: %d MB)\n",
                   current_time, proc[idx].name, proc[idx].memory_required, available_memory);
        }
        
        if (proc[idx].first_execution == 0) {
            proc[idx].start_time = current_time;
            proc[idx].response_time = proc[idx].start_time - proc[idx].arrival_time;
            proc[idx].first_execution = 1;
        }
        
        int exec_time = (proc[idx].remaining_time < time_quantum) ? proc[idx].remaining_time : time_quantum;
        
        printf("Time %d: Executing process %s (remaining: %d, quantum: %d)\n",
               current_time, proc[idx].name, proc[idx].remaining_time, exec_time);
        
        proc[idx].remaining_time -= exec_time;
        current_time += exec_time;
        
        // Add newly arrived processes to queue
        for (int i = 0; i < n; i++) {
            if (proc[i].arrival_time <= current_time && !visited[i] && proc[i].remaining_time > 0) {
                queue[rear++] = i;
                visited[i] = 1;
            }
        }
        
        if (proc[idx].remaining_time > 0) {
            queue[rear++] = idx;
            printf("Time %d: Process %s preempted (remaining: %d)\n\n", current_time, proc[idx].name, proc[idx].remaining_time);
        } else {
            proc[idx].completion_time = current_time;
            proc[idx].turnaround_time = proc[idx].completion_time - proc[idx].arrival_time;
            proc[idx].waiting_time = proc[idx].turnaround_time - proc[idx].burst_time;
            printf("Time %d: Process %s completed\n\n", current_time, proc[idx].name);
            completed++;
        }
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
    proc[0] = (Process){1, "P1", "Batch Analytics", 0, 5, 5, 0, 0, 0, 0, 0, 2500, 0, 0, 0};
    proc[1] = (Process){2, "P2", "Web Request (critical)", 1, 3, 3, 0, 0, 0, 0, 0, 512, 0, 0, 0};
    proc[2] = (Process){3, "P3", "Batch Analytics (long-running)", 2, 8, 8, 0, 0, 0, 0, 0, 3200, 0, 0, 0};
    proc[3] = (Process){4, "P4", "Web Request (short)", 1, 2, 2, 0, 0, 0, 0, 0, 256, 0, 0, 0};
    proc[4] = (Process){5, "P5", "Database Query", 3, 4, 4, 0, 0, 0, 0, 0, 1024, 0, 0, 0};
    proc[5] = (Process){6, "P6", "Web Request (quick)", 5, 1, 1, 0, 0, 0, 0, 0, 128, 0, 0, 0};
    proc[6] = (Process){7, "P7", "Web Request", 4, 2, 2, 0, 0, 0, 0, 0, 384, 0, 0, 0};
    proc[7] = (Process){8, "P8", "API Request", 4, 2, 2, 0, 0, 0, 0, 0, 448, 0, 0, 0};
    proc[8] = (Process){9, "P9", "Report Generation", 7, 6, 6, 0, 0, 0, 0, 0, 1800, 0, 0, 0};
    proc[9] = (Process){10, "P10", "Data Processing", 6, 5, 5, 0, 0, 0, 0, 0, 1200, 0, 0, 0};
}

int main() {
    Process proc[MAX_PROCESSES];
    double start_time, end_time;
    
    printf("========================================\n");
    printf("  Round Robin Scheduling Algorithm\n");
    printf("  Time Quantum = %d units\n", TIME_QUANTUM);
    printf("  Cross-Platform (Windows/Linux)\n");
    printf("========================================\n\n");
    
    initialize_processes(proc);
    
    printf("Initialized 10 processes\n");
    printf("Total Available Memory: %d MB\n\n", TOTAL_MEMORY);
    
    start_time = get_time();
    calculate_rr(proc, MAX_PROCESSES, TOTAL_MEMORY, TIME_QUANTUM);
    end_time = get_time();
    
    print_process_table(proc, MAX_PROCESSES);
    print_performance_metrics(proc, MAX_PROCESSES, end_time - start_time);
    
    printf("\n\n=================================\n");
    printf("Scenario Analysis:\n");
    printf("=================================\n");
    printf("Round Robin provides fair CPU allocation with time quantum.\n");
    printf("All processes get equal opportunity, preventing starvation.\n");
    printf("Good for interactive systems but has context switch overhead.\n\n");
    
    return 0;
}
