#include <osbind.h>
#include <stdarg.h>
#include <stdio.h>

#if 1
// There are problems with this, sometimes it gives wrong parameter values because the stack is misaligned.
void _debug(const char* __restrict__ s, ...)
{
    char msg[80];
    va_list ap;
    va_start(ap, s);
    sprintf(msg,s,ap);
    va_end(ap);
    char *c = msg;
    while (*c)
        Bconout(2, *c++);
    Bconout(2, '\r');
    Bconout(2, '\n');
}
#endif