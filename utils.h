#ifndef  _UTILS_H_
#define  _UTILS_H_
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "defs.h"

#define min(a, b) \
    ({ typeof(a) _a = (a); typeof(b) _b = (b); (_a < _b) ? _a : _b; })

extern size_t NUM_DISK_SECTORS;
extern size_t firehose_strtoint(char *s);

#endif
