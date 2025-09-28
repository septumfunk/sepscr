#ifndef SS_ASM_H
#define SS_ASM_H

#include "sepscr/backend/instruction.h"
#include "sepscr/backend/unit.h"

#define MAP_NAME ss_preprocessor
#define MAP_K sf_str
#define MAP_V sf_str
#define EQUAL_FN sf_str_eq
#define HASH_FN sf_str_hash
#include <sf/containers/map.h>
#define MAP_NAME ss_strint
#define MAP_K sf_str
#define MAP_V ss_integer
#define EQUAL_FN sf_str_eq
#define HASH_FN sf_str_hash
#include <sf/containers/map.h>
#define MAP_NAME ss_opmap
#define MAP_K sf_str
#define MAP_V const ss_instruction_info *
#define EQUAL_FN sf_str_eq
#define HASH_FN sf_str_hash
#include <sf/containers/map.h>
#define MAP_NAME ss_pmap
#define MAP_K sf_str
#define MAP_V const ss_type_info *
#define EQUAL_FN sf_str_eq
#define HASH_FN sf_str_hash
#include <sf/containers/map.h>

typedef struct {
    ss_preprocessor ppt;
    uint64_t const_count;
    ss_strint constants;
    ss_strint labels;

    ss_opmap opcodes;
    ss_pmap types;
} ss_assembler;

ss_assembler ss_assembler_new(void);
void ss_assembler_delete(ss_assembler *assembler);

ss_status ss_assemble(ss_assembler *assembler, ss_unit *unit, sf_str string);

#endif // SS_ASM
