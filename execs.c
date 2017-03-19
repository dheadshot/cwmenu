#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "sfuncs.h"

#include "execs.h"


ProcWaitNode *pwnroot = NULL, *pwnptr = NULL;


char **splitargsintontsa(char *args)
{
  unsigned long nargs = 1, i, evn = 0;
  int indq = 0, insq = 0, ines = 0, lws = 0, inev = 0;
  
  	printf("## Args: \"%s\"\n",args);
  
  for (i=0;args[i]!=0;i++)
  {
    switch (args[i])
    {
      case '\\':
        if (insq == 0 && ines==0) ines = 1;
        else ines = 0;
        lws = 0;
      break;
      
      case '$':
        if (insq == 0 && ines == 0)
        {
          evn++;
        }
        ines = 0;
      break;
      
      case '\'':
        if (ines == 0)
        {
          insq = 1-insq;
        }
        else inev = 0;
        ines = 0;
        lws = 0;
      break;
      
      case '"':
        if (insq == 0 && ines==0)
        {
          indq = 1-indq;
        }
        else if (insq==0) inev = 0;
        ines = 0;
        lws = 0;
      break;
      
      case ' ':
      case '\t':
      case '\n':
        if (insq == 0 && indq == 0 && ines == 0 && lws == 0)
        {
          nargs++;
          inev = 0;
        }
        else if (insq == 0 && ines == 0 && lws == 0) inev = 0;
        ines = 0;
        lws = 1;
      break;
      
      case '\r':
        /* Ignore these */
      break;
      
      default:
        ines = 0;
        lws = 0;
      break;
    }
  }
  
  char **ans = (char **) malloc(sizeof(char *)*(nargs+1));
  if (ans == NULL) return NULL;
  ans[nargs] = (char *) NULL;
  unsigned long j = 0, n = 0, k = 0;
  char /* anarg[1024] = ""; */ *anarg = (char *) malloc(sizeof(char)*(1+evn)*1024);
  char *evptr = NULL, anev[1024] = "";
  if (anarg == NULL)
  {
    free(ans);
    return NULL;
  }
  
  for (i=0;args[i]!=0;i++)
  {
    switch (args[i])
    {
      case '\\':
        if (insq == 0 && ines==0) ines = 1;
        else
        {
          ines = 0;
          anarg[j] = '\\';
          j++;
        }
        lws = 0;
      break;
      
      case '$':
        if (insq == 0 && ines == 0)
        {
          if (inev)
          {
            anev[k] = 0;
            if (k == 0) evptr = getenv("$");
            else evptr = getenv(anev);
            if (evptr != NULL)
            {
              anarg[j] = 0;
              strcat(anarg,evptr);
              j = strlen(anarg);
            }
            k = 0;
          }
          inev = 1;
        }
        else
        {
          anarg[j] = '$';
          j++;
        }
        ines = 0;
      break;
      
      case '\'':
        if (ines == 0)
        {
          insq = 1-insq;
        }
        else
        {
          anarg[j] = '\'';
          j++;
          inev = 0;
        }
        ines = 0;
        lws = 0;
      break;
      
      case '"':
        if (insq == 0 && ines==0)
        {
          indq = 1-indq;
        }
        else
        {
          inev = 0;
          anarg[j] = '"';
          j++;
        }
        ines = 0;
        lws = 0;
      break;
      
      case ' ':
      case '\t':
      case '\n':
        if (insq == 0 && indq == 0 && ines == 0 && lws == 0)
        {
          if (inev!=0)
          {
            anev[k] = 0;
            if (k == 0) evptr = getenv("$");
            else evptr = getenv(anev);
            if (evptr != NULL)
            {
              anarg[j] = 0;
              strcat(anarg,evptr);
              j = strlen(anarg);
            }
            k = 0;
            inev = 0;
          }
          anarg[j] = 0;
          ans[n] = (char *) malloc(sizeof(char)*(j+1));
          if (ans[n] == NULL)
          {
            /* Erk!  Free everything! */
            for (j=0;j<n;j++)
            {
              free(ans[j]);
              free(anarg);
            }
            free(ans);
            return NULL;
          }
          strcpy(ans[n],anarg);
          	printf("## arg[%lu]=\"%s\"\n",n,ans[n]);
          n++;
          j = 0;
          inev = 0;
        }
        else
        {
          if (insq == 0 && ines == 0 && lws == 0)
          {
            inev = 0;
            anev[k] = 0;
            if (k == 0) evptr = getenv("$");
            else evptr = getenv(anev);
            if (evptr != NULL)
            {
              anarg[j] = 0;
              strcat(anarg,evptr);
              j = strlen(anarg);
            }
            k = 0;
          }
          anarg[j] = args[i];
          j++;
        }
        ines = 0;
        lws = 1;
      break;
      
      case '\r':
        /* Ignore these */
      break;
      
      default:
        ines = 0;
        lws = 0;
        if (inev)
        {
          anev[k] = args[i];
          k++;
        }
        else
        {
          anarg[j] = args[i];
          j++;
        }
      break;
    }
  }
  
  if (inev!=0)
  {
    anev[k] = 0;
    if (k == 0) evptr = getenv("$");
    else evptr = getenv(anev);
    if (evptr != NULL)
    {
      anarg[j] = 0;
      strcat(anarg,evptr);
      j = strlen(anarg);
    }
    k = 0;
    inev = 0;
  }
  anarg[j] = 0;
  ans[n] = (char *) malloc(sizeof(char)*(j+1));
  if (ans[n] == NULL)
  {
    /* Erk!  Free everything! */
    for (j=0;j<n;j++)
    {
      free(ans[j]);
      free(anarg);
    }
    free(ans);
    return NULL;
  }
  strcpy(ans[n],anarg);
  	printf("## arg[%lu]=\"%s\"\n",n,ans[n]);
  
  ans[nargs] = (char *) NULL;
  
  
  return ans;
}


