#ifndef SS_STATE_H
#define SS_STATE_H

#include "value.h"
#include "unit.h"

#define VEC_NAME ss_stack
#define VEC_T ss_value
#include <sf/containers/vec.h>

#define SS_STACK_MAX 1024
#define SS_DEBUG

typedef struct {
    char *str;
    ss_byte *ins_s;
} ss_runtime_error;

typedef struct ss_state {
    ss_unit *unit;
    ss_byte *ip;
    ss_stack stack;
} ss_state;

ss_state *ss_state_new(void);
void ss_state_delete(ss_state *state);

void ss_push(ss_state *state, ss_value value);
void ss_pop(ss_state *state, ss_integer count);
ss_value ss_get(ss_state *state, ss_integer reg);
void ss_set(ss_state *state, ss_integer reg, ss_value value);
ss_value ss_loadconst(ss_state *state, ss_integer index);

const ss_type_info *ss_gettype(ss_state *state, ss_integer reg);
void ss_print(ss_state *state, ss_integer reg);

ss_status ss_dounit(ss_state *state, ss_unit *unit);
void ss_push_instruction(ss_unit *unit, ss_opcode opcode, ss_value *operands);
sf_str ss_disassemble(ss_unit *unit, size_t offset);
sf_str ss_disassemble_all(ss_unit *unit);
sf_str ss_stackdump(ss_state *state);

#endif // SS_STATE
