CFLAGS = `pkg-config --libs --cflags libusb-1.0` -static -lpthread -lrt
CC = gcc

sources = firehose.c  firehose.h  generic.h  qdl_usb.c  qdl_usb.h  sahara.c  sahara.h xml_parser.c  xml_parser.h misc.h misc.c

all:flash to_qdl_mode

flash: flash.c $(sources)
	$(CC) $(sources) flash.c $(CFLAGS) -o flash

to_qdl_mode: to_qdl_mode.c $(sources)
	$(CC) $(sources) to_qdl_mode.c $(CFLAGS) -o to_qdl_mode


clean:
	rm -f flash to_qdl_mode
