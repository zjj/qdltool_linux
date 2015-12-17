#ifndef __FIREHOSE_H_
#define __FIREHOSE_H_
#include "utils.h"
#include "xml_parser.h"
#include "qdl_usb.h"
#include "defs.h"

#define send_data write_tx

#define MAX_LENGTH (1024*1024) //1M
#define MAX_RESP_LENGTH (1024*1024)
#define XML_HEADER "<\?xml version=\"1.0\" \?>"

extern char respbuf[MAX_RESP_LENGTH];
extern char rubbish[MAX_LENGTH];
extern char bigchunk[MAX_LENGTH*200];
extern size_t NUM_DISK_SECTORS;

typedef enum response_t {
    NAK, 
    ACK,
    NIL 
} response_t;

typedef struct {
    char xml[4096]; 
} firehose_configure_t;

typedef response_t (*parse_xml_reader_func)(xml_reader_t *reader);
typedef response_t (*get_maxpayload_support_from_xreader_func)(xml_reader_t *reader, char *maxpayload_support, int sz);
extern response_t process_firehose_configure();
extern response_t firehose_emmc_info_response_xml_reader(xml_reader_t *reader);
extern response_t firehose_emmc_info_reponse();
extern response_t firehose_emmc_info();

typedef struct {
    char label[128];
    char sector_numbers[128];
    char start_sector[128];
    char storagedrive[128];
    
    char xml[4096];
} firehose_erase_t;

extern response_t process_firehose_erase_xml(char *xml, int length);
extern init_firehose_erase(xml_reader_t *reader, firehose_erase_t *erase);
extern int send_firehose_erase(firehose_erase_t erase);
extern response_t firehose_erase_response();


typedef struct {
    char filename[256];
    char what[256];
    
    char xml[4096];
} firehose_patch_t;

extern response_t process_patch(char *xml);
extern response_t process_firehose_patch_xml(char *, int);


typedef struct {
    char act[128];
    int delayinseconds;

    char xml[4096];
} firehose_power_t;

extern response_t power_response();
extern void init_firehose_power(char *act, firehose_power_t *power);
extern int send_firehose_power(firehose_power_t power);
extern response_t process_power_action(char *act);


typedef struct {
    size_t file_sector_offset;
    size_t sector_size;
    size_t sector_numbers;
    size_t physical_partition_number;
    size_t start_sector;
    bool sparse;
    char filename[256];
    char label[256];

    char xml[4096];
} firehose_program_t;

typedef struct {
    size_t file_sector_offset;
    size_t sector_size;
    size_t sector_numbers;
    size_t physical_partition_number;
    size_t start_sector;
    bool sparse;
    char filename[256];
    char label[256];
    size_t len;

    char xml[4096];
} firehose_simlock_t;

extern int init_firehose_simlock_from_xml_reader(xml_reader_t *reader, firehose_simlock_t *slk);

extern char *respbuf_ref(size_t *len);
extern int clear_rubbish();
extern int read_response(void *buf, int length, int *act);
extern int send_command(void *buf, int len);
extern size_t strtoint(char *s);


#endif
