#define _GNU_SOURCE
#include "xml_parser.h"
#include "qdl_usb.h"
#include "sahara.h"
#include "firehose.h"
#include "misc.h"

#define send_data write_tx

char xml_header[] = "<\?xml version=\"1.0\" encoding=\"UTF-8\" \?>";
char rubbish[MAX_LENGTH] = {0};

#define MAX_RESP_LENGTH MAX_LENGTH
char respbuf[MAX_RESP_LENGTH] = {0};
/*
use resp_buffer for the response log buffer
it just returns a ptr to response_xml
*/
char *respbuf_ref()
{
    memset(respbuf, 0, sizeof(respbuf));
    return respbuf;
}

static int clear_rubbish()
{
    bzero(rubbish, sizeof(rubbish));
    return read_rx_timeout(rubbish, sizeof(rubbish), NULL, 500); //500ms timeout
}

int read_response(void *buf, int length, int *act)
{
    memset(buf, 0, length);
    return read_rx_timeout(buf, length, act, 500);
}

int send_command(void *buf, int len)
{
    clear_rubbish();
    return write_tx(buf, len, NULL);
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

response_t _response(parse_xml_reader_func func)
{   
    int r, status;
    int timeout = TIMEOUT;
    char *ptr = respbuf_ref();
    xml_reader_t reader;
    response_t response;
     
    while(timeout>0){
        r = 0;
        status = read_response(ptr, MAX_RESP_LENGTH-(ptr-respbuf), &r);
        if(status>=0 && r>0){
            ptr += r;
            xmlInitReader(&reader, respbuf, ptr - respbuf);
            response = func(reader);
            if(response != NIL)
                return response;
        }else{
            timeout--;
            sleep(1);
        }
    }
    return NIL;
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

response_t program_response()
{
    return  _response(program_response_xml_reader);
}

response_t common_response()
{
    return _response(common_response_xml_reader);
}

response_t patch_response()
{
    return common_response();
}

response_t erase_response()
{
    return common_response();
}

response_t power_response()
{
    return common_response();
}

response_t transmit_response_xml_reader(xml_reader_t reader)
{
    xml_token_t token;
    char ack[128] = {0};
    char rawmode[128] = {0};
    bool is_response_tag = FALSE;
    while ((token = xmlGetToken(&reader)) != XML_TOKEN_NONE) {
        if(token == XML_TOKEN_TAG){
            is_response_tag = xmlIsTag(&reader, "response");
            if(is_response_tag){
                bzero(ack, sizeof(ack));
               bzero(rawmode, sizeof(rawmode));
            }
        }
        if (token == XML_TOKEN_ATTRIBUTE && is_response_tag) {
            if (xmlIsAttribute(&reader, "value"))
                xmlGetAttributeValue(&reader, ack, sizeof(ack));

            if (xmlIsAttribute(&reader, "rawmode"))
                xmlGetAttributeValue(&reader, rawmode, sizeof(rawmode));

            if (ack[0] && rawmode[0]){
                return (strncasecmp(ack, "ACK", 3)==0 
                        && strncasecmp(rawmode, "false", 5)==0)?ACK:NAK;
            }
        }
    }
    return NIL;
}

response_t transmit_response()
{   
    int status, r;
    int timeout = TIMEOUT;
    char *ptr = respbuf_ref();
    xml_reader_t reader;
    response_t response;
    
    while(timeout>0){
        r = 0;
        status = read_response(ptr, MAX_RESP_LENGTH-(ptr-respbuf), &r);
        //LOG("%s", respbuf);
        if(status>=0 && r>0){
            ptr += r;
            xmlInitReader(&reader, respbuf, ptr - respbuf);
            response = transmit_response_xml_reader(reader);
            if(response != NIL)
                return response;
        }else{
            timeout--;
            sleep(1);
        }
    }
    return NIL;
}

response_t transmit_file_response()
{
    return transmit_response();
}

response_t firehose_configure(size_t *payload)
{
    int r = 0, w = 0;
    int loop;
    char cmd[4096] = {0};
    char ack[128] = {0};
    char *format = "<?xml version=\"1.0\" ?><data><configure "
                   "MaxPayloadSizeToTargetInBytes=\"%s\" "
                   "verbose=\""FIREHOSE_VERBOSE"\" "
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

response_t program_response_xml_reader(xml_reader_t reader)
{
    xml_token_t token;
    char ack[128] = {0};
    char rawmode[128] = {0};
    bool is_response_tag = FALSE;
    while ((token = xmlGetToken(&reader)) != XML_TOKEN_NONE) {
        if(token == XML_TOKEN_TAG){
            is_response_tag = xmlIsTag(&reader, "response");
            if(is_response_tag){
                bzero(ack, sizeof(ack));
                bzero(rawmode, sizeof(rawmode));
            }
        }
        if (token == XML_TOKEN_ATTRIBUTE && is_response_tag) {
            if (xmlIsAttribute(&reader, "value"))
                xmlGetAttributeValue(&reader, ack, sizeof(ack));
            if (xmlIsAttribute(&reader, "rawmode"))
                xmlGetAttributeValue(&reader, rawmode, sizeof(rawmode));
            if (ack[0] && rawmode[0]){
                return (strncasecmp(ack, "ACK", 3)==0
                        && strncasecmp(rawmode, "true", 4)==0)?ACK:NAK;
            }
        }
    }
    return NIL;
}

void parse_program_xml(char *xml,
                       size_t length,
                       size_t *file_sector_offset,
                       size_t *sector_size,
                       size_t *sector_numbers,
                       char *start_sector,
                       char *filename)
{
    char buf[4096] = {0};
    char tempbuf[128] = {0};
    memcpy(buf, xml, length);
    xml_reader_t reader;
    xml_token_t token;
    xmlInitReader(&reader, buf, length);

    while ((token = xmlGetToken(&reader)) != XML_TOKEN_NONE) {
        if (token == XML_TOKEN_ATTRIBUTE) {
            if (xmlIsAttribute(&reader, "SECTOR_SIZE_IN_BYTES")){
                xmlGetAttributeValue(&reader, tempbuf, sizeof(tempbuf));
                *sector_size = atoll(tempbuf);
                bzero(tempbuf, sizeof(tempbuf));
            }
            if (xmlIsAttribute(&reader, "file_sector_offset")){
                xmlGetAttributeValue(&reader, tempbuf, sizeof(tempbuf));
                *file_sector_offset = atoll(tempbuf);
                bzero(tempbuf, sizeof(tempbuf));
            }
            if (xmlIsAttribute(&reader, "num_partition_sectors")){
                xmlGetAttributeValue(&reader, tempbuf, sizeof(tempbuf));
                *sector_numbers = atoll(tempbuf);
                bzero(tempbuf, sizeof(tempbuf));
            }
            if (xmlIsAttribute(&reader, "start_sector")){
                xmlGetAttributeValue(&reader, tempbuf, sizeof(tempbuf));
                strncpy(start_sector, tempbuf, strlen(tempbuf));
                bzero(tempbuf, sizeof(tempbuf));
            }
            if (xmlIsAttribute(&reader, "filename")){
                xmlGetAttributeValue(&reader, filename, 128);
            }
        }
    }
}

void parse_erase_xml(char *xml,
                     size_t length,
                     char *label,
                     char *start_sector,
                     size_t *sector_numbers,
                     char *physical_partition_number)
{
    char buf[4096] = {0};
    char tempbuf[128] = {0};
    memcpy(buf, xml, length);
    xml_reader_t reader;
    xml_token_t token;
    xmlInitReader(&reader, buf, length);

    while ((token = xmlGetToken(&reader)) != XML_TOKEN_NONE) {
        if (token == XML_TOKEN_ATTRIBUTE) {
            if (xmlIsAttribute(&reader, "num_partition_sectors")){
                xmlGetAttributeValue(&reader, tempbuf, sizeof(tempbuf));
                *sector_numbers = atoll(tempbuf);
                bzero(tempbuf, sizeof(tempbuf));
            }
            if (xmlIsAttribute(&reader, "start_sector")){
                xmlGetAttributeValue(&reader, tempbuf, sizeof(tempbuf));
                strncpy(start_sector, tempbuf, strlen(tempbuf));
                bzero(tempbuf, sizeof(tempbuf));
            }
            if (xmlIsAttribute(&reader, "label")){
                xmlGetAttributeValue(&reader, tempbuf, sizeof(tempbuf));
                strncpy(label, tempbuf, strlen(tempbuf));
                bzero(tempbuf, sizeof(tempbuf));
            }
            if (xmlIsAttribute(&reader, "storagedrive")) {
                xmlGetAttributeValue(&reader, tempbuf, sizeof(tempbuf));
                *physical_partition_number = atoll(tempbuf);
                bzero(tempbuf, sizeof(tempbuf));
            }
        }
    }
}

void generate_patch_xml(char *patch, char *line)
{
    bzero(patch, 4096);
    sprintf(patch, "%s", xml_header);
    strcat(patch, "<data>");
    strncat(patch, line, strlen(line));
    strcat(patch, "</data>");
}
                        
void generate_program_xml(char xml[4096],
                          size_t sector_size,
                          size_t sector_numbers,
                          char *start_sector,
                          u8 physical_partition_number)
{
    char buf[1024] = {0};
    char *template= "<data> <program "
                    "SECTOR_SIZE_IN_BYTES=\"%zu\" "
                    "num_partition_sectors=\"%zu\" "
                    "physical_partition_number=\"%d\" "
                    "start_sector=\"%s\" "
                    "/></data>";
    sprintf(buf, template, sector_size, sector_numbers,
                physical_partition_number, start_sector);
    strncpy(xml, xml_header, strlen(xml_header));
    strncat(xml, buf, strlen(buf));
}

response_t transmit_file(int fd,
                   size_t sector_numbers,
                   size_t sector_size,
                   char *start_sector,
                   u8 physical_partition_number)
{
    size_t sent;
    int r, w, status;
    response_t response;
    char program_xml[4096] = {0};
    int payload = 16*1024; //16K
    char *packet = (char *)malloc(payload);
    bzero(packet, payload); 
    clear_rubbish();

    generate_program_xml(program_xml,
                         sector_size,
                         sector_numbers,
                         start_sector,
                         physical_partition_number);

    send_program_xml(program_xml, strlen(program_xml));
    response = program_response();

    if (response == NAK)
        xerror("NAK program response");
    if (response == NIL)
        xerror("no ACK or NAK found in response");

    status = send_data(packet, 0, &w);//flush function?
    sent = w = 0;
    clear_rubbish();
    while((r=read(fd, packet, payload))>0){
        status = send_data(packet, ((r + sector_size - 1)/sector_size*sector_size), &w);
        if((status < 0) || (w != (r + sector_size - 1)/sector_size*sector_size)){
            xerror("failed, status: %d  w: %d", status, w);
            return NAK;
        }
        memset(packet, 0, payload);
        sent += w;
        printf("\r %zu / %zu    ", sent, sector_numbers*sector_size); fflush (stdout);
    }
    status = send_data(packet, 0, &w);//flush function?
    free(packet);
    packet = NULL;
    response = transmit_file_response();
    if (response == ACK)
        info("  succeeded");
    else
        info("transmit failed");
    return response;
}

response_t partition_erase(char *start_sector,
                           char sector_numbers,
                           u8 physical_partition_number)
{
    char cmd[4096] = {0};
    char *format= "<\?xml version=\"1.0\" "
                         "encoding=\"UTF-8\" \?><data>"
                         "<erase "
                         "start_sector=\"%s\" "
                         "num_partition_sectors=\"%zu\" "
                         "StorageDrive=\"%d\" "
                         "/></data>";
    sprintf(cmd, format, start_sector, sector_numbers, physical_partition_number);
    send_command(cmd, strlen(cmd));
    return erase_response();
}

response_t power_action(char *act)
{
    char cmd[128] = {0};
    char *format = "<?xml version=\"1.0\" ?>"
                   "<data><power value=\"%s\" /></data>";
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
    char tempbuf[128] = {0};
    memcpy(buf, xml, length);
    xml_reader_t reader;
    xml_token_t token;
    xmlInitReader(&reader, buf, length);

    while ((token = xmlGetToken(&reader)) != XML_TOKEN_NONE) {
        if (token == XML_TOKEN_ATTRIBUTE) {
            if (xmlIsAttribute(&reader, "filename")){
                xmlGetAttributeValue(&reader, tempbuf, sizeof(tempbuf));
                strncpy(filename, tempbuf, strlen(tempbuf));
                bzero(tempbuf, sizeof(tempbuf));
            }

            if (xmlIsAttribute(&reader, "what")){
                xmlGetAttributeValue(&reader, tempbuf, sizeof(tempbuf));
                strncpy(what, tempbuf, strlen(tempbuf));
                bzero(tempbuf, sizeof(tempbuf));
            }
        }
    }
}
