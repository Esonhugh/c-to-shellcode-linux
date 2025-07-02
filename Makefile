PAYLOAD=payload.c

CC=gcc
CFLAGS=-Os -fPIC -nostdlib -nostartfiles -ffreestanding -fno-asynchronous-unwind-tables -fno-ident -s -e start
CFLAGS=-Os -fPIC -pie -nostdlib -nostartfiles -ffreestanding -fno-asynchronous-unwind-tables -fno-ident -s -e start

CFLAGS  = -Os   # This is for size reduce Optimizing flag
CFLAGS += -fPIC # This is for generate position independent code
CFLAGS += -pie  # This is for generate position independent executable
CFLAGS += -nostdlib # This is for not linking standard libraries
CFLAGS += -nostartfiles # This is for not linking standard startup files
CFLAGS += -ffreestanding # This is for not linking standard libraries
CFLAGS += -fno-asynchronous-unwind-tables # This is for not generating unwind
CFLAGS += -fno-ident # This is for not generating identification
CFLAGS += -s # This is for stripping symbols
CFLAGS += -e start # This is for setting the entry point to 'start'


#DEBUG=gdb
DEBUG=pwndbg

# build binary payload.bin from payload.c
build: payload.obj
build: link
build: cleanup

# run the payload with a custom loader 
run: craft
run:
	./runner

# run the payload but run with debugger
debug: craft
debug:
	$(DEBUG) ./runner

# clear everthing generated
clean:
	rm -f payload.o core payload.bin .gdb_history runner

payload.obj:
	$(CC) -c $(PAYLOAD) -o payload.o $(CFLAGS)

link:
	ld -T common/linker.ld payload.o -o payload.bin

cleanup:
	rm -f payload.o core .gdb_history runner

craft:
	python3 ./common/craft.py > runner.c
	gcc ./runner.c -o runner
	rm ./runner.c

install-pwndbg:
	curl -qsL 'https://install.pwndbg.re' | sh -s -- -t pwndbg-gdb
