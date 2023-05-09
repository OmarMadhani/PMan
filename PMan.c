// Name: Omar Madhani
// Date Completed: 2/9/2023
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "PMan.h"

struct Process {
    pid_t processID;
    char processName[100]; // command requested by user
    int status;
    
    // terminationProcessed simulates removing a process from our linked list
    int terminationProcessed; // A value 0 (process currently running), 1 (user has been told the process has terminated), or 2 (process currently stopped)
    struct Process* next; // pointer to the next process in the linked list
};

int main() {
    usleep(150000);
    
    // intially set the head of our linked list of processes to NULL
    struct Process* head = NULL;
    getUserCommand(&head);   
    return 0;
}

// used to strip the new line escape sequence from the user input
void removeNewLine(char* string) {
    int index = 0;
    while (string[index] != '\n') {
        index++;
    }
    string[index] = '\0';
    return;
}

// updates a process status with the WNOHANG option
void obtainCurrentProcessStatus(struct Process* node) {
    if (node == NULL) {
        return;
    }
    int status;
    node->status = waitpid(node->processID, &status, WNOHANG);
}

// updates the status of every process in our linked list
void updateProcessStatusInList(struct Process** head) {
    if (head == NULL) {
        return;
    }
    struct Process* curNode = *head;
    while (curNode != NULL) {
        obtainCurrentProcessStatus(curNode);
        curNode = curNode->next;
    }
}

void bgStartFunction(char* processID, struct Process** head) {
    struct Process* curNode = *head;
    while (curNode != NULL) {
        updateProcessStatusInList(head);

        // only execute the following code if the process is currently stopped (indicated by terminationProcessed = 2)
        // also check if the child process has not ended
        if ((int) curNode->processID == atoi(processID) && curNode->terminationProcessed == 2 && (curNode->status != -1 && curNode->status != (int) curNode->processID)) {
            kill(atoi(processID), SIGCONT);
            
            // indicate that the process is now running by setting terminationProcessed to 0;
            curNode->terminationProcessed = 0;
            printf("Process %s resumed\n", processID);
            usleep(100000);
            return;
        
        // tell the user that the process they wish to start is currently running (indicated by terminationProcessed = 0)
        } else if ((int) curNode->processID == atoi(processID) && curNode->terminationProcessed == 0 && (curNode->status != -1 && curNode->status != (int) curNode->processID)) {
            printf("Process %s already running\n", processID);
            return;
        }
        curNode = curNode->next;
    }
    printf("Error: Process %s does not exist.\n", processID);
}

void bgStopFunction(char* processID, struct Process** head) {
    struct Process* curNode = *head;
    while (curNode != NULL) {
        updateProcessStatusInList(head);
        
        // only execute the following code if the process is currently stopped (indicated by terminationProcessed = 0)
        if ((int) curNode->processID == atoi(processID) && curNode->terminationProcessed == 0 && (curNode->status != -1 && curNode->status != (int) curNode->processID)) {
            
            // send the SIGSTOP signal to a process and update its terminationProcessed attribute to 2
            kill(atoi(processID), SIGSTOP);
            curNode->terminationProcessed = 2;
            printf("Process %s temporarily stopped\n", processID);
            usleep(100000);
            return;
        
        // tell the user that the process they wish to stop is currently stopped (indicated by terminationProcessed = 2)
        } else if ((int) curNode->processID == atoi(processID) && curNode->terminationProcessed == 2 && (curNode->status != -1 && curNode->status != (int) curNode->processID)) {
            printf("Process %s currently stopped\n", processID);
            return;
        }
        curNode = curNode->next;
    }
    printf("Error: Process %s does not exist.\n", processID);
}

