bits 16
org 0x8000

KERNEL_LOAD     equ 0x00020000
KERNEL_LBA      equ 10
KERNEL_SECTORS  equ 64 ;  32

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    sti

    ; ---- show stage2 running ----
    mov ah,0x0E
    mov al,'S'
    int 0x10

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
    jc disk_fail

    mov ah,0x0E
    mov al,'J'
    int 0x10

    call enable_a20

    ; ---- switch to protected mode ----
    cli
    lgdt [gdt_descriptor]

    mov eax, cr0
    or eax, 1
    mov cr0, eax

    jmp 0x08:pm_entry

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

    mov esp,0x0009F000

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
