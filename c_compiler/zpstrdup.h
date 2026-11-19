#ifndef ZPSTRDUP_H
#define ZPSTRDUP_H

/* strdup() is POSIX, not ISO C89 (so strict C89 builds see no prototype for it) */

#include <stdlib.h>
#include <string.h>

static char* zp_strdup(const char* s) {
    char* copy = (char*)malloc(strlen(s) + 1);
    if (copy) strcpy(copy, s);
    return copy;
}

#endif /* ZPSTRDUP_H */
