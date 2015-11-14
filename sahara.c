#include "sahara.h"
#include "qdl_usb.h"

void hexdump(void *buf, int length)
{
    u8 *ptr = (u8 *)buf;
    int i = 0;
    for(i=0; i<length; i++)
        printf("%02x ", *(ptr+i));
    printf("\n");
}

int sahara_handle_packet_hello()
{
    int status = 0;
    struct sahara_packet_hello packet_hello;
    int length = 0;
    read_rx(&packet_hello, sizeof(packet_hello), &length);
    do
    {
        if ((length != sizeof(struct sahara_packet_hello))||
            (length != packet_hello.length)){
            status = -10;
            break;
        }
        if (packet_hello.command != SAHARA_HELLO_ID){
            status = -11;
            break;
        }
        if (packet_hello.mode != SAHARA_MODE_IMAGE_TX_PENDING){
            status = -12;
            break;
        }
    } while(0);
    return status;
}

int sahara_send_packet_hello_resp()
{
    int status = 0;
    struct sahara_packet_hello_resp packet_hello_resp;
    packet_hello_resp.command = SAHARA_HELLO_RESP_ID;
    packet_hello_resp.length = sizeof(packet_hello_resp);
    packet_hello_resp.version = SAHARA_VERSION_MAJOR;
    packet_hello_resp.version_supported = SAHARA_VERSION_MAJOR_SUPPORTED;
    packet_hello_resp.status = 0x00; //Success
    packet_hello_resp.mode = SAHARA_MODE_IMAGE_TX_PENDING;
    packet_hello_resp.reserved0 = 0;
    packet_hello_resp.reserved1 = 0;
    packet_hello_resp.reserved2 = 0;
    packet_hello_resp.reserved3 = 0;
    packet_hello_resp.reserved4 = 0;
    packet_hello_resp.reserved5 = 0;
    int sent = 0;
    write_tx((u8 *)&packet_hello_resp, packet_hello_resp.length, &sent);
    if (sent != packet_hello_resp.length){
        status = -1;
    }
    return status;
}

int sahara_handle_packet_read_data(struct sahara_packet_read_data *packet_read_data, int len)
{
    int status = 0;
    do{
        if ((len != sizeof(struct sahara_packet_read_data))||
            (len != packet_read_data->length)){
            status = -20;
            break;
        }
        if (packet_read_data->command != SAHARA_READ_DATA_ID){
            status = -21;
            break;
        }
    } while(0);
    return status;
}

int sahara_handle_packet_end_image_tx(struct sahara_packet_end_image_tx *end_image_tx, int len)
{
    int status = 0;
//    hexdump(end_image_tx, sizeof(struct sahara_packet_end_image_tx));
    do{
        if ((len != sizeof(struct sahara_packet_end_image_tx))||
            (len != end_image_tx->length)){
            status = -30;
            break;
        }
        if (!(end_image_tx->command==SAHARA_END_IMAGE_TX_ID &&
             end_image_tx->status==SAHARA_STATUS_SUCCESS)){
            status = -31;
            break;
        }

    } while(0);
    return status;
}

int sahara_send_packet_done()
{   
    int status = 0;
    int sent = 0;
    struct sahara_packet_done done;
    done.command = SAHARA_DONE_ID;
    done.length = sizeof(struct sahara_packet_done);
    write_tx(&done, done.length, &sent);
    if(sent!= done.length)
        status = -40;
    return status;
} 

int sahara_handle_packet_done_resp()
{
    int status = 0;
    struct sahara_packet_done_resp done_resp;
    int length = 0;
    read_rx(&done_resp, sizeof(struct sahara_packet_done_resp), &length);
    if (length != sizeof(struct sahara_packet_done_resp) ||
         ((u8 *)(&done_resp))[0] != SAHARA_DONE_RESP_ID ){
        status = -41;
        return status;
    }

    status = done_resp.image_tx_status; //IMAGE_TX_PENDING or IMAGE_TX_COMPLETE
    return status;
}

int dowload_firehose_image(char *image_path)
{
    int status = 0;
    int fd;
    struct stat file_stat;
    struct sahara_packet_read_data *packet_read_data;
    u32 offset;
    u32 length;
    int end_image_tx_bool = 0;
    u8 buf[128] = {0};

    fd = open(image_path, O_RDONLY);
    if(fd<0){
        perror(image_path);
        exit(-1);
    }
    fstat(fd, &file_stat);
    u8 *image_data = malloc(file_stat.st_size);
    if(image_data==NULL){
        perror("malloc");
        exit(1);
    }
    read(fd, image_data, file_stat.st_size);
    close(fd);

    sahara_handle_packet_hello();       //rev hello
    sahara_send_packet_hello_resp();    //send hello resp packet

    while(1){
        int len = 0;
        memset(buf, 0, 128);
        read_rx(buf, 128, &len);              //read_data or end_image_tx
        switch(buf[0]){
            case SAHARA_READ_DATA_ID:
                status = sahara_handle_packet_read_data((struct sahara_packet_read_data *)buf, len);
                packet_read_data = (struct sahara_packet_read_data *)buf;
                break;
            case SAHARA_END_IMAGE_TX_ID:
                status =
                    sahara_handle_packet_end_image_tx((struct sahara_packet_end_image_tx *)buf, len);
                end_image_tx_bool = 1;
                break;
            defaut:
                status = -1;
        }
        if(status || end_image_tx_bool)
            break;
        offset = packet_read_data->data_offset;
        length = packet_read_data->data_length;
        write_tx(image_data+offset, length, &len);
    }
    if (status<0)
        return status;
    status = sahara_send_packet_done();
    if (status<0)
        return status;
    status = sahara_handle_packet_done_resp();
    if (status<0)
        return status;
    free(image_data);
    return status;
}
