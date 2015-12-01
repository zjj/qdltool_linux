#include "firehose_configure.h"

static size_t strtoint(char *s)
{
    return strtoull(s, 0, 0);
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
        if (r > 0){
            ptr += r;
        }
        if (status < 0){
            if(ptr>respbuf){
                xml_reader_t reader;
                xmlInitReader(&reader, buf, ptr-buf);
                response = func(&reader, maxpayload_support, sz);
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

response_t firehose_configure_response_1st(char *maxpayload_support, int sz)
{
    return _firehose_configure_1st_response(firehose_configure_1st_xml_reader, maxpayload_support, sz);
}

response_t firehose_configure_response_2nd()
{
    return commond_response();
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
