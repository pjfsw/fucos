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
    movdqu xmm0, [esi]  ; Load 4 pixels: [B|G|R|A] [B|G|R|A] ...

    ; --- BLUE (assuming it's in byte 0) ---
    movdqa xmm1, xmm0
    pand xmm1, [mask_b] ; Mask: 0x000000F8
    psrld xmm1, 3       ; Shift Blue down to bits 0-4

    ; --- GREEN (assuming it's in byte 1) ---
    movdqa xmm2, xmm0
    pand xmm2, [mask_g] ; Mask: 0x0000FC00
    psrld xmm2, 5       ; Shift Green down to bits 5-10

    ; --- RED (assuming it's in byte 2) ---
    movdqa xmm3, xmm0
    pand xmm3, [mask_r] ; Mask: 0x00F80000
    psrld xmm3, 8       ; Shift Red down to bits 11-15

    por xmm1, xmm2
    por xmm1, xmm3      ; Combine all: RRRRRGGGGGGBBBBB

    ; Trick packssdw by sign-extending the 16-bit values so they don't get clamped!
    pslld xmm1, 16      ; Shift left 16 bits
    psrad xmm1, 16      ; Arithmetic shift right 16 bits (sign-extends)

    packssdw xmm1, xmm1 ; Now it packs perfectly!
    movq [edi], xmm1    ; Store 8 bytes (4 pixels)

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
    ; These masks isolate the top 5 or 6 bits of each 8-bit channel
    mask_b times 4 dd 0x000000F8  ; Blue in first byte
    mask_g times 4 dd 0x0000FC00  ; Green in second byte
    mask_r times 4 dd 0x00F80000  ; Red in third byte