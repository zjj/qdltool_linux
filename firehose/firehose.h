#ifndef FIREHOSE_SHARE_H
#define FIREHOSE_SHARE_H
#include "global.h"
#include "xml_parser.h"
#include "qdl_usb.h"

#define XML_HEADER "<\?xml version=\"1.0\" encoding=\"UTF-8\" \?> "
#define MAX_LENGTH (1024*1024)
#define MAX_RESP_LENGTH (1024*1024)

typedef response_t (*parse_xml_reader_func)(xml_reader_t *reader);

extern char respbuf[MAX_RESP_LENGTH] = {0};
extern char rubbish[MAX_LENGTH] = {0};
extern char bigchunk[MAX_LENGTH*200] = {0};

extern char *respbuf_ref(size_t *len);
extern int clear_rubbish();
extern int read_response(void *buf, int length, int *act);
extern int send_command(void *buf, int len);
extern response_t _response(parse_xml_reader_func func);
extern response_t common_response();
extern size_t strtoint(char *s);

#endif
