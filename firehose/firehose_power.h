#ifndef _FIREHOSE_POWER_H
#define _FIREHOSE_POWER_H
#include "firehose.h"

typedef struct {
    char act[128];
    int delayinseconds;

    char xml[4096];
} firehose_power_t;

extern response_t power_response();
extern void init_firehose_power(char *act, firehose_power_t *power);
extern int send_firehose_power(firehose_power_t power);
extern response_t process_power_action(char *act);

#endif
