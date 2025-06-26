# b main
# b *(void ()())(PAYLOAD)
file ./runner
# break point at payload init point
b *(&main+66)
r
# step into shellcode payload
si 