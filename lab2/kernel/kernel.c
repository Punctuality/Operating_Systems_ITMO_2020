#define SCREEN_SIZE 25*80*2
#define VIDEO_MEM 0xB8000
#define MAX_COLS 80
#define MAX_ROWS 25
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

char *vidptr = VIDEO_MEM;
struct idt_entry idt_table[IDT_SIZE];
struct idt_pointer idt_ptr;
unsigned int current_loc;

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

void clear_screen(void){
    unsigned int i = 0;
    while (i < SCREEN_SIZE) {
        vidptr[i++] = ' ';
        vidptr[i++] = 0x07;
    }
}

void print_newline(void){
    unsigned int line_size = 2 * MAX_COLS;
    current_loc += (line_size - current_loc % (line_size));
}

void print(const char *str){
    char *vidptr = VIDEO_MEM;
    unsigned int i = 0;
    while (str[i] != '\0') {
        if (str[i] == '\n') {
            print_newline();
        } else {
            vidptr[current_loc++] = str[i];
            vidptr[current_loc++] = 0x07;
        }
        i++;
    }
}

void print_hex(unsigned char hex_val) {
    char hex[3];
    hex[1] = (char) (hex_val % 16) + 48;
    hex[0] = (char) ((hex_val >> 4) % 16) + 48;
    hex[2] = '\0';
    print(hex);
}

void test_print(void) {
    const char *echo = "tetasts> ";
    print(echo);
}

#include "../drivers/keyboard_map.h"

void keyboard_handler(void)
{
    const char *echo = "echo> ";
    signed char keycode;
    char *vidptr = VIDEO_MEM;
    // TODO check the mask
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


void fill_map(){
    unsigned char tmp[128] = {
            0,  27, '1', '2', '3', '4', '5', '6', '7', '8',     /* 9 */
            '9', '0', '-', '=', '\b',     /* Backspace */
            '\t',                 /* Tab */
            'q', 'w', 'e', 'r',   /* 19 */
            't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', /* Enter key */
            0,                  /* 29   - Control */
            'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',     /* 39 */
            '\'', '`',   0,                /* Left shift */
            '\\', 'z', 'x', 'c', 'v', 'b', 'n',                    /* 49 */
            'm', ',', '.', '/',   0,                              /* Right shift */
            '*',
            0,  /* Alt */
            ' ',  /* Space bar */
            0,  /* Caps lock */
            0,  /* 59 - F1 key ... > */
            0,   0,   0,   0,   0,   0,   0,   0,
            0,  /* < ... F10 */
            0,  /* 69 - Num lock*/
            0,  /* Scroll Lock */
            0,  /* Home key */
            0,  /* Up Arrow */
            0,  /* Page Up */
            '-',
            0,  /* Left Arrow */
            0,
            0,  /* Right Arrow */
            '+',
            0,  /* 79 - End key*/
            0,  /* Down Arrow */
            0,  /* Page Down */
            0,  /* Insert Key */
            0,  /* Delete Key */
            0,   0,   0,
            0,  /* F11 Key */
            0,  /* F12 Key */
            0,  /* All other keys are undefined */
    };
    for (int i = 0; i < 128; i++) keyboard_map[i] = tmp[i];
}

void kernel_c() {
    vidptr = VIDEO_MEM;
    current_loc = 0;
    const char *echo = "echo> ";
    for (int i = 0; i < IDT_SIZE; i++) {
        load_idt_entry(i, 0, 0, 0);
    }
    idt_init();
    load_idt_entry(0x21, (unsigned long) keyboard_handler_int, 0x08, 0x8e);
    kb_init();
    clear_screen();

    print("KEYMAP_TABLE:\n");
    unsigned char cur[1];
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 8; j++) {
            print_hex(keyboard_map[i*8 + j]);
            print(" ");
        }
        print_newline();
    }

    while(1) __asm__("hlt\n\t");
}
