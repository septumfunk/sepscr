#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sf/str.h>
#include <sf/fs.h>

#include "sepscr/backend/instruction.h"
#include "sepscr/backend/unit.h"
#include "sepscr/backend/value.h"
#include "sf/result.h"

ss_unit ss_unit_new(void) {
    return (ss_unit) {
        .bytecode = ss_bytevec_new(),
        .constants = ss_constvec_new(),
        .dbg_chunks = ss_dbgvec_new(),
        .entry = 0,
    };
}

void ss_unit_free(ss_unit *self) {
    ss_bytevec_free(&self->bytecode);
    ss_constvec_free(&self->constants);
    ss_dbgvec_free(&self->dbg_chunks);
}

void ss_unit_save(ss_unit *self, const char *path) {
    FILE *f = fopen(path, "wb");
    if (f == NULL) {
        fprintf(stderr, "Failed to open file '%s' for writing", path);
        return;
    }

    size_t constants_size = 0;
    for (uint64_t i = 0; i < self->constants.count; ++i) {
        ss_value val = ss_unit_get_constant(self, i);
        if (val.tt == SS_PRIMITIVE_STR)
            constants_size += sizeof(val.tt) + sizeof(size_t) + ((sf_str *)val.value.pointer)->len;
        else constants_size += sizeof(val.tt) + ss_typeinfo(val.tt)->size;
    }

    size_t bytecode_size = self->bytecode.count;
    size_t buffer_size = sizeof(ss_unit_info) + constants_size + bytecode_size;
    uint8_t *buffer = calloc(1, buffer_size);
    ss_unit_info header = {
        .header = SS_UNIT_HEADER,
        .version = {SS_BYTECODE_VERSION},
        .loc_constants = sizeof(ss_unit_info), // After Header
        .loc_bytecode = sizeof(ss_unit_info) + constants_size,
        .entry = self->entry,
    };
    memcpy(buffer, &header, sizeof(ss_unit_info));

    uint8_t *write = buffer + sizeof(ss_unit_info);
    for (uint64_t i = 0; i < self->constants.count; ++i) {
        ss_value val = ss_unit_get_constant(self, i);
        memcpy(write, &val.tt, sizeof(val.tt));
        write += sizeof(val.tt);
        if (val.tt == SS_PRIMITIVE_STR) {
            sf_str *str = val.value.pointer;
            memcpy(write, &str->len, sizeof(str->len));
            write += sizeof(str->len);
            memcpy(write, &str->c_str, str->len);
            write += str->len;
        } else {
            memcpy(write, &val.value, ss_typeinfo(val.tt)->size);
            write += ss_typeinfo(val.tt)->size;
        }
    }

    memmove(buffer + header.loc_bytecode, self->bytecode.data, bytecode_size);

    if (fwrite(buffer, sizeof(uint8_t), buffer_size, f) != buffer_size)
        fprintf(stderr, "Failed to save bytecode unit to file '%s'", path);

    free(buffer);
    fclose(f);
}

ss_status ss_unit_load_file(ss_unit *self, sf_str path) {
    *self = ss_unit_new();

    size_t filesize = 0;
    if (!sf_file_exists(path))
        return ss_status_err(sf_str_fmt("Unit file '%s' not found.", path.c_str));
    uint8_t *file = malloc((size_t)sf_file_size(path));
    sf_result res = sf_load_file(file, path);
    if (!res.is_ok) {
        ss_status stat = ss_status_err(sf_str_fmt("Unit file '%s' couldn't be loaded: %s", path.c_str, res.err));
        sf_discard(res);
        return stat;
    }

    ss_unit_info header = *(ss_unit_info *)file;

    uint8_t *constants = file + header.loc_constants;
    size_t constant_size = header.loc_bytecode - header.loc_constants;
    uint8_t *bytecode = file + header.loc_bytecode;
    size_t bytecode_size = filesize - constant_size - sizeof(header);

    if ((constant_size > UINT64_MAX - bytecode_size) || (sizeof(header) + constant_size + bytecode_size != filesize)) {
        free(file);
        return ss_status_err(sf_str_fmt("File '%s' is not a valid unit file.", path.c_str));
    }

    while (constants < bytecode) {
        ss_value val = { .tt = *constants++ };
        if (val.tt == SS_PRIMITIVE_STR) {
            size_t len = *(size_t *)constants;
            constants += sizeof(size_t);

            char *new = calloc(1, len + 1);
            memcpy(new, constants, len);
            constants += len;

            sf_str *str = ss_alloc(SS_PRIMITIVE_STR, sizeof(sf_str));
            *str = sf_own(new);

            ss_unit_push_constant(self, (ss_value){.tt = SS_PRIMITIVE_STR, .value.pointer = str});
        } else {
            memcpy(&val.value, constants, ss_typeinfo(val.tt)->size);
            ss_unit_push_constant(self, val);
            constants += ss_typeinfo(val.tt)->size;
        }
    }
    ss_bytevec_append(&self->bytecode, bytecode, bytecode_size);
    free(file);

    self->entry = header.entry;

    return ss_status_ok(SS_STATUS_OK);
}

void ss_unit_push_debug(ss_unit *self, uint64_t offset, uint64_t line_number) {
    ss_dbgvec_push(&self->dbg_chunks, (ss_debug_chunk) {
        .offset = offset,
        .line_number = line_number,
    });
}

uint64_t ss_unit_push_constant(ss_unit *self, ss_value value) {
    ss_constvec_push(&self->constants, value);
    uint64_t offset = self->constants.count - 1;
    return offset;
}

ss_value ss_unit_get_constant(ss_unit *self, uint64_t offset) {
    return ss_constvec_get(&self->constants, offset);
}
