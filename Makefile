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
#CFLAGS += -s # This is for stripping symbols
CFLAGS += -ggdb3
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
debug: add-symbol-command
debug:
	$(DEBUG) ./runner

# clear everthing generated
clean:
	rm -f payload.o core payload.bin .gdb_history runner ./common/add-symbol-command.gdb

payload.obj:
	$(CC) -c $(PAYLOAD) -o payload.o $(CFLAGS)

link:
	ld -T common/linker.ld -e start payload.o --oformat binary -o payload.bin

add-symbol-command:
	@bash ./common/add-symbol-command
	bash ./common/add-symbol-command > ./common/add-symbol-command.gdb

cleanup:
	rm -f core .gdb_history runner

craft:
	python3 ./common/craft.py > runner.c
	gcc ./runner.c -o runner
	rm ./runner.c
	# this is important to strip symbols from runner main
	objcopy -N PAYLOAD -N __dso_handle -N data_start -N __data_start ./runner

install-pwndbg:
	curl -qsL 'https://install.pwndbg.re' | sh -s -- -t pwndbg-gdb