void bgKillFunction(char* processID, struct Process** head, int killAll) {
    struct Process* curNode = *head;
    while (curNode != NULL) {
        if ((int) curNode->processID == atoi(processID)) {
            
            // only execute the following statements if the process is currently running
            if (curNode->terminationProcessed == 0) {
                
                // sends the SIGTERM signal to a process
                kill(atoi(processID), SIGTERM);
                usleep(100000);
                
                // terminationProcessed is updated to 1 to indicate that the process has been terminated
                curNode->terminationProcessed = 1;
                printf("Process: %s terminated\n", processID);
                return;
            } else if (curNode->terminationProcessed == 2) { // (if the process is currently stopped)
                
                // start the process and then immediately kill it. I experienced issues by killing a process while it was stopped
                kill(atoi(processID), SIGCONT);
                kill(atoi(processID), SIGKILL);
                usleep(100000);
                curNode->terminationProcessed = 1;
                printf("Process: %s terminated\n", processID);
                return;  
            }
            
        }
        curNode = curNode->next;
    }

    if (killAll == 0) {
        printf("Error: Process %s does not exist.\n", processID);
    }
}

// kills all the processes created by PMan (used when the user exits PMan)
void killAll(struct Process** head) {
    struct Process* curNode = *head;
    char processIdStr[100];
    while (curNode != NULL) {
        sprintf(processIdStr, "%d", (int) curNode->processID);
        
        // we pass in 1 as a parameter to the bgKillFunction to avoid printing "Error: Proces (pid) does not exist."
        bgKillFunction(processIdStr, head, 1);
        curNode = curNode->next;
    }
}

// generates the file path of the stat file of a pid in /proc
void getProcStatFilePath(char* procFilePath, char* processID) {
    strcat(procFilePath, "/proc/");
    strcat(procFilePath, processID);
    strcat(procFilePath, "/stat");
}

// generates the file path of the status file of a pid in /proc
void getProcStatusFilePath(char* procFilePath, char* processID) {
    strcat(procFilePath, "/proc/");
    strcat(procFilePath, processID);
    strcat(procFilePath, "/status");
}

// prints the comm, state, utime, stime, and rss of a process obtained from the stat file in /proc
void printProcStatContent(char* procStatFilePath) {
    FILE *procStat = fopen(procStatFilePath, "r");
    
    // immediately return if we cannot locate a file
    if (procStat == NULL) {
        return;
    }
    
    // throwaway variables used to store unwanted information from the stat file while using fscanf
    int ignoreInt;
    unsigned ignoreU;
    long int ignoreLint;
    long unsigned ignoreLu;
    long long unsigned ignoreLlu;

    // variables to store information that will be printed out
    char comm[300];
    char processState;
    long unsigned utimeVal;
    long unsigned stimeVal;
    long int rssVal;

    // reference for fscanf: https://www.tutorialspoint.com/c_standard_library/c_function_fscanf.htm
    fscanf(procStat, "%d %s %c %d %d %d %d %d %u %lu %lu %lu %lu %lu %lu %ld %ld %ld %ld %ld %ld %llu %lu %ld", 
    &ignoreInt, comm, &processState, &ignoreInt, &ignoreInt, &ignoreInt, &ignoreInt, &ignoreInt, &ignoreU, &ignoreLu, &ignoreLu, &ignoreLu, &ignoreLu,
    &utimeVal, &stimeVal, &ignoreLint, &ignoreLint, &ignoreLint, &ignoreLint, &ignoreLint, &ignoreLint, &ignoreLlu, &ignoreLu, &rssVal);

    fclose(procStat);
    printf("\ncomm: %s\n", comm);
    printf("state: %c\n", processState);
    
    //cast the utime and stime to a float before printing and divide by sysconf(_SC_CLK_TCK)
    printf("utime: %.2f\n", ((float) utimeVal)/sysconf(_SC_CLK_TCK));
    printf("stime: %.2f\n", ((float) stimeVal)/sysconf(_SC_CLK_TCK));
    printf("rss: %ld\n", rssVal);
    strcpy(procStatFilePath, "");
}

