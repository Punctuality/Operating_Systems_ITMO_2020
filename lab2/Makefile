CC = gcc
ASM = nasm
LD = ld

run: all target/os-image
	qemu-system-x86_64 -fda target/os-image

recompile: prep all

all: target/os-image

prep: clean boot/ kernel/ drivers/
	mkdir target

clean:
	rm -rf target/

target/screen.o: drivers/screen.c
	$(CC) -fno-pie -m32 -ffreestanding -c $< -o $@

target/kernel.o: kernel/kernel.c
	$(CC) -fno-pie -m32 -ffreestanding -c $< -o $@

target/kernel_entry.o: boot/kernel_entry.asm
	$(ASM) $< -f elf32 -o $@

target/io_functions.o: drivers/io_functions.asm
	$(ASM) $< -f elf32 -o $@

target/bootstrap.bin: boot/bootstrap.asm
	$(ASM) $< -f bin -o $@

target/kernel.bin: target/kernel_entry.o target/screen.o target/kernel.o target/io_functions.o
	$(LD) -m elf_i386 -o $@ -Ttext 0x1000 $^ --oformat binary

target/os-image: target/bootstrap.bin target/kernel.bin
	cat $^ > $@