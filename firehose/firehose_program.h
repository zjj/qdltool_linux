#ifndef FIREHOSE_PROGRAM_H_
#define FIREHOSE_PROGRAM_H_
#include "firehose_share.h"

typedef struct {
    size_t file_sector_offset;
    size_t sector_size;
    size_t sector_numbers;
    size_t physical_partition_number;
    size_t start_sector;
    bool sparse;
    char filename[256];

    char xml[4096];
} firehose_program_t;

#endif
