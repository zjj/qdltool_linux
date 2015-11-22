CFLAGS = `pkg-config --libs --cflags libusb-1.0` -lpthread -lrt
CC = gcc

flash_sources = firehose.c  firehose.h  generic.h  qdl_usb.c  qdl_usb.h  sahara.c  sahara.h xml_parser.c  xml_parser.h misc.h misc.c

to_qdl_mode_sources = generic.h misc.h misc.c

all:flash to_qdl_mode

flash: flash.c $(flash_sources)
	$(CC) $(flash_sources) flash.c $(CFLAGS) -o flash

to_qdl_mode: to_qdl_mode.c $(to_qdl_mode_sources)
	$(CC) $(to_qdl_mode_sources) to_qdl_mode.c $(CFLAGS) -o to_qdl_mode


clean:
	rm -f flash to_qdl_mode
