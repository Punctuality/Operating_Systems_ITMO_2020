//
// Created by sdfedorov on 14/01/2021.
//

#include "util.h"

void mem_cpy(char* dest, char* orig, unsigned long size) {
    unsigned long iter;
    for (iter = 0; iter < size; iter++) dest[iter] = orig[iter];
}

void str_cpy(char* dest, char* str) {
    unsigned long iter = 0;
    while (str[iter] != '\0') {
        dest[iter] = str[iter];
        iter++;
    }
}

char str_eq(char* a_str, char* b_str) {
    char equal = 1;
    unsigned long i = 0;
    do if (a_str[i] != b_str[i]) {
        equal = 0;
        break;
    } while (a_str[i] != '\0' || b_str[i] != '\0');
    return equal;
}