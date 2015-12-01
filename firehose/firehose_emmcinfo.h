#ifndef FIREHOSE_EMMCINFO_H
#define FIREHOSE_EMMCINFO_H
#include "firehose_share.h"
extern size_t num_disk_sectors;

extern response_t firehose_emmc_info_response_xml_reader(xml_reader_t *reader);
extern response_t firehose_emmc_info_reponse();
extern response_t firehose_emmc_info();

#endif
