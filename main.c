// include necessary libraries and initialize and declera necessary variables
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CONTEXT_SWITCH 10
#define SILVER 80
#define GOLD_PLATINUM 120
#define CONTEXT_SWITCH_TIME 10
#define MAX_PROCESSES 10
#define MAX_INSTRUCTIONS 100
int current_priority = -1;
int count = 0;
int time = 0;
int upd_time = 0;
int completed_processes;
int last_executed_process_index = -1;
int instruction_count;
// creation of a struct for processes
struct Process {
    char name[10];
    int priority;
    int arrival_time;
    char process_type[10];
    int remaining_quantum;
    int execution_time;
    int is_platinum;
    int burst_time;
    int initial_burst_time;
    int completion_time;
    int num_quantums_executed;
    int temp_arrival_time;
};

// creation of instruction struct
struct Instruction {
    char name[20];
    int time;
};

// comparison between processes
int compare_processes(const void* a, const void* b) {
    const struct Process* processA = (const struct Process*)a;
    const struct Process* processB = (const struct Process*)b;
    // comparison between priority of processes I take it in reverse order since my code execution works in reverse order for priorities
	if (processA->priority != processB->priority) {
        return processA->priority - processB->priority;
    } 
    // if they have same priority then check arrival time
    else if (processA->arrival_time != processB->arrival_time) {
        return processA->arrival_time - processB->arrival_time;
    }  
    // if they have same arrival time, then check name of the processes
    else {
        return strcmp(processA->name, processB->name);
    }
}




// this is for calculating the burst time to use at the end of the main
int calculate_burst_time(const char* process_name, struct Instruction instructions[], int instruction_count) {
    char file_name[30];
    sprintf(file_name, "%s.txt", process_name);
    FILE* process_file = fopen(file_name, "r");

    if (process_file == NULL) {
        perror("Error opening file");
        return -1;
    }
    int burst_time = 0;
    char instruction_name[20];
    // for each process read instructions and take their values and add them 
    while (fscanf(process_file, "%s", instruction_name) == 1) {
        for (int i = 0; i < instruction_count; i++) {
            if (strcmp(instruction_name, instructions[i].name) == 0) {
                burst_time += instructions[i].time;
                break;
            }
        }
    }
    fclose(process_file);
    return burst_time;
}


// decleration of instructions structure
struct Instruction instructions[MAX_INSTRUCTIONS];
// to execute platinum processes when they come
int firstPlatinum(struct Process processes[], int num_processes, int time, int stime) {
    int last_executed_instruction_index[MAX_PROCESSES] = {0};
    int quantum;
    for (int i = 0; i < num_processes; i++) {
        if (processes[i].arrival_time >= stime && processes[i].arrival_time <= time && processes[i].burst_time > 0) {// Check if the process has arrived until the given time
            if (strcmp(processes[i].process_type, "PLATINUM") == 0) {
                // I add context switches at the beginning of the each process so before starting the platinum process I add the context switch
                time += CONTEXT_SWITCH_TIME;
                // If the process is platinum read instructions and execute them one by one
                char process_instruction_file[30];
                sprintf(process_instruction_file, "%s.txt", processes[i].name);
                FILE* process_file = fopen(process_instruction_file, "r");
                char instruction_name[20];
                int execution_time = 0;

                for (int j = 0; j < last_executed_instruction_index[i]; j++) {
                    fscanf(process_file, "%s", instruction_name);
                }

                while (fscanf(process_file, "%s", instruction_name) == 1 && execution_time < processes[i].burst_time) {
                    for (int j = 0; j < instruction_count; j++) {
                    // comparing instruction names if they are equal check you can execute it or not
                        if (strcmp(instruction_name, instructions[j].name) == 0) {
                            int instr_time = instructions[j].time;
                            if (instr_time <= quantum - execution_time) {
                                execution_time += instr_time;
                            } else {
                                execution_time = quantum;
                            }                         
                            // decreasing the remaining quantum and burst time
                            processes[i].remaining_quantum -= instr_time;
                            processes[i].burst_time -= instr_time;
                            // increasing the time
                            time += instr_time;
                            last_executed_instruction_index[i]++;
                        }
                    }
                }
                fclose(process_file);
                // updating completed process number
                completed_processes++;
                processes[i].completion_time = time;
                // adding context switch since 1 platinum process done
                time += CONTEXT_SWITCH_TIME; // Update time for context switch
                return time;
            }
            
        }
    }
    return time;
}

