
CC=gcc
CFLAG = -Iinclude -fPIC -shared
LD_FLAG = -fPIC -s -Wl,-z,relro,-z,now,-z,noexecstack -fstack-protector-all

test.so:
	$(CC) $(CFLAG) -o test.so 1.c
	file test.so
	readelf -a test.so
