#ifndef SS_INSTRUCTION_H
#define SS_INSTRUCTION_H

#include "value.h"

#define SS_BYTECODE_VERSION 0x01, 0x01, 0x00
#define SS_UNIT_HEADER { 'S', 'S', SS_BYTECODE_VERSION }
#define ss_operand(type) (*(type *)((state->ip += sizeof(type)) - sizeof(type)))

typedef enum {
    SS_OP_NOOP,
    SS_OP_CONST,
    SS_OP_POP,
    SS_OP_COPY,
    SS_OP_RET,
    SS_OP_JUMP,

    SS_OP_STR_ADD,
    SS_OP_STR_FROM,
    SS_OP_STR_PRINT,

    SS_OP_NUM_NEW,
    SS_OP_NUM_ADD,
    SS_OP_NUM_SUB,
    SS_OP_NUM_FROM,

    SS_OP_INT_NEW,
    SS_OP_INT_ADD,
    SS_OP_INT_SUB,
    SS_OP_INT_FROM,

    SS_OP_PTR_NEW,
    SS_OP_PTR_COPY,

    SS_OP_OBJ_NEW,
    SS_OP_OBJ_SET,
    SS_OP_OBJ_GET,

    SS_OP_UNKNOWN,
    SS_OP_COUNT,
} ss_opcode;

typedef struct {
    const char *asm_name;
    const ss_type tt;
} ss_operand;

typedef enum {
    SS_STATUS_OK,
    SS_STATUS_RETURN,
} ss_controlcode;

#define EXPECTED_NAME ss_status
#define EXPECTED_O ss_controlcode
#define EXPECTED_E sf_str
#include <sf/containers/expected.h>

struct ss_state;
typedef ss_status (*ss_instruction)(struct ss_state *state);

typedef struct {
    ss_opcode opcode;
    ss_instruction operation;
    const char *const asm_name;
    const struct {
        const uint8_t count;
        const ss_operand *const array;
    } operands;
} ss_instruction_info;

#define ss_ins_get(opcode) (&SS_INSTRUCTION_SET[opcode])
extern const ss_instruction_info SS_INSTRUCTION_SET[SS_OP_COUNT];

#endif // SS_INSTRUCTION
