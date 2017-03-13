#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "sfuncs.h"


int streq_(char *a, char *b)
{
  unsigned long al, bl;
  if (a == NULL || b == NULL) return 0;
  al = strlen(a);
  bl = strlen(b);
  if (al != bl) return 0;
  if (memcmp(a,b,al)==0) return 1;
  return 0;
}

int startsame_(char *a, char *b)
{
  unsigned long al, bl;
  if (a == NULL || b == NULL) return 0;
  al = strlen(a);
  bl = strlen(b);
  if (al < bl)
  {
    if (memcmp(a,b,al)==0) return 1;
  }
  else
  {
    if (memcmp(a,b,bl)==0) return 1;
  }
  return 0;
}

void strltrim(char *ostr, char *istr)
{
  long n;
  for (n=0;istr[n]!=0;n++) if (istr[n] != ' ') break;
  strcpy(ostr, istr+n);
}
