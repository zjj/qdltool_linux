#ifndef _FIREHOSE_H
#define _FIREHOSE_H
#include "xml_parser.h"
#include "qdl_usb.h" 
#include "sahara.h" 
#include "firehose.h" 
#include "device.h" 
#include "utils.h"
#include "global.h"

#define send_data write_tx

typedef enum response_t {
    NAK, 
    ACK,
    NIL
} response_t;

typedef struct {
    char file_sector_offset_s[128];
    size_t file_sector_offset;

    char sector_size_s[128];
    size_t sector_size;

    char sector_numbers_s[128];
    size_t sector_numbers;
    
    char start_sector[128];
    size_t start_sector;

    bool sparse;
    char sparse_s[128]; 

    char filename[256];
    char xml[4096];
} firehose_program_t; 

typedef struct {
    char label[128];
    char sector_numbers[128];
    char start_sector[128];
    char storagedrive[128];
    
    char xml[4096];
} firehose_erase_t;

typedef struct {
    size_t max_payload;   
    
    char xml[4096];
} firehose_configure_t;

typedef struct{
    char what[4096];
    char filename[256]; 
    
    char xml[4096];
} firehose_patch_t;

typedef struct {
    char act[128];
    
    char xml[4096];
} firehose_power_t;

typedef response_t (*parse_xml_reader_func)(xml_reader_t *reader);

extern char bigchunk[MAX_LENGTH*200];
extern char xml_header[];

extern void parse_program_xml(char *xml, size_t length, program_t *program);
extern int send_program(program_t program);
extern response_t program_response();

extern response_t program_response_xml_reader();
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
