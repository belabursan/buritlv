#include "dbg.h"
#include <stdio.h>



void dbg (const char *txt, ...)
{
#ifdef DEBUG
    va_list args;
    va_start (args, txt);
    vprintf (txt, args);
    printf ("\n");
    va_end (args);
#else
    (void)txt; //disable "unused variable" warning
#endif
}

void error (const char *txt, ...)
{
#ifdef DEBUG
    va_list args;
    printf ("ERROR: ");
    va_start (args, txt);
    vprintf (txt, args);
    printf ("\n");
    va_end (args);
#else
    (void)txt; //disable "unused variable" warning
#endif
}

void sysl(const int prio, const char *txt, ...)
{
    va_list args;
    va_start (args, txt);
    vsyslog (prio, txt, args);
    va_end (args);
}
