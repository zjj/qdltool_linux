#include "firehose_patch.h"

response_t firehose_patch_response()
{
    return common_response();
}

void init_firehose_patch(xml_reader_t *reader, firehose_patch_t *patch)
{
    bzero(patch->xml, sizeof(patch->xml));
    char *template = XML_HEADER
                     "<data>"
                     "%s"
                     "</data>";
    sprintf(patch->xml, template, reader->buffer);

    xml_token_t token;
    while ((token = xmlGetToken(reader)) != XML_TOKEN_NONE) {
        if (token == XML_TOKEN_ATTRIBUTE) {
            if (xmlIsAttribute(reader, "filename")){
                xmlGetAttributeValue(reader, patch->filename, sizeof(patch->filename));
            }

            if (xmlIsAttribute(reader, "what")){
                xmlGetAttributeValue(reader, patch->what, sizeof(patch->what);
            }
        }
    }
}

int send_firehose_patch(firehose_patch_t patch)
{
    return send_command(patch.xml, strlen(patch.xml));
}

responst_t process_firehose_patch(char *xml, int length)
{
    firehose_patch_t patch;
    memset(&patch, 0, sizeof(firehose_patch_t));
    xml_reader_t reader;
    xmlInitReader(&reader, xml, length);
    init_firehose_patch(&reader, &patch);
    send_firehose_patch(patch);
    return firehose_patch_response(); 
}
