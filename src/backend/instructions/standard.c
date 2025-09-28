#include "sepscr/backend/instruction.h"
#include "sepscr/backend/state.h"
#include "sepscr/backend/value.h"

ss_status ss_ins_noop(ss_state *state) {
    (void)state;
    return ss_status_ok(SS_STATUS_OK);
}

ss_status ss_ins_const(ss_state *state) {
    ss_integer offset = ss_operand(ss_integer);
    ss_value val = ss_loadconst(state, offset);
    ss_push(state, val);

    return ss_status_ok(SS_STATUS_OK);
}

ss_status ss_ins_pop(ss_state *state) {
    ss_integer count = ss_operand(ss_integer);
    ss_pop(state, count);
    return ss_status_ok(SS_STATUS_OK);
}

ss_status ss_ins_copy(ss_state *state) {
    ss_integer reg = ss_operand(ss_integer);
    ss_value to_copy = ss_get(state, reg);
    if (to_copy.tt == SS_PRIMITIVE_POINTER || to_copy.tt == SS_PRIMITIVE_STR || to_copy.tt == SS_PRIMITIVE_OBJ)
        ++ss_alloc_header(to_copy.value.pointer)->ref_count;
    ss_push(state, to_copy);
    return ss_status_ok(SS_STATUS_OK);
}

ss_status ss_ins_ret(ss_state *state) {
    (void)state;
    // TODO: Stack frames
    return ss_status_ok(SS_STATUS_RETURN);
}

ss_status ss_ins_jump(ss_state *state) {
    state->ip = state->unit->bytecode.data + ss_operand(ss_integer);
    return ss_status_ok(SS_STATUS_OK);
}