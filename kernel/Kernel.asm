BITS    64
mov     ecx, 0x0013c680
mov     eax, 0xff18679a
xor     rdi, rdi
mov     rdi, 0xc0000000

Write:
mov     [rdi], eax
add     rdi, 4
loop    Write

jmp     $