# b main
# b *(void ()())(PAYLOAD)
# file ./runner
# set debug-file-directory ./
# print PAYLOAD
# print *(void ()())(PAYLOAD)
# break point at payload init point
b *(&main+71)
r
# step into shellcode payload
si
# add-symbol-file 
source ./common/add-symbol-command.gdb

## shellcode disassemble
# disassemble