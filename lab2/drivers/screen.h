//
// Created by sdfedorov on 14/01/2021.
//

#ifndef OPERATING_SYSTEMS_ITMO_2020_SCREEN_H
#define OPERATING_SYSTEMS_ITMO_2020_SCREEN_H

#define VIDEO_ADDRESS (char*) 0xB8000
#define MAX_COLS 80
#define MAX_ROWS 25
#define BYTES_PER_CHAR 2
#define SCREEN_SIZE MAX_ROWS * MAX_COLS * BYTES_PER_CHAR
#define DEFAULT_COLOR 0x0f

char *vidptr;
unsigned int current_loc;

void clear_screen(void);
void print_newline(void);
void print(const char *str);
void print_hex(unsigned char hex_val);

#endif //OPERATING_SYSTEMS_ITMO_2020_SCREEN_H
