#include "firehose.h"

#define MAX_RESP_LENGTH MAX_LENGTH
char respbuf[MAX_RESP_LENGTH] = {0};
char rubbish[MAX_LENGTH] = {0};
char bigchunk[MAX_LENGTH*200] = {0};

#define XML_HEADER "<\?xml version=\"1.0\" encoding=\"UTF-8\" \?> "

char *respbuf_ref(size_t *len)
{
    if(len)
        *len = sizeof(respbuf);
    memset(respbuf, 0, sizeof(respbuf));
    return respbuf;
}

static int clear_rubbish()
{
    bzero(rubbish, sizeof(rubbish));
    return read_rx_timeout(rubbish, sizeof(rubbish), NULL, 10);
}

int read_response(void *buf, int length, int *act)
{
    memset(buf, 0, length);
    return read_rx_timeout(buf, length, act, 100);
}

int send_command(void *buf, int len)
{
    clear_rubbish();
    return write_tx(buf, len, NULL);
}

int send_program_xml(char *xml, int length)
{
    clear_rubbish();
    return send_command(xml, length);
}

int send_patch_xml(char *xml, int length)
{
    clear_rubbish();
    return send_command(xml, length);
}

static response_t _response(parse_xml_reader_func func)
{
    int r, status;
    int retry = MAX_RETRY;
    size_t len = 0;  
    char *buf = respbuf_ref(&len);
    char *ptr = buf;
    response_t response;

    while(retry>0){
        r = 0;
        status = read_response(ptr, len-(ptr-buf), &r);
        if (r > 0){ 
            ptr += r;
        }
        if (status < 0){
            if(ptr>respbuf){
                xml_reader_t reader;
                xmlInitReader(&reader, buf, ptr-buf);
                response = func(&reader);
                if(response != NIL){
                    return response;
                }
                retry --; 
                usleep(1000);
                continue;
            }
        }
        retry --; 
        usleep(1000);
    }
    return NIL;
}

response_t common_response()
{
    return _response(common_response_xml_reader);
}

response_t patch_response()
{
    return common_response();
}

response_t power_response()
{
    return common_response();
}

response_t transmit_response_xml_reader(xml_reader_t *reader)
{
    xml_token_t token;
    char ack[128] = {0};
    char rawmode[128] = {0};
    bool is_response_tag = FALSE;
    while ((token = xmlGetToken(reader)) != XML_TOKEN_NONE) {
        if(token == XML_TOKEN_TAG){
            is_response_tag = xmlIsTag(reader, "response");
            if(is_response_tag){
                bzero(ack, sizeof(ack));
                bzero(rawmode, sizeof(rawmode));
            }
        }
        if (token == XML_TOKEN_ATTRIBUTE && is_response_tag) {
            if (xmlIsAttribute(reader, "value"))
                xmlGetAttributeValue(reader, ack, sizeof(ack));

            if (xmlIsAttribute(reader, "rawmode"))
                xmlGetAttributeValue(reader, rawmode, sizeof(rawmode));

            if (ack[0] && rawmode[0]){
                return (strncasecmp(ack, "ACK", 3)==0 
                        && strncasecmp(rawmode, "false", 5)==0)?ACK:NAK;
            }
        }
    }
    return NIL;
}

response_t firehose_configure_2nd_xml_reader(xml_reader_t *reader)
{
    char ack[128] = {0};
    char maxpayload[4096] = {0}; 
    
    bool is_response_tag = FALSE;
    xml_token_t token;
    while ((token = xmlGetToken(reader)) != XML_TOKEN_NONE) {
        if(token == XML_TOKEN_TAG){
            bzero(ack, sizeof(ack));
            bzero(maxpayload, sizeof(maxpayload));
            is_response_tag = xmlIsTag(reader, "response");
            if(is_response_tag){
                bzero(ack, sizeof(ack));
            }
        }

        if (token == XML_TOKEN_ATTRIBUTE && is_response_tag) {
            if (xmlIsAttribute(reader, "value"))
                xmlGetAttributeValue(reader, ack, sizeof(ack));

            if (xmlIsAttribute(reader, "MaxPayloadSizeToTargetInBytes"))
                xmlGetAttributeValue(reader, maxpayload, sizeof(maxpayload));
            
            if (ack[0] && maxpayload[0]){
                return  strncasecmp(ack, "ACK", 3)==0?ACK:NAK;
            }
        }
    }
    return NIL;
}