// start the round robin
void priorityRoundRobinScheduling(struct Process processes[], int num_processes, struct Instruction instructions[], int instruction_count) {
    completed_processes = 0;
    int last_executed_instruction_index[MAX_PROCESSES] = {0}; // keep last executed instruction
    
    // when all processes complete stop
    while (completed_processes < num_processes) {
    	// loop as number of processes
        for (int i = 0; i < num_processes; ++i) {
        	// control that the process should be executed or wait
            if (processes[i].arrival_time <= time && processes[i].burst_time > 0 && processes[i].priority == current_priority) {
                int quantum;
                // depend on the type of the process update its quantum 
                if (processes[i].is_platinum) {
                    quantum = processes[i].burst_time;
                } else {
                    quantum = (strcmp(processes[i].process_type, "SILVER") == 0) ? SILVER : GOLD_PLATINUM;
                }

                if ((i != last_executed_process_index && last_executed_process_index != -1) || time == 0) {
                    processes[i].num_quantums_executed++;
                    // initial context switch or make context switch when you change the process
                    time += CONTEXT_SWITCH_TIME; 
                }
                
                
				
                last_executed_process_index = i;
                // reading instructions from file and execution
                char process_instruction_file[30];
                sprintf(process_instruction_file, "%s.txt", processes[i].name);
                FILE* process_file = fopen(process_instruction_file, "r");

                char instruction_name[20];
                int execution_time = 0;

                for (int j = 0; j < last_executed_instruction_index[i]; j++) {
                    fscanf(process_file, "%s", instruction_name);
                }

                while (fscanf(process_file, "%s", instruction_name) == 1 && execution_time < quantum) {
                    for (int j = 0; j < instruction_count; j++) {
                        if (strcmp(instruction_name, instructions[j].name) == 0) {
                            int instr_time = instructions[j].time;
                            if (instr_time <= quantum - execution_time) {
                                execution_time += instr_time;
                            } else {
                                execution_time = quantum;
                            }
                            
                            processes[i].remaining_quantum -= instr_time;
                            processes[i].burst_time -= instr_time;
                            processes[i].execution_time += instr_time;
                            upd_time = time;
                            time += instr_time;
                            last_executed_instruction_index[i]++;

                            // Check for exit instruction
                            if (strcmp(instruction_name, "exit") == 0) {
                                // Process has reached exit instruction update its completion time                                                           
                                processes[i].completion_time = time;
                                ++completed_processes; // increase the number of completed processes
                            }

                            // Check for priority changes
                            if (strcmp(processes[i].process_type, "SILVER") == 0 && processes[i].num_quantums_executed == 3) { // if silver process reaches 3 quantum number then change its type as gold
                                strcpy(processes[i].process_type, "GOLD");                             
                            } else if (strcmp(processes[i].process_type, "GOLD") == 0 && processes[i].num_quantums_executed == 5) { // if gold process reaches 5 quantum number then change its type as platinum
                                strcpy(processes[i].process_type, "PLATINUM");
                            }
							
							// check whether there exist any platinum or not until that time
                            time = firstPlatinum(processes, num_processes, time, upd_time);
                            break;
                        }
                    }
                }

                fclose(process_file);
                
            }
        }
	
		
        if (completed_processes < num_processes) {
            int last_process_index = 0;
            for (int i = 0; i < num_processes; ++i) {
                if (processes[i].burst_time > 0) {
                    last_process_index = i;
                    break;
                }
            }
			
			// this is for time jumping
            if (time < processes[last_process_index].arrival_time) {
                time = processes[last_process_index].arrival_time;
            }

            for (int i = 0; i < num_processes; ++i) {
                if (processes[i].arrival_time <= time && processes[i].burst_time > 0) {                  
                    current_priority = processes[i].priority; // Update current priority since my priority and round robin depends on this
                }
            }
        }
    }
}


int main() {
	// creating processes struct
    struct Process processes[MAX_PROCESSES];
    int process_count = 0;

    FILE* file = fopen("definition.txt", "r");

    if (file == NULL) {
        perror("Error opening file");
        return 1;
    }

    while (process_count < MAX_PROCESSES && fscanf(file, "%s %d %d %s", processes[process_count].name, &processes[process_count].priority,
            &processes[process_count].arrival_time, processes[process_count].process_type) == 4) {
		
        
        processes[process_count].execution_time = 0;

        // determining whether the process is platinum or not
        if (strcmp(processes[process_count].process_type, "PLATINUM") != 0) {
            processes[process_count].is_platinum = 0;
        } else {
            processes[process_count].is_platinum = 1;
        }

        process_count++;
    }

    fclose(file);
	
	// sort processes structure with respect to compare_processes function
    qsort(processes, process_count, sizeof(struct Process), compare_processes);
    
    instruction_count = 0;
    FILE* instructions_file = fopen("instructions.txt", "r");
    if (instructions_file == NULL) {
        perror("Error opening file");
        return 1;
    }
    for (int i = 0; i < process_count; i++) {
        processes[i].burst_time = calculate_burst_time(processes[i].name, instructions, instruction_count);
        if (processes[i].burst_time < 0) {
            return 1;
        }
		// keeping burst time as initial_burst time since I decrease burst time inside my code	    
        processes[i].initial_burst_time = processes[i].burst_time;
        // keeping the executed quantum number
        processes[i].num_quantums_executed = 0;
    }

	// calculating the instruction count
    while (instruction_count < MAX_INSTRUCTIONS && fscanf(instructions_file, "%s %d", instructions[instruction_count].name, &instructions[instruction_count].time) == 2) {
        instruction_count++;
    }

    fclose(instructions_file);

    for (int i = 0; i < process_count; i++) {
        processes[i].burst_time = calculate_burst_time(processes[i].name, instructions, instruction_count);
        // it is done with that process
        if (processes[i].burst_time < 0) {
            return 1;
        }
        // keeping burst time as initial_burst time since I decrease burst time inside my code
        processes[i].initial_burst_time = processes[i].burst_time;
    }

    // calling round robin
    priorityRoundRobinScheduling(processes, process_count, instructions, instruction_count);

    // decleration of average turnaround time and average waiting time
    float avg_ta = 0;
    float avg_wt = 0;

    for (int i = 0; i < process_count; i++) {
        // for turnaround time I find completion time and arrival time		
        int turnaround_time = processes[i].completion_time - processes[i].arrival_time;
        // for waiting time I subtract burst time from turnaround time
        int waiting_time = turnaround_time - processes[i].initial_burst_time;
        avg_ta += turnaround_time;
        avg_wt += waiting_time;
    }

	avg_wt = avg_wt / process_count;
    printf("%.1f\n", avg_wt);

    avg_ta = avg_ta / process_count;
    printf("%.1f\n", avg_ta);

    

    return 0;
}
