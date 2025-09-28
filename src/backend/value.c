#include "sepscr/backend/value.h"
#include "sf/str.h"
#include <stdlib.h>
#include <string.h>

sf_str ss_byte_string(ss_value self) {
    return sf_str_fmt("%02X", self.value.byte);
}

sf_str ss_number_string(ss_value self) {
    return sf_str_fmt("%f", self.value.number);
}

sf_str ss_integer_string(ss_value self) {
    return sf_str_fmt("%lld", self.value.integer);
}

sf_str ss_pointer_string(ss_value self) {
    return sf_str_fmt("%p", self.value.pointer);
}

sf_str ss_str_string(ss_value self) {
    return sf_str_dup(*(sf_str *)self.value.pointer);
}

const struct ss_pdata SS_PRIMITIVES[SS_PRIMITIVE_COUNT] = {
    [SS_PRIMITIVE_BYTE] = {
        .tt = SS_PRIMITIVE_BYTE,
        .name = "byte",
        .size = sizeof(ss_byte),
        .to_string = ss_number_string,
        .default_value.byte = 0x0,
    },
    [SS_PRIMITIVE_NUMBER] = {
        .tt = SS_PRIMITIVE_NUMBER,
        .name = "number",
        .size = sizeof(ss_number),
        .to_string = ss_number_string,
        .default_value.number = 0.0,
    },
    [SS_PRIMITIVE_INTEGER] = {
        .tt = SS_PRIMITIVE_INTEGER,
        .name = "integer",
        .size = sizeof(ss_integer),
        .to_string = ss_integer_string,
        .default_value.integer = 0,
    },
    [SS_PRIMITIVE_POINTER] = {
        .tt = SS_PRIMITIVE_POINTER,
        .name = "pointer",
        .size = sizeof(ss_pointer),
        .to_string = ss_pointer_string,
        .default_value.pointer = NULL,
    },
    [SS_PRIMITIVE_STR] = {
        .tt = SS_PRIMITIVE_STR,
        .name = "str",
        .size = sizeof(ss_pointer),
        .to_string = ss_str_string,
        .default_value.pointer = NULL,
    },
    [SS_PRIMITIVE_OBJ] = {
        .tt = SS_PRIMITIVE_OBJ,
        .name = "obj",
        .size = sizeof(ss_pointer),
        .to_string = ss_pointer_string,
        .default_value.pointer = NULL,
    },
};

void *ss_alloc(ss_type tt, size_t size) {
    char *block = calloc(1, sizeof(ss_block_header) + size);
    memcpy(block, &(ss_block_header){.tt = tt, .size = size, .ref_count = 1}, sizeof(ss_block_header));
    return block + sizeof(ss_block_header);
}

void ss_object_cleanup(void *ud, sf_str key, ss_value val) {
    (void)ud;
    sf_str_free(key);
    if (val.tt == SS_PRIMITIVE_POINTER || val.tt == SS_PRIMITIVE_STR || val.tt == SS_PRIMITIVE_OBJ) {
        ss_block_header *header = ss_alloc_header(val.value.pointer);
        if (--header->ref_count == 0) {
            if (val.tt == SS_PRIMITIVE_OBJ)
                ss_object_foreach((ss_object *)val.value.pointer, ss_object_cleanup, NULL);
            free(header);
        }
    }
}
