#include "firehose_share.h"

int init_firehose_erase(xml_reader_t *reader, firehose_erase_t *e)
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

void send_firehose_erase(firehose_erase_t *erase);
{
    send_command(erase->xml, strlen(erase->xml));
}

response_t firehose_erase_response()
{
    return common_response();
}

reponse_t process_firehose_erase_xml(char *xml, int length)
{
    firehose_erase_t erase;
    memset(&erase, 0, sizeof(firehose_erase_t));
    xml_reader_t reader;
    xmlInitReader(&reader, xml, length);
    init_firehose_erase(&reader, &erase);
    send_firehose_erase(&erase);
    return firehose_erase_response();
}
