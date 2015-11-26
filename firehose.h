#ifndef _FIREHOSE_H
#define _FIREHOSE_H
#include "xml_parser.h"
#include "qdl_usb.h"
#include "global.h"


typedef enum response_t {
    NAK, 
    ACK,
    NIL
} response_t;

typedef response_t (*parse_xml_reader_func)(xml_reader_t reader);

extern char bigchunk[MAX_LENGTH*100];
extern char xml_header[];
extern void parse_program_xml(char *xml,
                                 size_t length,
                                 size_t *offset,
                                 size_t *sector_size,
                                 size_t *sector_numbers,
                                 size_t *start_sector,
                                 char *filename,
                                 bool *sparse);
extern response_t program_response();
extern response_t program_response_xml_reader();
extern int send_program_xml(char *xml, int length);
extern int send_rawdata(char *buf, int size);
extern response_t firehose_configure(size_t *payload);
extern response_t transmit_chunk(char *chunk,
                                size_t sector_number,
                                size_t sector_size,
                                size_t start_sector,
                                u8 physical_partition_number);
extern int send_patch_xml(char *xml, int length);
extern response_t patch_response();
extern response_t common_response();
extern void generate_patch_xml(char *patch, char *line);
extern response_t partition_erase(size_t start_sector, size_t sector_numbers, u8 physical_partition_number);

#endif
