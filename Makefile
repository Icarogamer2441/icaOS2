# Makefile para icaOS2
ASM=nasm
CC=gcc
LD=ld

# Flags de compilação
ASMFLAGS=-f elf32
CFLAGS=-m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -Wall -Wextra -c -I.
LDFLAGS=-m elf_i386 -T link.ld

# Arquivos objeto
OBJECTS=boot/boot.o \
        kernel/kernel.o \
        drivers/screen.o \
        drivers/keyboard.o \
        drivers/ata.o \
        drivers/system.o \
        fs/filesystem.o \
        lib/string.o \
        lib/memory.o \
        compiler/compiler.o \
        editor/editor.o \
        ui/ui.o
KERNEL_BIN=kernel.bin

# Diretórios
ISO_DIR=iso
BOOT_DIR=$(ISO_DIR)/boot
GRUB_DIR=$(BOOT_DIR)/grub

# Alvos principais
all: os.iso

# Cria a imagem ISO
os.iso: $(KERNEL_BIN)
	mkdir -p $(GRUB_DIR)
	cp $(KERNEL_BIN) $(BOOT_DIR)/$(KERNEL_BIN)
	echo 'set timeout=0' > $(GRUB_DIR)/grub.cfg
	echo 'set default=0' >> $(GRUB_DIR)/grub.cfg
	echo '' >> $(GRUB_DIR)/grub.cfg
	echo 'menuentry "icaOS2" {' >> $(GRUB_DIR)/grub.cfg
	echo '    multiboot /boot/kernel.bin' >> $(GRUB_DIR)/grub.cfg
	echo '    module /boot/kernel.bin' >> $(GRUB_DIR)/grub.cfg
	echo '    boot' >> $(GRUB_DIR)/grub.cfg
	echo '}' >> $(GRUB_DIR)/grub.cfg
	grub-mkrescue -o os.iso $(ISO_DIR)

# Compila o kernel
$(KERNEL_BIN): $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $(OBJECTS)
	grub-file --is-x86-multiboot $@ || { echo "Erro: kernel.bin não é multiboot!"; exit 1; }

# Regra para arquivos .o a partir de .c
%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

# Regra para arquivos .o a partir de .asm
%.o: %.asm
	$(ASM) $(ASMFLAGS) $< -o $@

# Limpa os arquivos gerados
clean:
	rm -f $(OBJECTS) $(KERNEL_BIN) os.iso grub.cfg
	rm -rf $(ISO_DIR)

.PHONY: all clean 