// adjusts the formatting of the voluntary and nonvoluntary context switch lines obtained from the status file.
void removeSpaces(char* destinationString, char* string) {
    int stringIndex = 0;
    int destStringIndex = 0;
    
    // continue adding characters from the original string to destinationString until we reach ':'
    while (stringIndex < strlen(string)) {
        destinationString[destStringIndex] = string[stringIndex];
        if (string[stringIndex] == ':') {
            stringIndex++;
            destStringIndex++;
            break;
        }    
        stringIndex++;
        destStringIndex++;
    }

    // the character at this index would be a tab
    stringIndex++; // skip the tab character by incrementing the index counter of the original string
    
    // insert a space (rather than a tab) into destinationString
    destinationString[destStringIndex] = ' '; 
    destStringIndex++;
    
    // continue adding characters from the orginal string to destinationString until we reach a space
    while (stringIndex < strlen(string)) {
        if (string[stringIndex] ==  ' ') {
            
            // insert a new line and null terminator to destinationString once we've reached a space in the original string
            strcat(destinationString, "\n\0");
            return;
        }
        destinationString[destStringIndex] = string[stringIndex];
        stringIndex++;
        destStringIndex++;
    }
}

// prints the number of voluntary and nonvoluntary context switches of a process obtained from the status file in /proc
void printProcStatusContent(char* procStatusFilePath) {
    FILE *procStatus = fopen(procStatusFilePath, "r");
    
    // immediately return if we cannot locate a file
    if (procStatus == NULL) {
        return;
    }
    
    char voluntaryCxtSwitch[1000];
    char nonVoluntaryCxtSwitch[1000];
    char statusLine[3000];
    
    // continue reading lines from the status file using fgets until we reach the voluntary and nonvoluntary context switch lines
    while (fgets(statusLine, 3000, procStatus) != NULL) {
        if (strstr(statusLine, "nonvoluntary_ctxt_switches") != NULL) {
            strcpy(nonVoluntaryCxtSwitch, statusLine);
        } else if(strstr(statusLine, "voluntary_ctxt_switches") != NULL) {
            strcpy(voluntaryCxtSwitch, statusLine);
        } 
    }
    
    fclose(procStatus);
    
    // obtain the print format for the voluntary and nonvoluntary context switches
    char reducedSpaceVoluntaryCxtSwitch[300];
    char reducedSpaceNonVoluntaryCxtSwitch[300];
    removeSpaces(reducedSpaceVoluntaryCxtSwitch, voluntaryCxtSwitch);
    removeSpaces(reducedSpaceNonVoluntaryCxtSwitch, nonVoluntaryCxtSwitch);
    printf("%s", reducedSpaceVoluntaryCxtSwitch);
    printf("%s\n", reducedSpaceNonVoluntaryCxtSwitch);
    strcpy(procStatusFilePath, "");
}

// calls the functions above to print the comm, state, utime, stime, rss, voluntary context switches, and nonvoluntary context switches of a desired process
void pStatFunction(char* processID, struct Process **head) {
    updateProcessStatusInList(head);
    struct Process* curNode = *head;
    
    // traverse the linked list until we reach the correct process
    while (curNode != NULL) {
        if (curNode->processID == atoi(processID) && curNode->terminationProcessed != 1 && curNode->status != -1) {
            char procStatFilePath[1000];
            strcpy(procStatFilePath, "");
            getProcStatFilePath(procStatFilePath, processID);
            printProcStatContent(procStatFilePath);
    
            char procStatusFilePath[1000];
            strcpy(procStatusFilePath, "");
            getProcStatusFilePath(procStatusFilePath, processID);
            printProcStatusContent(procStatusFilePath);
            
            return;
        }
        curNode = curNode->next;
    }
    
    //print the following if we cannot locate a running process in our list with the pid provided
    printf("Error: Process %s does not exist.\n", processID);
}

