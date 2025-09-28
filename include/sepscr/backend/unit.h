#ifndef SS_BYTECODE_H
#define SS_BYTECODE_H

#include <stdint.h>
#include <sf/str.h>
#include "value.h"
#include "instruction.h"

typedef struct {
    uint64_t offset;
    uint64_t line_number;
} ss_debug_chunk;

#define VEC_NAME ss_bytevec
#define VEC_T uint8_t
#include <sf/containers/vec.h>
#define VEC_NAME ss_constvec
#define VEC_T ss_value
#include <sf/containers/vec.h>
#define VEC_NAME ss_dbgvec
#define VEC_T ss_debug_chunk
#include <sf/containers/vec.h>

typedef struct {
    ss_bytevec bytecode;
    ss_constvec constants;
    ss_dbgvec dbg_chunks;
    uint64_t dbg_index;
    ss_integer entry;
} ss_unit;

#pragma pack(push, 1)
typedef struct {
    char header[5];
    uint8_t version[3];
    uint64_t loc_constants;
    uint64_t loc_bytecode;
    ss_integer entry;
} ss_unit_info;
#pragma pack(pop)

ss_unit ss_unit_new(void);
void ss_unit_free(ss_unit *self);
void ss_unit_save(ss_unit *self, const char *path);
ss_status ss_unit_load_file(ss_unit *self, sf_str path);

void ss_unit_push_debug(ss_unit *self, uint64_t offset, uint64_t line_number);
uint64_t ss_unit_push_constant(ss_unit *self, ss_value value);
ss_value ss_unit_get_constant(ss_unit *self, uint64_t offset);

#endif // SS_BYTECODE
