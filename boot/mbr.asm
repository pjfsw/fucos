bits 16
org 0x7C00

start:
  mov ah, 0x0E
  mov al, 'M'
  int 0x10

  cli
  xor ax, ax
  mov ds, ax
  mov es, ax
  mov ss, ax
  mov sp, 0x7C00
  sti

  mov [boot_drive], dl

  ; load stage2 to 0000:8000 (phys 0x8000)
  mov bx, 0x8000
  mov dh, 0          ; head
  mov ch, 0          ; cylinder
  mov cl, 2          ; sector (1 = MBR, 2 = next)
  mov al, 16         ; sectors to read (8KB). Adjust to your stage2 size.
  mov dl, [boot_drive]
  mov ah, 0x02       ; int13 read
  int 0x13
  jc disk_fail

  ; after reading stage2 successfully
  mov [0x8000 + boot_drive - 0x8000], dl
  jmp 0x0000:0x8000

disk_fail:
  mov si, msg
.print:
  lodsb
  test al, al
  jz .hang
  mov ah, 0x0E
  int 0x10
  jmp .print
.hang:
  cli
  hlt
  jmp .hang

boot_drive db 0
msg db "MBR disk read fail", 0

times 510-($-$$) db 0
dw 0xAA55
