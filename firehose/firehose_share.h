#ifndef FIREHOSE_SHARE_H_
#define FIREHOSE_SHARE_H_
#include "firehose.h"

#define MAX_LENGTH (1024*1024) //1M
#define MAX_RESP_LENGTH (1024*1024)

extern char respbuf[MAX_RESP_LENGTH];
extern char rubbish[MAX_LENGTH];
extern char bigchunk[MAXf_LENGTH*200];
extern size_t num_disk_sectors;

extern char *respbuf_ref(size_t *len);
extern int clear_rubbish();
extern int read_response(void *buf, int length, int *act);
extern int send_command(void *buf, int len);
extern static response_t _response(parse_xml_reader_func func);
extern response_t common_response();
extern size_t strtoint(char *s);

#endif
