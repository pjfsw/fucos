global gfxBlit32To16
gfxBlit32To16:
    push ebp
    mov ebp, esp
    push edi
    push esi
    push ebx

    mov esi, [ebp+8]    ; src (32-bit Soft Buffer)
    mov edi, [ebp+12]   ; dst (16-bit Hardware)
    mov ebx, [ebp+20]   ; height
    
    ; Load Masks into XMM registers once
    movdqa xmm5, [mask_r]
    movdqa xmm6, [mask_g]
    movdqa xmm7, [mask_b]

.loop_y:
    push edi            ; Save start of hardware line
    mov edx, [ebp+16]   ; width
    shr edx, 2          ; 4 pixels per iteration

.loop_x:
    movdqu xmm0, [esi]  ; Load 4 pixels (32-bit each)

    ; Extract and align Red: (R >> 8) << 11
    movdqa xmm1, xmm0
    pand xmm1, xmm5     ; 00R80000
    psrld xmm1, 8       ; Shift Red down to correct bits (565)

    ; Extract and align Green: (G >> 8) << 5
    movdqa xmm2, xmm0
    pand xmm2, xmm6     ; 0000FC00
    psrld xmm2, 5       ; Shift Green down

    ; Extract and align Blue: (B >> 8)
    movdqa xmm3, xmm0
    pand xmm3, xmm7     ; 000000F8
    psrld xmm3, 3       ; Shift Blue down

    ; Combine channels
    por xmm1, xmm2
    por xmm1, xmm3

    ; Now we have 4 packed pixels in 4 dwords. Squeeze into 4 words.
    ; This packs [0000RRRR, 0000RRRR, 0000RRRR, 0000RRRR] -> [RRRRRRRR RRRRRRRR]
    packssdw xmm1, xmm1 
    movq [edi], xmm1    ; Store 4 pixels (8 bytes) to VRAM

    add esi, 16
    add edi, 8
    dec edx
    jnz .loop_x

    pop edi
    add edi, [ebp+24]   ; Correctly jump by hardware PITCH
    dec ebx
    jnz .loop_y

    pop ebx
    pop esi
    pop edi
    mov esp, ebp
    pop ebp
    ret

section .data
align 16
    mask_r times 4 dd 0x00F80000
    mask_g times 4 dd 0x0000FC00
    mask_b times 4 dd 0x000000F8