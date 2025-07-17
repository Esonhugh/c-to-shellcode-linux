
CC=gcc
CFLAG = -Iinclude -fPIC -shared
LD_FLAG = -fPIC -s -Wl,-z,relro,-z,now,-z,noexecstack -fstack-protector-all

test.so1:
	$(CC) $(CFLAG) -o test.so 2.c
	file test.so
	readelf -a test.so

test.so:
	go build --buildmode=c-shared -o test.so  1.go