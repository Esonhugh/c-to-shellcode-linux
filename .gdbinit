# b main
# b *(void ()())(PAYLOAD)
file ./runner
# print PAYLOAD
# print *(void ()())(PAYLOAD)
# break point at payload init point
b *(&main+66)
r
# step into shellcode payload
si

## shellcode disassemble
# disassemble