
CC=gcc
CFLAG = -Iinclude -fPIC -shared
LD_FLAG = -fPIC -s -Wl,-z,relro,-z,now,-z,noexecstack -fstack-protector-all

test: test_constructor
test:
	file test.so
	readelf -a test.so

test_no_libc_lib:
	$(CC) $(CFLAG) -o test.so 1.c

test_libc_lib:
	$(CC) $(CFLAG) -o test.so 2.c

test_go_lib:
	go build --buildmode=c-shared -o test.so  1.go

test_constructor:
	$(CC) $(CFLAG) -o test.so 3.c