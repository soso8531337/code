CC=arm-linux-gcc
all:
	$(CC) -g -DLINUX -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE -D_BOARD_DLDUIS2000 -D_TEST_RC -D_HAVE_INTTYPES_H -MD -Wall -fpic  -o xl *.c -L./mipsel -lpthread -liconv -lz -ljson 
clean:
	rm xl *.o -rf
