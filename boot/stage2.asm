bits 16
org 0x8000

KERNEL_LOAD     equ 0x00020000
KERNEL_LBA      equ 10
KERNEL_SECTORS  equ 128 ;  64KB

start:
    ; Disable interrupts and set memory segments to start of memory
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    sti

    ; ---- show stage2 running ----
    mov ah,0x0E
    mov al,'S'
    int 0x10

    call load_kernel
    jc disk_fail

    mov ah,0x0E
    mov al,'J'
    int 0x10

    call setup_vbe
    call enable_a20


    ; ---- switch to protected mode ----
    cli
    lgdt [gdt_descriptor]

    mov eax, cr0
    or eax, 1
    mov cr0, eax

    jmp 0x08:pm_entry

setup_vbe:
    ; 1. Get VBE Controller Info
    mov di, vbe_info_block
    mov dword [di], "VBE2"      ; Set signature BEFORE calling 4F00h
    mov ax, 0x4F00
    int 0x10
    
    cmp ax, 0x004F
    jne .vbe_failed

    ; 2. Point FS:SI to the mode list
    ; The list is a Far Pointer (Segment:Offset)
    mov si, [vbe_info_block + 14] 
    mov ax, [vbe_info_block + 16]
    mov fs, ax

.next_mode:
    mov cx, [fs:si]             ; Get mode number from list
    add si, 2                   ; Advance pointer for next time
    
    cmp cx, 0xFFFF              ; End of list marker?
    je .mode_not_found
    
    push si                     ; Save list position
    push cx                     ; Save the mode we're testing

    ; 3. Get Mode Info for THIS specific mode
    ; CX is already the mode number
    xor ax, ax
    mov es, ax
    mov ax, 0x4F01
    mov di, mode_info_block
    int 0x10
    
    cmp ax, 0x004F
    jne .pop_and_retry

    ; --- CHECK ATTRIBUTES ---
    mov ax, [mode_info_block]
    test ax, 0x01               ; Mode supported?
    jz .pop_and_retry
    test ax, 0x80               ; Linear Frame Buffer supported?
    jz .pop_and_retry

    ; --- CHECK FOR 800x600x32 ---
    cmp word [mode_info_block + 18], 800
    jne .pop_and_retry
    cmp word [mode_info_block + 20], 600
    jne .pop_and_retry
    cmp byte [mode_info_block + 25], 16
    jne .pop_and_retry
    
    ; 4. FOUND IT!
    pop cx                      ; Restore our found mode number
    pop si                      ; Clean up stack
    
    mov ax, 0x4F02
    mov bx, cx
    or bx, 0x4000               ; BIT 14: Enable Linear Framebuffer
    int 0x10
    
    ; 5. Save the Physical Address of the Framebuffer (Offset 40)
    mov eax, [mode_info_block + 40]
    mov [vbe_lfb_ptr], eax
    ret

.pop_and_retry:
    pop cx
    pop si
    jmp .next_mode

.vbe_failed:
    mov al, 'V'
    jmp .error_print
.mode_not_found:
    mov al, 'X'
.error_print:
    mov ah, 0x0E
    int 0x10
.hang:
    cli
    hlt
    jmp .hang

load_kernel:
    ; ---- load kernel ----
    mov ax, (KERNEL_LOAD >> 4)
    mov es, ax
    xor bx, bx

    mov word [dap_sector_count], KERNEL_SECTORS
    mov word [dap_offset], bx
    mov word [dap_segment], es
    mov dword [dap_lba], KERNEL_LBA

    mov dl, 0x80
    mov si, dap
    mov ah, 0x42
    int 0x13
    ret

enable_a20:
    in   al, 0x92
    or   al, 00000010b      ; set A20 enable bit
    and  al, 11111110b      ; optional: clear reset bit
    out  0x92, al
    ret    

; ----------------------------------------------------
; 32-bit protected mode
; ----------------------------------------------------
bits 32
pm_entry:
    mov ax,0x10
    mov ds,ax
    mov es,ax
    mov fs,ax
    mov gs,ax
    mov ss,ax

; ---- NEW: Copy Kernel from 0x20000 to 0x100000 ----
    ; We move (KERNEL_SECTORS * 512) bytes
    mov esi, 0x20000          ; Source: where BIOS loaded it
    mov edi, 0x100000         ; Destination: 1MB mark
    mov ecx, (KERNEL_SECTORS * 512) / 4   ; Count: (Sectors * 512 bytes) / 4 (for dwords)
    rep movsd                 ; Perform the copy

    mov esp,0x0009F000

    ; Move the address of the info block into EBX
    ; Since ES was 0 and ORG was 0x8000, the address is just the label
    mov ebx, mode_info_block 

    jmp KERNEL_LOAD

; ----------------------------------------------------
; back to 16-bit data definitions
; ----------------------------------------------------
bits 16

disk_fail:
    mov ah,0x0E
    mov al,'E'
    int 0x10
.hang:
    cli
    hlt
    jmp .hang

; -------- GDT --------
gdt:
    dq 0x0000000000000000
    dq 0x00CF9A000000FFFF
    dq 0x00CF92000000FFFF

gdt_descriptor:
    dw gdt_descriptor - gdt - 1
    dd gdt

; -------- Disk Address Packet --------
dap:
    db 0x10
    db 0
dap_sector_count: dw 0
dap_offset:       dw 0
dap_segment:      dw 0
dap_lba:          dd 0
                  dd 0
vbe_info_block:  times 512 db 0 ; Reserve 512 bytes for VBE Info
mode_info_block: times 256 db 0    ; Reserve 256 bytes for Mode Info
vbe_lfb_ptr:     dd 1          ; To store the Linear Framebuffer address