// will print processes id's of terminated background jobs when bglist is called
void printTerminatedProcesses(int terminatedProcesses[], int termProcessedIndex) {
    if (termProcessedIndex > 0) {
        printf("Process: ");
        int index;
        for (index = 0; index < termProcessedIndex; index++) {
            if (termProcessedIndex == 1) {
                printf("%d ", terminatedProcesses[index]);
            } else if (index == 0) {
                printf("%d,", terminatedProcesses[index]);
            } else if (index + 1 < termProcessedIndex) {
                printf(" %d,", terminatedProcesses[index]);
            } else {
                printf(" %d ", terminatedProcesses[index]);
            }
        }
        printf("terminated\n");
    }
}

// prints the pid and file path of currently running processes in our linked list
// prints out the number of background jobs currently running
// acknowledges the termination of processes
// getcwd reference: https://www.ibm.com/docs/en/zos/2.3.0?topic=functions-getcwd-get-path-name-working-directory
void bgListFunction(struct Process* head) {
    usleep(100000);
    
    // obtain the most up to date status of every process in our linked list
    updateProcessStatusInList(&head);
    int terminatedProcesses[100];
    int termProcessedIndex = 0;
    
    // count stores the number of background jobs currently running
    int count = 0; 
    struct Process* curNode = head;
    while (curNode != NULL) {
        
        // if the process has not been terminated
        if (curNode->status != -1 && curNode->status != curNode->processID) {
            char currentWorkingDirectory[200];
            
            // if the user ran an executable with its full path, no need to print the cwd
            // reference for cwd: https://www.ibm.com/docs/en/zos/2.3.0?topic=functions-getcwd-get-path-name-working-directory
            if (strstr(curNode->processName, getcwd(currentWorkingDirectory, sizeof(currentWorkingDirectory))) != NULL) {
                printf("%d: %s\n", curNode->processID, curNode->processName);
            } 
            
            // if the user ran an executable using ./[executable_name], print out the cwd before the executable name
            else {
                printf("%d: %s/%s\n", curNode->processID, currentWorkingDirectory, curNode->processName); //temp->status);
            }
            
            // increment the number of background jobs currently running
            count++;
        } 
        
        // if the process has terminated, we must tell the user that it has been terminated
        else {
            
            // if PMan has not yet acknowledged that this process has been terminated
            if (curNode->terminationProcessed != 1) {
                
                // the process id's in this array will be printed out only once (the next time bglist is called by the user) 
                terminatedProcesses[termProcessedIndex] = (int) curNode->processID;
                
                // set the terminationProcessed attribute to 1 to avoid telling the user that the process has terminated every time bglist is called 
                curNode->terminationProcessed = 1;
                termProcessedIndex++;
            }
        }
        curNode = curNode->next;
    }

    // print the total number of background jobs
    printf("\nTotal background jobs: %d\n", count);
    printTerminatedProcesses(terminatedProcesses, termProcessedIndex);
}

