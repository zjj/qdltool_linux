#ifndef _FIREHOSE_FLASH_IMAGE_H
#define _FIREHOSE_FLASH_IMAGE_H

#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE
#endif
#include <sys/types.h>
#include <unistd.h>
#include "firehose.h"
#include "sparse_format.h"
#include "defs.h"
#include "utils.h"

response_t process_sparse_file(int fd, firehose_program_t program);
response_t process_general_file(int fd, firehose_program_t program);

#endif
