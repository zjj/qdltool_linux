#ifndef _FIREHOSE_ERASE_H
#define _FIREHOSE_ERASE_H
#include "firehose.h"

typedef struct {
    char label[128];
    char sector_numbers[128];
    char start_sector[128];
    char storagedrive[128];
    
    char xml[4096];
} firehose_erase_t;

extern reponse_t process_firehose_erase_xml(char *xml, int length);
extern init_firehose_erase(xml_reader_t *reader, firehose_erase_t *erase);
extern send_firehose_erase(firehose_erase_t *erase);
extern firehose_erase_response();

#endif
