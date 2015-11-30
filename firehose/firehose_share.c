#ifndef FIREHOSE_SHARE_H
#define FIREHOSE_SHARE_H

#define MAX_RESP_LENGTH MAX_LENGTH
char respbuf[MAX_RESP_LENGTH] = {0};
char rubbish[MAX_LENGTH] = {0};
char bigchunk[MAX_LENGTH*200] = {0};

char *respbuf_ref(size_t *len)
{
    if(len)
        *len = sizeof(respbuf);
    memset(respbuf, 0, sizeof(respbuf));
    return respbuf;
}

int clear_rubbish()
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

size_t strtoint(char *s)
{
    return strtoull(s, 0, 0);
}

#endif
