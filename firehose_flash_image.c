#include "firehose_flash_image.h"

int process_sprase_header(int fd, sparse_header_t *sparse_header)
{   
    int ret = 0;
    ret = read(fd, sparse_header, sizeof(sparse_header_t));
    if (ret < 0) {
        xerror("sparse_header init errror");
    }
    if (sparse_header->magic != SPARSE_HEADER_MAGIC) {
        //xerror("sparse_header.magic");
        lseek64(fd, -sizeof(sparse_header_t),SEEK_CUR); //if this is not sparse, just lseek back
        return -1;
    }
    if (sparse_header->major_version != SPARSE_HEADER_MAJOR_VER) {
        xerror("sparse_header.major_version");
    }
    if (sparse_header->file_hdr_sz < SPARSE_HEADER_LEN) {
        xerror("sparse_header.file_hdr_sz");
    }
    if (sparse_header->chunk_hdr_sz < sizeof(chunk_header_t)) {
        xerror("sparse_header.chunk_hdr_sz");
    }
    if (sparse_header->file_hdr_sz > SPARSE_HEADER_LEN) {
        lseek64(fd, sparse_header->file_hdr_sz - SPARSE_HEADER_LEN, SEEK_CUR);
    }
    return 0;
}


response_t process_sparse_chunks(int fd, sparse_header_t sparse_header, firehose_program_t program)
{
    chunk_header_t chunk_header;
    size_t bigchunk_left = sizeof(bigchunk);
    char *bigchunk_ptr = bigchunk;
    off64_t out_offset = 0;
    size_t base_start_sector = program.start_sector;
    response_t resp;
    int i = 0;
    for (i = 0; i < sparse_header.total_chunks; i++) {
        int ret = 0;
        ret = read(fd, &chunk_header, sizeof(chunk_header_t));
        if (ret < 0) {
            xerror("read chunk_header error");
        }
        if (sparse_header.chunk_hdr_sz > CHUNK_HEADER_LEN) {
            lseek64(fd, sparse_header.chunk_hdr_sz - CHUNK_HEADER_LEN, SEEK_CUR);
        }
        size_t raw_chunk_data_size = 0;
        switch (chunk_header.chunk_type){
            case(CHUNK_TYPE_RAW):
                raw_chunk_data_size = chunk_header.chunk_sz*sparse_header.blk_sz;
                while(raw_chunk_data_size){
                    size_t to_read, r;
                    to_read = min(raw_chunk_data_size, bigchunk_left);
                    r = read(fd, bigchunk_ptr, to_read);
                    if (r != to_read)
                        xerror("r != to_read");
                    bigchunk_left -= r;
                    bigchunk_ptr += r;
                    raw_chunk_data_size -= r;
                    out_offset += r;  
                    if (bigchunk_left == 0){
                        program.sector_numbers = sizeof(bigchunk)/program.sector_size;
                        program.start_sector = base_start_sector + out_offset/program.sector_size - program.sector_numbers;
                        resp = transmit_chunk(bigchunk, program);
                        if (resp != ACK){
                            xerror("transmit_chunk error");
                        }
                        bigchunk_left =  sizeof(bigchunk);
                        bigchunk_ptr = bigchunk;
                    }
                }
                break;
            case(CHUNK_TYPE_FILL):
                /*
                    don't do fill now, maybe in the future since
                    there's no fill chunk in android's image for the time being.
                */
                if (bigchunk_left < sizeof(bigchunk)){
                    program.sector_numbers = (bigchunk_ptr - bigchunk)/program.sector_size;
                    program.start_sector = base_start_sector + out_offset/program.sector_size - program.sector_numbers;
                    resp = transmit_chunk(bigchunk, program);
                    if (resp != ACK){
                        xerror("transmit_chunk error");
                    }
                    bigchunk_left =  sizeof(bigchunk);
                    bigchunk_ptr = bigchunk;
                }
                lseek64(fd, chunk_header.total_sz-CHUNK_HEADER_LEN, SEEK_CUR); //4bytes
                out_offset += chunk_header.chunk_sz*sparse_header.blk_sz;
                break;
            case(CHUNK_TYPE_CRC32):
                if (bigchunk_left < sizeof(bigchunk)){
                    program.sector_numbers = (bigchunk_ptr - bigchunk)/program.sector_size;
                    program.start_sector = base_start_sector + out_offset/program.sector_size - program.sector_numbers;
                    resp = transmit_chunk(bigchunk, program);
                    if (resp != ACK){
                        xerror("transmit_chunk error");
                    }
                    bigchunk_left =  sizeof(bigchunk);
                    bigchunk_ptr = bigchunk;
                }
                lseek64(fd, chunk_header.total_sz-CHUNK_HEADER_LEN, SEEK_CUR); //4bytes
                break;
            case(CHUNK_TYPE_DONT_CARE):
                if (bigchunk_left < sizeof(bigchunk)){
                    program.sector_numbers = (bigchunk_ptr - bigchunk)/program.sector_size;
                    program.start_sector = base_start_sector + out_offset/program.sector_size - program.sector_numbers;
                    resp = transmit_chunk(bigchunk, program);
                    if (resp != ACK){
                        xerror("transmit_chunk error");
                    }
                    bigchunk_left =  sizeof(bigchunk);
                    bigchunk_ptr = bigchunk;
                }
                lseek64(fd, chunk_header.total_sz-CHUNK_HEADER_LEN, SEEK_CUR);
                out_offset += chunk_header.chunk_sz*sparse_header.blk_sz;
                break;
            default:
                xerror("unknown chunk type");
        }
    }
    return ACK;
}

response_t process_sparse_file(int fd, firehose_program_t program)
{
    sparse_header_t sparse_header;
    if (process_sprase_header(fd, &sparse_header) < 0){
        return process_general_file(fd, program);
    }
    return process_sparse_chunks(fd, sparse_header, program);
}

response_t process_general_file(int fd, firehose_program_t program)
{
    response_t resp;
    size_t r = 0;
    while((r = read(fd, bigchunk, sizeof(bigchunk))) > 0){
        program.sector_numbers = (r + program.sector_size - 1)/program.sector_size;
        /*the xml will be updated in transmit_chunk*/
        resp = transmit_chunk(bigchunk, program);
        if (resp != ACK){
           xerror("transmit_chunk error in process_general_file");
        }
        program.start_sector += program.sector_numbers;
        memset(bigchunk, 0, r);
    }
    return ACK;
}

response_t process_simlock_file(int fd, firehose_simlock_t slk)
{
    /*
        we should transmit it in one loop ?
    */
    response_t resp;
    size_t r = 0;
    while((r = read(fd, bigchunk, sizeof(bigchunk))) > 0){
        slk.sector_numbers = (r + slk.sector_size - 1)/slk.sector_size;
        /*the xml will be updated in transmit_chunk*/
        resp = transmit_chunk_simlock(bigchunk, slk);
        if (resp != ACK){
           xerror("transmit_chunk error in process_simlock_file");
        }
        slk.start_sector += slk.sector_numbers;
        memset(bigchunk, 0, r);
    }
    return ACK;
}
