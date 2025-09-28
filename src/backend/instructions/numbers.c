#include "sepscr/backend/instruction.h"
#include "sepscr/backend/state.h"
#include "sepscr/backend/value.h"
#include <stdlib.h>

ss_status ss_ins_num_new(ss_state *state) {
    ss_push(state, (ss_value){.tt = SS_PRIMITIVE_NUMBER, .value.number = ss_operand(ss_number), });
    return ss_status_ok(SS_STATUS_OK);
}

ss_status ss_ins_num_add(ss_state *state) {
    ss_value val1 = ss_get(state, ss_operand(ss_integer));
    ss_value val2 = ss_get(state, ss_operand(ss_integer));

    if (val1.tt != SS_PRIMITIVE_NUMBER)
        return ss_status_err(sf_str_fmt("[reg1] is of type '%s', expected number.", ss_typeinfo(val1.tt)->name));
    if (val2.tt != SS_PRIMITIVE_NUMBER)
        return ss_status_err(sf_str_fmt("[reg2] is of type '%s', expected number.", ss_typeinfo(val2.tt)->name));

    ss_push(state, (ss_value){.tt = SS_PRIMITIVE_NUMBER, .value.number = val1.value.number + val2.value.number});
    return ss_status_ok(SS_STATUS_OK);
}

ss_status ss_ins_num_sub(ss_state *state) {
    ss_value val1 = ss_get(state, ss_operand(ss_integer));
    ss_value val2 = ss_get(state, ss_operand(ss_integer));

    if (val1.tt != SS_PRIMITIVE_NUMBER)
        return ss_status_err(sf_str_fmt("[reg1] is of type '%s', expected number.", ss_typeinfo(val1.tt)->name));
    if (val2.tt != SS_PRIMITIVE_NUMBER)
        return ss_status_err(sf_str_fmt("[reg2] is of type '%s', expected number.", ss_typeinfo(val2.tt)->name));

    ss_push(state, (ss_value){.tt = SS_PRIMITIVE_NUMBER, .value.number = val1.value.number - val2.value.number});
    return ss_status_ok(SS_STATUS_OK);
}

ss_status ss_ins_num_from(ss_state *state) {
    ss_value self = ss_get(state, ss_operand(ss_integer));
    switch (self.tt) {
        case SS_PRIMITIVE_BYTE:
            ss_push(state, (ss_value){.value.number = self.value.byte, .tt = SS_PRIMITIVE_NUMBER});
        case SS_PRIMITIVE_NUMBER:
            ss_push(state, self);
        case SS_PRIMITIVE_INTEGER:
            ss_push(state, (ss_value){.value.number = (ss_number)self.value.integer, .tt = SS_PRIMITIVE_NUMBER});
        case SS_PRIMITIVE_POINTER:
            ss_push(state, (ss_value){.value.number = (ss_number)(ss_integer)self.value.pointer, .tt = SS_PRIMITIVE_NUMBER});
        case SS_PRIMITIVE_STR:
            ss_push(state, (ss_value){.value.number = atof(((sf_str *)self.value.pointer)->c_str), .tt = SS_PRIMITIVE_NUMBER});
        default: break;
    }
    return ss_status_ok(SS_STATUS_OK);
}

ss_status ss_ins_int_new(ss_state *state) {
    ss_push(state, (ss_value){.tt = SS_PRIMITIVE_INTEGER, .value.integer = ss_operand(ss_integer), });
    return ss_status_ok(SS_STATUS_OK);
}

ss_status ss_ins_int_add(ss_state *state) {
    ss_value val1 = ss_get(state, ss_operand(ss_integer));
    ss_value val2 = ss_get(state, ss_operand(ss_integer));

    if (val1.tt != SS_PRIMITIVE_INTEGER)
        return ss_status_err(sf_str_fmt("[reg1] is of type '%s', expected integer.", ss_typeinfo(val1.tt)->name));
    if (val2.tt != SS_PRIMITIVE_INTEGER)
        return ss_status_err(sf_str_fmt("[reg2] is of type '%s', expected integer.", ss_typeinfo(val2.tt)->name));

    ss_push(state, (ss_value){.tt = SS_PRIMITIVE_INTEGER, .value.integer = val1.value.integer + val2.value.integer});
    return ss_status_ok(SS_STATUS_OK);
}

ss_status ss_ins_int_sub(ss_state *state) {
    ss_value val1 = ss_get(state, ss_operand(ss_integer));
    ss_value val2 = ss_get(state, ss_operand(ss_integer));

    if (val1.tt != SS_PRIMITIVE_INTEGER)
        return ss_status_err(sf_str_fmt("[reg1] is of type '%s', expected integer.", ss_typeinfo(val1.tt)->name));
    if (val2.tt != SS_PRIMITIVE_INTEGER)
        return ss_status_err(sf_str_fmt("[reg2] is of type '%s', expected integer.", ss_typeinfo(val2.tt)->name));

    ss_push(state, (ss_value){.tt = SS_PRIMITIVE_INTEGER, .value.integer = val1.value.integer - val2.value.integer});
    return ss_status_ok(SS_STATUS_OK);
}

ss_status ss_ins_int_from(ss_state *state) {
    ss_value self = ss_get(state, ss_operand(ss_integer));
    switch (self.tt) {
        case SS_PRIMITIVE_BYTE:
            ss_push(state, (ss_value){.value.integer = self.value.byte, .tt = SS_PRIMITIVE_INTEGER});
        case SS_PRIMITIVE_NUMBER:
            ss_push(state, (ss_value){.value.integer = (ss_integer)self.value.number, .tt = SS_PRIMITIVE_INTEGER});
        case SS_PRIMITIVE_INTEGER:
            ss_push(state, self);
        case SS_PRIMITIVE_POINTER:
            ss_push(state, (ss_value){.value.integer = (ss_integer)self.value.pointer, .tt = SS_PRIMITIVE_INTEGER});
        case SS_PRIMITIVE_STR:
            ss_push(state, (ss_value){.value.integer = atoll(((sf_str *)self.value.pointer)->c_str), .tt = SS_PRIMITIVE_INTEGER});
        default: break;
    }
    return ss_status_ok(SS_STATUS_OK);
}