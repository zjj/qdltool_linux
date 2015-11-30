#ifndef _FIREHOSE_PATCH_H
#define _FIREHOSE_PATCH_H
#include "firehose.h"

typedef struct {
    // pass

    char xml[4096];
} firehose_patch_t;

extern responst_t process_patch(char *xml);

#endif

