//
// Created by sdfedorov on 14/01/2021.
//

#include "shell.h"
#include "../drivers/screen.h"

#define MSG_LEN 9

static char cli_msg[MSG_LEN] = "~console>";
static char cur_row[MAX_COLS];
static int cur_row_pos;
static char* result;

static void flush_the_row() {
    for (int i = 0; i < MAX_COLS; i++) {
        cur_row[i] = 0;
    }
    cur_row_pos = 0;
}

void shell_init() {
    clear_screen();
    flush_the_row();
// Greetings
    print(cli_msg, DEFAULT_MSG_COLOR);
}

void execute_command() {
    // TODO
    result = "some result";
    return;
}

void receive_char(char new_char) {
    switch (new_char) {
        case '\n': {
            execute_command();
            flush_the_row();
            print_newline();
            print_d(result);
            print_newline();
            print(cli_msg, DEFAULT_MSG_COLOR);
            break;
        };
        case 0x08: {
            if (cur_row_pos > 0) {
                backspace(1);
                cur_row[cur_row_pos--] = 0;
            }
            break;
        }
        default: {
            cur_row[cur_row_pos++] = new_char;
//            print_hex(new_char);
//            print_char(' ');
            print_char_d(new_char);
            break;
        }

    }
}
