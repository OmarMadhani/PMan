// Name: Omar Madhani
// Date Completed: 2/9/2023

#ifndef PMan
#define PMan

struct Process;
void removeNewLine(char* string);
void obtainCurrentProcessStatus(struct Process* node);
void updateProcessStatusInList(struct Process** head);
void bgStartFunction(char* processID, struct Process** head);
void bgStopFunction(char* processID, struct Process** head);
void bgKillFunction(char* processID, struct Process** head, int killAll);
void killAll(struct Process** head);
void getProcStatFilePath(char* procFilePath, char* processID);
void getProcStatusFilePath(char* procFilePath, char* processID);
void printProcStatContent(char* procStatFilePath);
void removeSpaces(char* destinationString, char* string);
void printProcStatusContent(char* procStatusFilePath);
void pStatFunction(char* processID, struct Process **head);
void printTerminatedProcesses(int terminatedProcesses[], int termProcessedIndex);
void bgListFunction(struct Process* head);
void bgFunction(char* executable, char* argumentList[], struct Process** head);
void userCommandHandler(char* argument, char* argumentList[], struct Process** head);
void getUserCommand(struct Process** head);

#endif