// forks this process and calls execvp to execute the user's command
void bgFunction(char* executable, char* argumentList[], struct Process** head) {
    
    // will be passed into execvp as a required parameter
    char* argv[] = {executable, NULL};
    
    // reference for fork: https://stackoverflow.com/questions/63616082/check-whether-child-process-has-terminated-in-c-on-unix-without-blocking
    pid_t newProcessID = fork();
    
    // if the fork did not work, return immediately
    if (newProcessID < 0) {
        return;
    } 
    
    else if (newProcessID != 0) {
        printf("Process (%d) created\n\n", newProcessID);
    }
    
    // if this is the child process (execute user's command)
    if (newProcessID == 0) {
        if (execvp(executable, argumentList) == -1) {
            printf("Error: execution of \"%s\" failed\n", executable);
            usleep(220000);
            exit(-1);
            return;
        }
    
    // if this is the parent process
    } else {
        usleep(300000);
        
        // create a new process node and define its initial attributes
        struct Process* node = (struct Process*)malloc(sizeof(struct Process));
        node->processID = newProcessID;
        obtainCurrentProcessStatus(node);
        
        char currentWorkingDirectory[200];
        
        // if the user ran an executable using its full path, simply store this path into the processName attribute
        if (strstr(executable, getcwd(currentWorkingDirectory, sizeof(currentWorkingDirectory))) != NULL) {
            strcpy(node->processName, executable);
        } 
        
        // if the user ran an executable using ./[executable_name], store the characters following ./ in the processName attribute
        else {
            strcpy(node->processName, &executable[2]);
        }
        node->next = NULL;

        // if our linked list is empty, make this new node the head
        if (*head == NULL) {
            *head = node; 
            return;
        }
        
        // otherwise, traverse our linked list until we reach an opening and then insert the new node
        struct Process *curNode = *head;
        while (curNode->next != NULL) {
            curNode = curNode->next;
        }
        curNode->next = node;
    }    
}

// calls the appropriate functions based on the users command
void userCommandHandler(char* argument, char* argumentList[], struct Process** head) {
   
   // if the user wants to start a background job
   if (strcmp(argument, "bg") == 0) {    
        if (argumentList[1] != NULL) {
            bgFunction(argumentList[1], &argumentList[1], head);
        } else {
            printf("Error: Invalid number of arguments\n");
        }
    
    // if the user wants a list of their current background processes or an indication of their terminated processes
    } else if (strcmp(argument, "bglist") == 0) {
        bgListFunction(*head);
    
    // if the user wants to forcefully terminate a process
    } else if (strcmp(argument, "bgkill") == 0) {
        if (argumentList[1] != NULL) {
            bgKillFunction(argumentList[1], head, 0);
        } else {
            printf("Error: Invalid number of arguments\n");
        }
    
    // if the user wants to temporarily stop a process
    } else if (strcmp(argument, "bgstop") == 0) {
        if (argumentList[1] != NULL) {
            bgStopFunction(argumentList[1], head);
        } else {
            printf("Error: Invalid number of arguments\n");
        }
    
    // if the user wants to resume a process
    } else if (strcmp(argument, "bgstart") == 0) {
        if (argumentList[1] != NULL) {
            bgStartFunction(argumentList[1], head);
        } else {
            printf("Error: Invalid number of arguments\n");
        }
    
    // if the user wants information about a process
    } else if (strcmp(argument, "pstat") == 0) {
        if (argumentList[1] != NULL) {
            pStatFunction(argumentList[1], head);
        } else {
            printf("Error: Invalid number of arguments\n");
        }
    } else {
        printf("PMan: > %s: command not found\n", argument);
    }
}

// prompts the user for input
void getUserCommand(struct Process** head) {
    while(1) {
        
        // obtain the most up to date status of every process in our linked list
        updateProcessStatusInList(head);
        char command[100];
        printf("PMan: > ");
        fgets(command, 100, stdin);
        
        // if the user provided some input
        if (command[0] != '\n') {
            removeNewLine(command);
            const char delim[2] = " ";
            char* argument;
            char* argumentList[100];

            // initially set every index in our argument list to NULL
            for (int index = 0; index < 100; index++) {
                argumentList[index] = NULL;
            }
            
            int index = 0;

            // reference for strtok: https://www.tutorialspoint.com/c_standard_library/c_function_strtok.htm
            argument = strtok(command, delim);
            
            // if the user wishes to exit PMan, kill all processes in our linked list and break from the while loop
            if (strcmp(argument, "q") == 0) {
                killAll(head);
                break;
            } 

            // continue adding tokens to argumentList
            while (argument != NULL) {
                argumentList[index] = argument;
                index++;
                argument = strtok(NULL, delim);
            }

            argument = argumentList[0];
            userCommandHandler(argument, argumentList, head);
        }
    }
}