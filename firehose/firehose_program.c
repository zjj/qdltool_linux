#include "firehose_program.h"

void update_xml_of_firehose_progarm(firehose_program_t *p)
{
    memset(p->xml, 0, sizeof(p->xml));
    char *template= XML_HEADER
                    "<data> <program "
                    "SECTOR_SIZE_IN_BYTES=\"%zu\" "
                    "num_partition_sectors=\"%zu\" "
                    "physical_partition_number=\"%zu\" "
                    "start_sector=\"%zu\" "
                    "/></data>";
    sprintf(p->xml, template, p->sector_size, p->sector_numbers,
                p->physical_partition_number, p->start_sector);
}

int init_firehose_program_from_xml_reader(xml_reader_t *reader, firehose_program_t *program)
{
    xml_token_t token;
    while ((token = xmlGetToken(reader)) != XML_TOKEN_NONE) {
        char tempbuf[256] = {0};
        if (token == XML_TOKEN_ATTRIBUTE) {
            if (xmlIsAttribute(reader, "SECTOR_SIZE_IN_BYTES")){
                xmlGetAttributeValue(reader, tempbuf, sizeof(tempbuf));
                program->sector_size = firehose_strtoint(tempbuf);
                continue;
            }
            if (xmlIsAttribute(reader, "file_sector_offset")){
                xmlGetAttributeValue(reader, tempbuf, sizeof(tempbuf));
                program->file_sector_offset = firehose_strtoint(tempbuf);
                continue;
            }
            if (xmlIsAttribute(reader, "num_partition_sectors")){
                xmlGetAttributeValue(reader, tempbuf, sizeof(tempbuf));
                program->sector_numbers = firehose_strtoint(tempbuf);
                continue;
            }
            if (xmlIsAttribute(reader, "physical_partition_number")){
                xmlGetAttributeValue(reader, tempbuf, sizeof(tempbuf));
                program->physical_partition_number = firehose_strtoint(tempbuf);
                continue;
            }
            if (xmlIsAttribute(reader, "start_sector")){
                xmlGetAttributeValue(reader, tempbuf, sizeof(tempbuf));
                program->start_sector = firehose_strtoint(tempbuf);
                continue;
            }
            if (xmlIsAttribute(reader, "filename")){
                xmlGetAttributeValue(reader, program->filename, sizeof(program->filename));
                continue;
            }
            if (xmlIsAttribute(reader, "sparse")){
                xmlGetAttributeValue(reader, tempbuf, sizeof(tempbuf));
                if(!strcasecmp(tempbuf, "true"))
                    program->sparse = True;
                if(!strcasecmp(tempbuf, "false"))
                    program->sparse = False;
                continue;
            }
        }
    }
    update_xml_of_firehose_progarm(program);
}

int send_program(firehose_program_t p)
{
    clear_rubbish();
    return send_command(p.xml, strlen(p.xml));
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

response_t program_response()
{
    return  _response(program_response_xml_reader);
}

response_t transmit_chunk_response_xml_reader(xml_reader_t *reader)
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

response_t transmit_chunk_response()
{
    return _response(transmit_chunk_response_xml_reader);
}

response_t transmit_chunk(char *chunk, firehose_program_t p)
{
    int w=0, status;
    int payload = 16*1024; //16K
    size_t total_size = p.sector_size*p.sector_numbers;
    char *ptr = chunk;
    char *end = chunk + total_size;
    response_t response;
    update_xml_of_firehose_progarm(&p); //actually, this shall have been done
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
