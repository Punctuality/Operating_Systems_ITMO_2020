#include "../drivers/keyboard_map.h"
#include "../drivers/screen.h"
#include "../drivers/idt.h"

extern unsigned char read_port(int port);
extern void write_port(unsigned short port, unsigned char data);
extern void keyboard_handler_int();

void keyboard_handler(void)
{
    const char *echo = "echo> ";
    signed char keycode;
    keycode = read_port(0x60);
    /* Only print characters on keydown event that have
     * a non-zero mapping */
    if (keyboard_map[keycode] == '\n') {
        print_newline();
        print(echo);
    } else if(keycode >= 0) {
        print("NEW HEX: ");
        print_hex(keycode);
        print(" ");
        vidptr[current_loc++] = keyboard_map[keycode];
        /* Attribute 0x07 is white on black characters */
        vidptr[current_loc++] = 0x07;

        print_newline();
    }

    /* Send End of Interrupt (EOI) to master PIC */
    write_port(0x20, 0x20);
}

void kb_init(void)
{
    /* This is a very basic keyboard initialization. The assumption is we have a
     * PS/2 keyboard and it is already in a propr state. This may not be the case
     * on real hardware. We simply enable the keyboard interupt */

    /* Get current master PIC interrupt mask */
    unsigned char curmask_master = read_port (0x21);

    /* 0xFD is 11111101 - enables only IRQ1 (keyboard) on master pic
       by clearing bit 1. bit is clear for enabled and bit is set for disabled */
    write_port(0x21, curmask_master & 0xFD);
}

void kmain() {
    vidptr = VIDEO_ADDRESS;
    current_loc = 0;
    for (int i = 0; i < IDT_SIZE; i++) {
        load_idt_entry(i, 0, 0, 0);
    }
    idt_init();
    load_idt_entry(0x21, (unsigned long) keyboard_handler_int, 0x08, 0x8e);
    kb_init();
    clear_screen();

    print("KEYMAP_TABLE:\n");
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 8; j++) {
            print_hex(keyboard_map[i*8 + j]);
            print(" ");
        }
        print_newline();
    }

    while(1) __asm__("hlt\n\t");
}
