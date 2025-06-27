PAYLOAD=payload.c

CC=gcc
CFLAGS=-Os -fPIC -nostdlib -nostartfiles -ffreestanding -fno-asynchronous-unwind-tables -fno-ident -s -e start


#DEBUG=gdb
DEBUG=pwndbg

build: payload.obj
build: link
build: cleanup

run: craft
run:
	./runner

debug: craft
debug:
	$(DEBUG) ./runner

clean:
	rm -f payload.o core payload.bin .gdb_history runner

payload.obj:
	$(CC) -c $(PAYLOAD) -o payload.o $(CFLAGS)

link:
	ld -T common/linker.ld payload.o -o payload.bin


extract:
	objcopy -O binary --only-section=.text payload.bin out.bin

cleanup:
	rm -f payload.o core .gdb_history runner

craft:
	python3 ./common/craft.py > runner.c
	gcc ./runner.c -o runner
	rm ./runner.c

install-pwndbg:
	curl -qsL 'https://install.pwndbg.re' | sh -s -- -t pwndbg-gdb
