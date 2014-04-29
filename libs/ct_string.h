#ifndef CT_STRING_H
#define CT_STRING_H

#include <stdint.h>

typedef struct ConstantTimeString {
    uint32_t allocated_length;
    uint32_t actual_length;
    unsigned char *string;
} ct_string;

void ct_string_init(ct_string *str);
int ct_string_concat(ct_string *str, const unsigned char *to_append, uint32_t max_length, uint32_t actual_length);
void ct_string_finalize(ct_string *str, unsigned char *buf, unsigned char filler);
uint32_t ct_string_allocated_length(ct_string *str);
void ct_string_deinit(ct_string *str);

#endif
