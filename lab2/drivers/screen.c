//
// Created by sdfedorov on 14/01/2021.
//

#include "screen.h"

char* vidptr = VIDEO_ADDRESS;
unsigned int current_loc;

void clear_screen(void){
    unsigned int i = 0;
    while (i < SCREEN_SIZE) {
        vidptr[i++] = ' ';
        vidptr[i++] = DEFAULT_COLOR;
    }
}

void print_newline(void){
    unsigned int line_size = 2 * MAX_COLS;
    current_loc += (line_size - current_loc % (line_size));
}

void print(const char *str){
    unsigned int i = 0;
    while (str[i] != '\0') {
        if (str[i] == '\n') {
            print_newline();
        } else {
            vidptr[current_loc++] = str[i];
            vidptr[current_loc++] = DEFAULT_COLOR;
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