int runcommand(char *cmdline, int waitforcmd)
{
  char **args;
  int ans = 0;
  
  args = splitargsintontsa(cmdline);
  if (args == NULL) return -1;
  
  pid_t childpid;
  
  childpid = fork();
  if (childpid == -1) return -1;
  if (childpid == 0)
  {
    /* Child */
    if (execvp(args[0], args) == -1) exit(1);
    return -3; /* Should never happen! */
  }
  else
  {
    /* Parent */
    if (waitforcmd)
    {
      while (waitpid(childpid,&ans,WNOHANG) == 0)
      {
        sleep(1);
      }
      if (ans < 0) ans = -ans;
    }
    else
    {
      if (AddPWN(childpid) == NULL) ans = -2;
      else ans = -10;
    }
    freentsa(args);
    return ans;
  }
}

ProcWaitNode *AddPWN(pid_t pid)
{
  ProcWaitNode *apwn;
  
  apwn = (ProcWaitNode *) malloc(sizeof(ProcWaitNode));
  if (apwn == NULL) return NULL;
  apwn->pid;
  apwn->next = NULL;
  
  if (pwnroot == NULL)
  {
    pwnroot = apwn;
  }
  else
  {
    for (pwnptr = pwnroot; pwnptr->next != NULL; pwnptr = pwnptr->next)
    {
      /* Navigate pwn list */
    }
    pwnptr->next = apwn;
  }
  return apwn;
}

int CleanPIDs()
{
  int status;
  ProcWaitNode *ppwn = NULL;
  
  if (pwnroot == NULL) return 1;
  for (pwnptr = pwnroot; pwnptr != NULL; pwnptr = pwnptr->next)
  {
    if (waitpid(pwnptr->pid,&status,WNOHANG) != 0)
    {
      if (ppwn == NULL) pwnroot = pwnptr->next;
      else ppwn->next = pwnptr->next;
      free(pwnptr);
      pwnptr = NULL;
      return status;
    }
    ppwn = pwnptr;
  }
  return 1;
}

void clean_pwnlist()
{
  int status;
  while (pwnroot != NULL)
  {
    waitpid(pwnroot->pid,&status,WNOHANG); /* Clear if possible */
    pwnptr = pwnroot->next;
    free(pwnroot);
    pwnroot = pwnptr;
  }
  
}