response_t firehose_configure_1st_xml_reader(xml_reader_t *reader, char *maxpayload_support, int sz)
{
    char ack[128] = {0};
    bool is_response_tag = FALSE;
    xml_token_t token;
    while ((token = xmlGetToken(reader)) != XML_TOKEN_NONE) {
        if(token == XML_TOKEN_TAG){
            bzero(ack, sizeof(ack));
            bzero(maxpayload_support, sz);
            is_response_tag = xmlIsTag(reader, "response");
            if(is_response_tag){
                bzero(ack, sizeof(ack));
            }
        }

        if (token == XML_TOKEN_ATTRIBUTE && is_response_tag) {
            if (xmlIsAttribute(reader, "value"))
                xmlGetAttributeValue(reader, ack, sizeof(ack));

            if (xmlIsAttribute(reader, "MaxPayloadSizeToTargetInBytes"))
                xmlGetAttributeValue(reader, maxpayload_support, sz);
            
            if (ack[0] && maxpayload_support[0]){
                return  strncasecmp(ack, "ACK", 3)==0?ACK:NAK;
            }
        }
    }
    return NIL;
}

response_t _firehose_configure_1st_response(parse_xml_reader_func func, char *maxpayload_support, int sz)
{
    int r, status;
    int retry = MAX_RETRY;
    size_t len = 0;  
    char *buf = respbuf_ref(&len);
    char *ptr = buf;
    response_t response;

    while(retry>0){
        r = 0;
        status = read_response(ptr, len-(ptr-buf), &r);
        if (status >=0 && r > 0){ 
            ptr += r;
        }else{
            if(ptr>respbuf){
                xml_reader_t reader;
                xmlInitReader(&reader, buf, ptr-buf);
                response = func(&reader, maxpayload_support, sz);
                if(response != NIL){
                    return response;
                }   
                retry --; 
                continue;
            }   
        }   
        retry --; 
    }   
    return NIL;
}

response_t transmit_chunk_response()
{
    return _response(transmit_response_xml_reader);
}

response_t firehose_configure_response_1st(char *maxpayload_support, int sz)
{
    return _firehose_configure_1st_response(firehose_configure_1st_xml_reader, maxpayload_support, sz);
}

response_t firehose_configure_response_2nd()
{
    return _response(firehose_configure_xml_reader);
}

void send_firehose_configure(firehose_configure_t c)
{
    send_command(c.xml, sizeof(c.xml));
}

response_t process_firehose_configure()
{
    response_t resp;
    char *format = XML_HEADER
                   "<data><configure "
                   "MaxPayloadSizeToTargetInBytes=\"%s\" "
                   "verbose=\""FIREHOSE_VERBOSE"\" "
                   "ZlpAwareHost=\"0\" "
                   "/></data>";
    char maxpayload[10] = "4096";
    char maxpayload_support[10] = {0};
    firehose_configure_t configure;

    sprintf(configure.xml, format, maxpayload);
    send_firehose_configure(configure);
    resp = firehose_configure_response_1st(maxpayload_support, sizeof(maxpayload_support));
    if(resp != ACK)
        return resp;

    memset(&configure, 0, sizeof(configure));
    sprintf(configure.xml, format, maxpayload_support);
    send_firehose_configure(configure);
    return firehose_configure_response_2nd();
}


