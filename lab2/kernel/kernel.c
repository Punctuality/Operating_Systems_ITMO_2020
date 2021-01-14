#include "../drivers/keyboard_map.h"
#include "../drivers/screen.h"

#define PIC_1_CTRL 0x20
#define PIC_2_CTRL 0xA0
#define PIC_1_DATA 0x21
#define PIC_2_DATA 0xA1
#define IDT_SIZE 256

struct idt_pointer
{
    unsigned short limit;
    unsigned int base;
} __attribute__((packed));

struct idt_entry
{
    unsigned short int offset_lowerbits;
    unsigned short int selector;
    unsigned char zero;
    unsigned char flags;
    unsigned short int offset_higherbits;
} __attribute__((packed));

struct idt_entry idt_table[IDT_SIZE];
struct idt_pointer idt_ptr;

static void initialize_idt_pointer()
{
    idt_ptr.limit = (sizeof(struct idt_entry) * IDT_SIZE) - 1;
    idt_ptr.base = (unsigned int)idt_table;
}

extern unsigned char read_port(int port);
extern void write_port(unsigned short port, unsigned char data);
extern void load_idt(void*);
extern void keyboard_handler_int();

void load_idt_entry(int isr_number, unsigned long base, short int selector, unsigned char flags)
{
    idt_table[isr_number].offset_lowerbits = base & 0xFFFF;
    idt_table[isr_number].offset_higherbits = (base >> 16) & 0xFFFF;
    idt_table[isr_number].selector = selector;
    idt_table[isr_number].flags = flags;
    idt_table[isr_number].zero = 0;
}

static void initialize_pic()
{
    /* ICW1 - begin initialization */
    write_port(PIC_1_CTRL, 0x11);
    write_port(PIC_2_CTRL, 0x11);

    /* ICW2 - remap offset address of idt_table */
    /*
    * In x86 protected mode, we have to remap the PICs beyond 0x20 because
    * Intel have designated the first 32 interrupts as "reserved" for cpu exceptions
    */
    write_port(PIC_1_DATA, 0x20);
    write_port(PIC_2_DATA, 0x28);

    /* ICW3 - setup cascading */
    write_port(PIC_1_DATA, 0x00);
    write_port(PIC_2_DATA, 0x00);

    /* ICW4 - environment info */
    write_port(PIC_1_DATA, 0x01);
    write_port(PIC_2_DATA, 0x01);
    /* Initialization finished */

    /* mask interrupts */
    write_port(0x21 , 0xff);
    write_port(0xA1 , 0xff);
}

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

void idt_init()
{
    initialize_pic();
    initialize_idt_pointer();
    load_idt(&idt_ptr);
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
