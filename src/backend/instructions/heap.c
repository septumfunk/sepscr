#include "sanitizers.h"
#include "sepscr/backend/instruction.h"
#include "sepscr/backend/state.h"
#include "sepscr/backend/value.h"
#include "sf/str.h"
#include <stdio.h>
#include <string.h>

void push_string(ss_state *state, sf_str to) {
    sf_str *str = ss_alloc(SS_PRIMITIVE_STR, sizeof(sf_str));
    memcpy(str, &to, sizeof(sf_str));
    ss_push(state, (ss_value){.tt = SS_PRIMITIVE_STR, .value.pointer = str});
}

ss_status ss_ins_str_add(ss_state *state) {
    ss_value val1 = ss_get(state, ss_operand(ss_integer));
    ss_value val2 = ss_get(state, ss_operand(ss_integer));

    if (val1.tt != SS_PRIMITIVE_STR)
        return ss_status_err(sf_str_fmt("[reg1] is of type '%s', expected string.", ss_typeinfo(val1.tt)->name));
    if (val2.tt != SS_PRIMITIVE_STR)
        return ss_status_err(sf_str_fmt("[reg2] is of type '%s', expected string.", ss_typeinfo(val2.tt)->name));

    __lsan_disable(); /// String is safely stored on the vm stack (ref-counted)
    push_string(state,
        sf_str_join(*(sf_str *)val1.value.pointer, *(sf_str *)val2.value.pointer)
    );
    __lsan_enable();
    return ss_status_ok(SS_STATUS_OK);
}

ss_status ss_ins_str_from(ss_state *state) {
    ss_value self = ss_get(state, ss_operand(ss_integer));
    sf_str out = SF_STR_EMPTY;
    switch (self.tt) {
        case SS_PRIMITIVE_BYTE:
            out = sf_str_fmt("%X", self.value.byte);
        case SS_PRIMITIVE_NUMBER:
            out = sf_str_fmt("%f", self.value.number);
        case SS_PRIMITIVE_INTEGER:
            out = sf_str_fmt("%lld", self.value.integer);
        case SS_PRIMITIVE_POINTER:
            out = sf_str_fmt("%p", self.value.pointer);
        case SS_PRIMITIVE_STR:
            out = sf_str_dup(*(sf_str *)self.value.pointer);
        default: break;
    }
    push_string(state, out);
    return ss_status_ok(SS_STATUS_OK);
}

ss_status ss_ins_str_print(ss_state *state) {
    ss_value val = ss_get(state, ss_operand(ss_integer));
    if (val.tt != SS_PRIMITIVE_STR)
        return ss_status_err(sf_str_fmt("[reg] is of type '%s', expected string.", ss_typeinfo(val.tt)->name));
    printf("%s", ((sf_str *)val.value.pointer)->c_str);
    return ss_status_ok(SS_STATUS_OK);
}

ss_status ss_ins_ptr_new(ss_state *state) {
    ss_integer tt = ss_operand(ss_integer);
    ss_integer size = ss_operand(ss_integer);

    if (tt < 0 || tt >= SS_PRIMITIVE_COUNT)
        return ss_status_err(sf_str_fmt("Unknown type-tag [%d].", tt));
    if (size < 0)
        return ss_status_err(sf_str_fmt("Invalid size [%d].", size));

    ss_push(state, (ss_value){.tt = SS_PRIMITIVE_POINTER, .value.pointer = ss_alloc((ss_type)tt, (size_t)size)});
    return ss_status_ok(SS_STATUS_OK);
}

ss_status ss_ins_ptr_copy(ss_state *state) {
    ss_value ptr = ss_get(state, ss_operand(ss_integer));
    ss_value value = ss_get(state, ss_operand(ss_integer));
    size_t offset = (size_t)ss_operand(ss_integer) * ss_typeinfo(value.tt)->size;

    if (ptr.tt != SS_PRIMITIVE_POINTER)
        return ss_status_err(sf_str_fmt("[ptr_reg] is of type '%s', expected pointer.", ss_typeinfo(ptr.tt)->name));
    ss_block_header *h = ss_alloc_header(ptr.value.pointer);
    if (h->tt != value.tt)
        return ss_status_err(sf_str_fmt("[value_reg] is of type '%s', pointer is type '%s'.", ss_typeinfo(ptr.tt)->name, ss_typeinfo(h->tt)->name));
    if (offset + ss_typeinfo(value.tt)->size > h->size)
        return ss_status_err(sf_lit("[offset] places [value_reg] outside of bounds of [ptr_reg]."));

    memcpy((char *)ptr.value.pointer + offset, &value.value, ss_typeinfo(value.tt)->size);
    return ss_status_ok(SS_STATUS_OK);
}


ss_status ss_ins_obj_new(ss_state *state) {
    __lsan_disable(); /// Object pointer is stored safely on the vm stack (ref-counted)
    ss_object *obj = ss_alloc(SS_PRIMITIVE_OBJ, sizeof(ss_object));
    *obj = ss_object_new();
    ss_push(state, (ss_value){.tt = SS_PRIMITIVE_OBJ, .value.pointer = obj});
    __lsan_enable();
    return ss_status_ok(SS_STATUS_OK);
}

ss_status ss_ins_obj_set(ss_state *state) {
    ss_value obj = ss_get(state, ss_operand(ss_integer));
    ss_value key = ss_get(state, ss_operand(ss_integer));
    ss_value val = ss_get(state, ss_operand(ss_integer));

    if (obj.tt != SS_PRIMITIVE_OBJ)
        return ss_status_err(sf_str_fmt("[obj_reg] is of type '%s', expected obj.", ss_typeinfo(obj.tt)->name));
    if (key.tt != SS_PRIMITIVE_STR)
        return ss_status_err(sf_str_fmt("[key_reg] is of type '%s', expected str.", ss_typeinfo(key.tt)->name));

    if (val.tt == SS_PRIMITIVE_POINTER || val.tt == SS_PRIMITIVE_STR || val.tt == SS_PRIMITIVE_OBJ)
        ss_alloc_header(val.value.pointer)->ref_count++;

    ss_object_set((ss_object *)obj.value.pointer, sf_str_dup(*(sf_str *)key.value.pointer), val);
    return ss_status_ok(SS_STATUS_OK);
}

ss_status ss_ins_obj_get(ss_state *state) {
    ss_value obj = ss_get(state, ss_operand(ss_integer));
    ss_value key = ss_get(state, ss_operand(ss_integer));

    if (obj.tt != SS_PRIMITIVE_OBJ)
        return ss_status_err(sf_str_fmt("[obj_reg] is of type '%s', expected obj.", ss_typeinfo(obj.tt)->name));
    if (key.tt != SS_PRIMITIVE_STR)
        return ss_status_err(sf_str_fmt("[key_reg] is of type '%s', expected str.", ss_typeinfo(key.tt)->name));

    ss_value val;
    if (!ss_object_get((ss_object *)obj.value.pointer, *(sf_str *)key.value.pointer, &val))
        return ss_status_err(sf_str_fmt("Key '%s' not found on object.", ((sf_str *)key.value.pointer)->c_str));

    if (val.tt == SS_PRIMITIVE_POINTER || val.tt == SS_PRIMITIVE_STR || val.tt == SS_PRIMITIVE_OBJ)
        ss_alloc_header(val.value.pointer)->ref_count++;
    ss_push(state, val);

    return ss_status_ok(SS_STATUS_OK);
}
