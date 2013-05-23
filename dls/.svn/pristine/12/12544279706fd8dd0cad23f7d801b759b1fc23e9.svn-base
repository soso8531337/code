CC=arm-linux-gcc
all:
	$(CC) -g  -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE -D_TEST_RC -MD -Wall -fpic  -I/opt/toolchain/gcc-4.2.4-uClibc-0.9.29-toolchain/usr/include  --sysroot=/opt/toolchain/gcc-4.2.4-uClibc-0.9.29-toolchain -o xl *.c -L./mipsel -lpthread -liconv  -lembed_thunder_manager  -lembed_thunder -ljson
clean:
	rm xl *.o -rf
