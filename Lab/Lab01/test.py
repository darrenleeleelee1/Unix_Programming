#!/usr/bin/env python3
from pwn import *
import pow as pw
import base64

r = remote('up23.zoolab.org', 10363)
pw.solve_pow(r)

r.recvuntil(b'Please complete the ')
num_of_challenge = int(r.recvuntil(b' ').decode())

r.recvuntil(b'\n\n')
r.recvuntil(b'\n\n')

for i in range(1, num_of_challenge+1):
    r.recvuntil(b' ')
    token = f"{i}: "
    r.recvuntil(token.encode())
    expression = r.recvuntil(b'=').decode()[:-1]
    
    big_num = eval(expression)
    bytes_le = big_num.to_bytes((big_num.bit_length() + 7) // 8, 'little')
    encoded = base64.b64encode(bytes_le)

    print(f"{token} {expression} = {encoded.decode()}")
    r.sendline(encoded)

# Read output in a non-blocking way
output = b''
while True:
    try:
        data = r.recv(timeout=1)
        if not data:
            break
        output += data
    except EOFError:
        break

print(output)
