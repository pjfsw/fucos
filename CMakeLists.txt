cmake_minimum_required(VERSION 3.20)
project(fucos C ASM)

set(CMAKE_C_STANDARD 11)

set(SRC ${CMAKE_SOURCE_DIR})
set(BUILD ${CMAKE_BINARY_DIR})

# ---- Tools ----
find_program(NASM nasm REQUIRED)
find_program(OBJCOPY objcopy REQUIRED)
find_program(QEMU qemu-system-i386 REQUIRED)

# ---- Disk image settings ----
set(IMG ${BUILD}/disk.img)
set(IMG_SIZE_MB 64)

# disk layout
set(KERNEL_LBA 10)     # keep <63 while using CHS hack
set(STAGE2_SECTORS 16) # sectors loaded by MBR

# ============================================================
# MBR
# ============================================================

set(MBR_BIN ${BUILD}/mbr.bin)

add_custom_command(
    OUTPUT ${MBR_BIN}
    COMMAND ${NASM} -f bin ${SRC}/boot/mbr.asm -o ${MBR_BIN}
    DEPENDS ${SRC}/boot/mbr.asm
)

add_custom_target(mbr ALL DEPENDS ${MBR_BIN})

# ============================================================
# Stage2
# ============================================================

set(STAGE2_BIN ${BUILD}/stage2.bin)

add_custom_command(
    OUTPUT ${STAGE2_BIN}
    COMMAND ${NASM} -f bin ${SRC}/boot/stage2.asm -o ${STAGE2_BIN}
    DEPENDS ${SRC}/boot/stage2.asm
)

add_custom_target(stage2 ALL DEPENDS ${STAGE2_BIN})

# ============================================================
# Kernel (32-bit freestanding)
# ============================================================

set(KERNEL_TARGET kernel)
set(KERNEL_ELF ${BUILD}/kernel.elf)
set(KERNEL_BIN ${BUILD}/kernel.bin)

add_executable(${KERNEL_TARGET}
    ${SRC}/kernel/start.S
    ${SRC}/kernel/kmain.c
    ${SRC}/kernel/serial.c
    ${SRC}/kernel/textmode.c
    ${SRC}/kernel/idt.c
    ${SRC}/kernel/isr.S
)

# produce build/kernel.elf
set_target_properties(${KERNEL_TARGET} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${BUILD}
    OUTPUT_NAME kernel
    SUFFIX ".elf"
)

target_compile_options(${KERNEL_TARGET} PRIVATE
    -m32
    -ffreestanding
    -fno-pic
    -fno-pie
    -fno-stack-protector
    -fno-omit-frame-pointer
    -O2 -Wall -Wextra
    -nostdlib -nostartfiles -nodefaultlibs
)

target_link_options(${KERNEL_TARGET} PRIVATE
    -m32
    -T ${SRC}/linker.ld
    -nostdlib
    -Wl,--build-id=none
)

# Convert ELF -> flat binary
add_custom_command(
    OUTPUT ${KERNEL_BIN}
    COMMAND ${OBJCOPY} -O binary ${KERNEL_ELF} ${KERNEL_BIN}
    DEPENDS ${KERNEL_TARGET}
)

add_custom_target(kernel_bin ALL DEPENDS ${KERNEL_BIN})

# ============================================================
# Disk image creation
# ============================================================

add_custom_command(
    OUTPUT ${IMG}

    COMMAND ${CMAKE_COMMAND} -E rm -f ${IMG}
    COMMAND truncate -s ${IMG_SIZE_MB}M ${IMG}

    # MBR
    COMMAND dd if=${MBR_BIN} of=${IMG} bs=512 count=1 conv=notrunc status=none

    # stage2 at LBA1
    COMMAND dd if=${STAGE2_BIN} of=${IMG} bs=512 seek=1 conv=notrunc status=none

    # kernel
    COMMAND dd if=${KERNEL_BIN} of=${IMG} bs=512 seek=${KERNEL_LBA} conv=notrunc status=none

    DEPENDS mbr stage2 kernel_bin
)

add_custom_target(image
  COMMAND ${CMAKE_COMMAND} -E rm -f ${IMG}
  COMMAND truncate -s ${IMG_SIZE_MB}M ${IMG}
  COMMAND dd if=${MBR_BIN} of=${IMG} bs=512 count=1 conv=notrunc status=none
  COMMAND dd if=${STAGE2_BIN} of=${IMG} bs=512 seek=1 conv=notrunc status=none
  COMMAND dd if=${KERNEL_BIN} of=${IMG} bs=512 seek=${KERNEL_LBA} conv=notrunc status=none
  DEPENDS mbr stage2 kernel_bin
  USES_TERMINAL
)
# ============================================================
# QEMU run targets
# ============================================================

add_custom_target(run
    COMMAND ${QEMU}
        -machine accel=kvm
        -cpu host
        -m 64M
        -drive format=raw,file=${IMG}
        -vga std
        -display sdl
        -serial stdio
    DEPENDS image
    USES_TERMINAL
)

add_custom_target(debug
    COMMAND ${QEMU}
        -machine accel=kvm
        -cpu host
        -m 64M
        -drive format=raw,file=${IMG}
        -vga std
        -display sdl
        -serial stdio
        -S -s
    DEPENDS image
    USES_TERMINAL
)
