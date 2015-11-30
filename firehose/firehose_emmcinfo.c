#include "firehose_emmcinfo.h"

response_t firehose_emmc_info_response_xml_reader(xml_reader_t *reader)
{
    xml_token_t token;
    char ack[128] = {0};
    char value[128] = {0};
    int is_response_tag = 0;
    int is_emmc_size_log_tag = 0;
    while ((token = xmlGetToken(reader)) != XML_TOKEN_NONE) {
        if(token == XML_TOKEN_TAG){
            is_emmc_size_log_tag = xmlIsTag(reader, "log");
            if (is_emmc_size_log_tag)
                is_response_tag = 0;
            else{
                is_response_tag = xmlIsTag(reader, "response");
                if (is_response_tag)
                    is_emmc_size_log_tag = 0;
            }
        }
        if (token == XML_TOKEN_ATTRIBUTE && is_response_tag){
            if (xmlIsAttribute(reader, "Value")){
                xmlGetAttributeValue(reader, ack, sizeof(ack));
            }
        }
        if (token == XML_TOKEN_ATTRIBUTE && is_emmc_size_log_tag){
            if (xmlIsAttribute(reader, "Value")){
                xmlGetAttributeValue(reader, value, sizeof(value));
                if (strcasestr(value, "eMMC size=")){
                    sscanf(value, "eMMC size=%zu", &num_disk_sectors);
                }
            }
        }

        if(ack[0]){
            if (strncasecmp("ACK", ack, 3)==0 && num_disk_sectors > 0)
                return ACK;
            if (strncasecmp("ACK", ack, 3)){
                return NAK
            }
        }
    }
    return NIL
}

response_t firehose_emmc_info_reponse()
{
    return _response(firehose_emmc_info_response_xml_reader);
}

response_t firehose_emmc_info()
{
    char *cmd = "<?xml version=\"1.0\" ?><data><eMMCinfo /></data>";
    send_command(cmd, strlen(cmd));
    return firehose_emmc_info_reponse();
}