void parse_program_xml(char *xml,
                       size_t length,
                       char *file_sector_offset,
                       char *sector_size,
                       char *sector_numbers,
                       char *start_sector,
                       char *filename,
                       char *sparse)
{
    char buf[4096] = {0};
    memcpy(buf, xml, length);
    xml_reader_t reader;
    xml_token_t token;
    xmlInitReader(&reader, buf, length);

    while ((token = xmlGetToken(&reader)) != XML_TOKEN_NONE) {
        if (token == XML_TOKEN_ATTRIBUTE) {
            if (xmlIsAttribute(&reader, "SECTOR_SIZE_IN_BYTES")){
                xmlGetAttributeValue(&reader, sector_size, 128);
                continue;
            }
            if (xmlIsAttribute(&reader, "file_sector_offset")){
                xmlGetAttributeValue(&reader, file_sector_offset, 128);
                continue;
            }
            if (xmlIsAttribute(&reader, "num_partition_sectors")){
                xmlGetAttributeValue(&reader, sector_numbers, 128);
                continue;
            }
            if (xmlIsAttribute(&reader, "start_sector")){
                xmlGetAttributeValue(&reader, start_sector, 128);
                continue;
            }
            if (xmlIsAttribute(&reader, "filename")){
                xmlGetAttributeValue(&reader, filename, 128);
                continue;
            }
            if (xmlIsAttribute(&reader, "sparse")){
                xmlGetAttributeValue(&reader, sparse, sizeof(128);
                continue;
            }
        }
    }
}

void generate_patch_xml(char patch[4096], char *line)
{
    bzero(patch, 4096);
    sprintf(patch, "%s", XML_HEADER);
    strcat(patch, "<data>");
    strncat(patch, line, strlen(line));
    strcat(patch, "</data>");
}
                        
void generate_program_xml(char xml[4096], program_t p)
{
    char buf[1024] = {0};
    char *template= "<data> <program "
                    "SECTOR_SIZE_IN_BYTES=\"%s\" "
                    "num_partition_sectors=\"%s\" "
                    "physical_partition_number=\"%s\" "
                    "start_sector=\"%s\" "
                    "/></data>";
    sprintf(buf, template, sector_size, sector_numbers,
                physical_partition_number, start_sector);
    strncpy(xml, XML_HEADER, strlen(XML_HEADER));
    strncat(xml, buf, strlen(buf));
}

response_t power_action(char *act)
{
    char cmd[128] = {0};
    char *format = "<?xml version=\"1.0\" ?>"
                   "<data><power value=\"%s\" delayinseconds=\"2\" /></data>";
    sprintf(cmd, format, act);
    send_command(cmd, strlen(cmd));
    return power_response();
}

void parse_patch_xml(char *xml,
                     size_t length,
                     char *what,
                     char *filename)
{
    char buf[4096] = {0};
    memcpy(buf, xml, length);
    xml_reader_t reader;
    xml_token_t token;
    xmlInitReader(&reader, buf, length);

    while ((token = xmlGetToken(&reader)) != XML_TOKEN_NONE) {
        if (token == XML_TOKEN_ATTRIBUTE) {
            if (xmlIsAttribute(&reader, "filename")){
                xmlGetAttributeValue(&reader, finalname, 128);
            }

            if (xmlIsAttribute(&reader, "what")){
                xmlGetAttributeValue(&reader, what, 128);
            }
        }
    }
}

response_t common_response_xml_reader(xml_reader_t reader)
{
    xml_token_t token;
    char value[128] = {0};
    int is_response_tag = 0;
    while ((token = xmlGetToken(&reader)) != XML_TOKEN_NONE) {
        if(token == XML_TOKEN_TAG)
            is_response_tag = xmlIsTag(&reader, "response");
        if (token == XML_TOKEN_ATTRIBUTE && is_response_tag){
            if (xmlIsAttribute(&reader, "Value")){
                xmlGetAttributeValue(&reader, value, sizeof(value));
                return (strncasecmp("ACK", value, 3)==0)?ACK:NAK;
            }
        }
    }
    return NIL;
}


int init_configure(xml_reader_t *reader, configure_t *c)
{
    
}

response_t firehose_configure_xml(size_t *payload)
{
    int r = 0, w = 0;
    int loop;
    char cmd[4096] = {0};
    char ack[128] = {0};
    char *format = "<?xml version=\"1.0\" ?><data><configure "
                   "MaxPayloadSizeToTargetInBytes=\"%s\" "
                   "verbose=\""FIREHOSE_VERBOSE"\" "
                   "ZlpAwareHost=\"0\" "
                   "/></data>";
    char MaxPayloadSizeToTargetInBytes[10] = "4096";
    char MaxPayloadSizeToTargetInBytesSupported[10] = {0};
    xml_reader_t reader;
    xml_token_t token;
    bool is_response_tag = FALSE;

    sprintf(cmd, format, MaxPayloadSizeToTargetInBytes);
    send_command(cmd, strlen(cmd));

    loop = 10;
    while(loop--){
        is_response_tag = FALSE;
        int stop = 0;
        r = 0;
        bzero(respbuf, MAX_RESP_LENGTH);
        read_response(respbuf, MAX_RESP_LENGTH, &r);
        xmlInitReader(&reader, respbuf, r);

        while ((token = xmlGetToken(&reader)) != XML_TOKEN_NONE) {
            if(token == XML_TOKEN_TAG){
                is_response_tag = xmlIsTag(&reader, "response");
                if(is_response_tag){
                    bzero(ack, sizeof(ack));
                    bzero(MaxPayloadSizeToTargetInBytesSupported,
                          sizeof(MaxPayloadSizeToTargetInBytesSupported));
                }
            }
            if (token == XML_TOKEN_ATTRIBUTE && is_response_tag) {
                if (xmlIsAttribute(&reader, "value"))
                    xmlGetAttributeValue(&reader, ack, sizeof(ack));

                if (xmlIsAttribute(&reader, "MaxPayloadSizeToTargetInBytesSupported")){
                    xmlGetAttributeValue(&reader,
                                         MaxPayloadSizeToTargetInBytesSupported,
                                         sizeof(MaxPayloadSizeToTargetInBytesSupported));
                    if (ack[0] && MaxPayloadSizeToTargetInBytesSupported[0]){
                        stop = 1;
                        break;
                    }
                }
            }
        }
        if (stop)
            break;
    }

    if (!MaxPayloadSizeToTargetInBytesSupported[0]){
        LOG("Get MaxPayloadSizeToTargetInBytesSupported failed");
    }/*
    else{
        LOG("Got MaxPayloadSizeToTargetInBytesSupported %s",
            MaxPayloadSizeToTargetInBytesSupported);
    }
    */

    bzero(ack, sizeof(ack));
    bzero(cmd, sizeof(cmd));

    sprintf(cmd, format, MaxPayloadSizeToTargetInBytesSupported);
    send_command(cmd, strlen(cmd));

    loop = 10;
    while(loop--){
        is_response_tag = FALSE;
        read_response(respbuf, MAX_RESP_LENGTH, &r); 
        xmlInitReader(&reader, respbuf, r);

        while ((token = xmlGetToken(&reader)) != XML_TOKEN_NONE) {
            if(token == XML_TOKEN_TAG){
                bzero(ack, sizeof(ack));
                bzero(MaxPayloadSizeToTargetInBytes, sizeof(MaxPayloadSizeToTargetInBytes));
                is_response_tag = xmlIsTag(&reader, "response");
            }

            if (token == XML_TOKEN_ATTRIBUTE && is_response_tag) {
                if (xmlIsAttribute(&reader, "value"))
                    xmlGetAttributeValue(&reader, ack, sizeof(ack));
                if (xmlIsAttribute(&reader, "MaxPayloadSizeToTargetInBytes")){
                    xmlGetAttributeValue(&reader,
                                         MaxPayloadSizeToTargetInBytes,
                                         sizeof(MaxPayloadSizeToTargetInBytes));
                    if (ack[0] && MaxPayloadSizeToTargetInBytes[0]){
                        *payload = atoll(MaxPayloadSizeToTargetInBytes);
                        return  strncasecmp(ack, "ACK", 3)==0?ACK:NAK;
                    }
                }
            }
        }
    }
    return NIL;
}

response_t configure_response()
{
}


int init_erase(xml_reader_t *reader, firehose_erase_t *e)
{
    xml_token_t token;
    while ((token = xmlGetToken(reader)) != XML_TOKEN_NONE) {
        if (token == XML_TOKEN_ATTRIBUTE) {
            if (xmlIsAttribute(reader, "num_partition_sectors")){
                xmlGetAttributeValue(reader, e->sector_numbers, 128);
            }
            if (xmlIsAttribute(reader, "start_sector")){
                xmlGetAttributeValue(reader, e->start_sector, 128);
            }
            if (xmlIsAttribute(reader, "label")){
                xmlGetAttributeValue(reader, e->label, 128);
            }
            if (xmlIsAttribute(reader, "storagedrive")) {
                xmlGetAttributeValue(reader, e->storagedrive, 128); 
            }
        }
    }
   
    char *format= XML_HEADER
                  "<data><erase "
                  "start_sector=\"%s\" "
                  "num_partition_sectors=\"%s\" "
                  "storagedrive=\"%s\" "
                  "/></data>";
    
    bzero(e->xml, sizeof(e->xml));
    sprintf(e->xml, format, e->start_sector, e->sector_numbers, e->storagedrive);
}

int init_program_from_xml_reader(xml_reader_t *reader, program_t *p) //FOR NUM_DISK_SECTORS
{
    xml_token_t token;
    while ((token = xmlGetToken(reader)) != XML_TOKEN_NONE) {
        if (token == XML_TOKEN_ATTRIBUTE) {
            if (xmlIsAttribute(reader, "SECTOR_SIZE_IN_BYTES")){
                xmlGetAttributeValue(reader, p->sector_size_s, 128);
                p->sector_size = strtoint(p->sector_size_s);
                continue;
            }
            if (xmlIsAttribute(reader, "file_sector_offset")){
                xmlGetAttributeValue(reader, p->file_sector_offset_s, 128);
                p->file_sector_offset = strtoint(p->file_sector_offset_s);  
                continue;
            }
            if (xmlIsAttribute(reader, "num_partition_sectors")){
                xmlGetAttributeValue(reader, p->sector_numbers_s, 128);
                p->sector_numbers = strtoint(p->sector_numbers_s);
                continue;
            }
            if (xmlIsAttribute(reader, "start_sector")){
                xmlGetAttributeValue(reader,p->start_sector_s, 128);
                p->start_sector = strtoint(p->start_sector_s);
                continue;
            }
            if (xmlIsAttribute(reader, "filename")){
                xmlGetAttributeValue(reader, filename, 256);
                continue;
            }
            if (xmlIsAttribute(reader, "sparse")){
                xmlGetAttributeValue(reader, p->sparse_s, sizeof(128);
                if(!strcasecmp(p->sparse_s, "true"))
                    p->sparse = True;
                if(!strcasecmp(p->sparse_s, "false"))
                    p->sparse = False;
                continue;
            }
        }
    }

    char *template= XML_HEADER
                    "<data> <program "
                    "SECTOR_SIZE_IN_BYTES=\"%zu\" "
                    "num_partition_sectors=\"%zu\" "
                    "physical_partition_number=\"%zu\" "
                    "start_sector=\"%s\" "
                    "/></data>";
    memset(p->xml, 0, sizeof(p->xml));
    sprintf(p->xml, template, sector_size, sector_numbers,
                physical_partition_number, p->start_sector_s);
}



void send_erase(firehose_erase_t e);
{
    send_command(e.xml, strlen(e.xml));
}

response_t erase_response()
{
    return common_response();
}

reponse_t process_erase_xml(char *xml, int length)
{
    firehose_erase_t erase;
    xml_reader_t reader; 
    xmlInitReader(&reader, xml, length);
    init_erase(&reader, &erase);
    send_erase(erase);
    return erase_response();
}

size_t strtoint(char *s)
{
    return strtoull(s, 0, 0);
}

int init_program(char *xml, int length, program_t *p)
{
    assert(length<=sizeof(p->xml)); 
    memcpy(p->xml, xml, length);
    xml_reader_t reader;
    xml_token_t token;
    xmlInitReader(&reader, p->xml, length);

    while ((token = xmlGetToken(&reader)) != XML_TOKEN_NONE) {
        if (token == XML_TOKEN_ATTRIBUTE) {
            if (xmlIsAttribute(&reader, "SECTOR_SIZE_IN_BYTES")){
                xmlGetAttributeValue(&reader, p->sector_size_s, 128);
                p->sector_size = strtoint(p->sector_size_s);
                continue;
            }
            if (xmlIsAttribute(&reader, "file_sector_offset")){
                xmlGetAttributeValue(&reader, p->file_sector_offset_s, 128);
                p->file_sector_offset = strtoint(p->file_sector_offset_s);  
                continue;
            }
            if (xmlIsAttribute(&reader, "num_partition_sectors")){
                xmlGetAttributeValue(&reader, p->sector_numbers_s, 128);
                p->sector_numbers = strtoint(p->sector_numbers_s);
                continue;
            }
            if (xmlIsAttribute(&reader, "start_sector")){
                xmlGetAttributeValue(&reader,p->start_sector_s, 128);
                p->start_sector = strtoint(p->start_sector_s);
                continue;
            }
            if (xmlIsAttribute(&reader, "filename")){
                xmlGetAttributeValue(&reader, filename, 256);
                continue;
            }
            if (xmlIsAttribute(&reader, "sparse")){
                xmlGetAttributeValue(&reader, p->sparse_s, sizeof(128);
                if(!strcasecmp(p->sparse_s, "true"))
                    p->sparse = True;
                if(!strcasecmp(p->sparse_s, "false"))
                    p->sparse = False;
                continue;
            }
        }
    }
    return 0;
}

void send_program(program_t p)
{
    clear_rubbish();
    return send_command(p.xml, strlen(p.xml));
}

response_t program_response()
{
    return  _response(program_response_xml_reader);
}

response_t program_response_xml_reader(xml_reader_t *reader)
{
    xml_token_t token;
    char ack[128] = {0};
    char rawmode[128] = {0};
    bool is_response_tag = FALSE;
    while ((token = xmlGetToken(reader)) != XML_TOKEN_NONE) {
        if(token == XML_TOKEN_TAG){
            is_response_tag = xmlIsTag(reader, "response");
            if(is_response_tag){
                bzero(ack, sizeof(ack));
                bzero(rawmode, sizeof(rawmode));
            }
        }
        if (token == XML_TOKEN_ATTRIBUTE && is_response_tag) {
            if (xmlIsAttribute(reader, "value"))
                xmlGetAttributeValue(reader, ack, sizeof(ack));
            if (xmlIsAttribute(reader, "rawmode"))
                xmlGetAttributeValue(reader, rawmode, sizeof(rawmode));
            if (ack[0] && rawmode[0]){
                return (strncasecmp(ack, "ACK", 3)==0
                        && strncasecmp(rawmode, "true", 4)==0)?ACK:NAK;
            }
        }
    }
    return NIL;
}

void generate_program_xml(char xml[4096], program_t p)
{
    char buf[1024] = {0};
    char *template= "<data> <program "
                    "SECTOR_SIZE_IN_BYTES=\"%s\" "
                    "num_partition_sectors=\"%s\" "
                    "physical_partition_number=\"%s\" "
                    "start_sector=\"%s\" "
                    "/></data>";
    sprintf(buf, template, sector_size, sector_numbers,
                physical_partition_number, start_sector);
    strncpy(xml, xml_header, strlen(xml_header));
    strncat(xml, buf, strlen(buf));
}

response_t transmit_chunk(char *chunk, program_t p)
{
    int w=0, status;
    int payload = 16*1024; //16K
    size_t total_size = p->sector_size*p->sector_numbers;
    char *ptr = chunk;
    char *end = chunk + total_size;
    response_t response;

    clear_rubbish();
    send_program(p);
    response = program_response();
    if (response == NAK)
        xerror("NAK program response");
    if (response == NIL)
        xerror("no ACK or NAK found in response");

    clear_rubbish();
    while(ptr < end){
        if(end - ptr < payload){
            status = send_data(ptr, end-ptr, &w);
            if((status < 0) || (w != end-ptr)){
                xerror("failed, status: %d  w: %d", status, w); 
            }   
        }else{
            status = send_data(ptr, payload, &w);
            if((status < 0) || (w != payload)){
                info("failed, status: %d  w: %d", status, w); 
                return NAK;
            }   
        }   
        ptr += w;
        printf("\r %zu / %zu    ", ptr-chunk, total_size); fflush (stdout);
    }   
    response = transmit_chunk_response();
    if (response == ACK)
        info("  succeeded");
    else
        info("  failed");
    return response;
}
