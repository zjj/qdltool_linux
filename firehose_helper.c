#include "firehose.h"
#include "firehose_help.h"

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

response_t common_response_xml_reader(xml_reader_t *reader)
{
    xml_token_t token;
    char value[128] = {0};
    int is_response_tag = 0;
    while ((token = xmlGetToken(reader)) != XML_TOKEN_NONE) {
        if(token == XML_TOKEN_TAG)
            is_response_tag = xmlIsTag(reader, "response");
        if (token == XML_TOKEN_ATTRIBUTE && is_response_tag){
            if (xmlIsAttribute(reader, "Value")){
                xmlGetAttributeValue(reader, value, sizeof(value));
                return (strncasecmp("ACK", value, 3)==0)?ACK:NAK;
            }
        }
    }
    return NIL;
}


void init_program_from_xml_reader(xml_reader_t *reader, program_t *p)
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
}

void send_program(program_t p)
{
    clear_rubbish();
    return send_command(p.xml, strlen(p.xml));
}

response_t _response(parse_xml_reader_func func)
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
                response = func(reader);
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

