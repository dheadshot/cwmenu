#ifndef __INC_EXECS_H__
#define __INC_EXECS_H__ 1

#include <unistd.h>

typedef struct BackgroundProcessWaitNode {
  pid_t pid;
  struct BackgroundProcessWaitNode *next;
} ProcWaitNode;


char **splitargsintontsa(char *args);
int runcommand(char *cmdline, int waitforcmd);
ProcWaitNode *AddPWN(pid_t pid);
int CleanPIDs();

#endif
