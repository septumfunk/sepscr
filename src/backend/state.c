#include <stdarg.h>
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>

#include "sepscr/backend/state.h"
#include "sepscr/backend/instruction.h"
#include "sepscr/backend/unit.h"
#include "sepscr/backend/value.h"
#include "sf/str.h"

#pragma clang diagnostic ignored "-Wformat-nonliteral"

ss_state *ss_state_new(void) {
    ss_state *state = calloc(1, sizeof(ss_state));
    *state = (ss_state) {
        .unit = NULL, // Empty by default
        .ip = NULL,
        .stack = ss_stack_new(),
    };

    return state;
}

void ss_state_delete(ss_state *state) {
    if (state->unit)
        ss_unit_free(state->unit);
    ss_stack_free(&state->stack);
}

void ss_push(ss_state *state, ss_value value) {
    ss_stack_push(&state->stack, value);
}

void ss_pop(ss_state *state, ss_integer count) {
    for (int i = 0; i < count; ++i) {
        ss_value val = ss_stack_pop(&state->stack);
        if (val.tt == SS_PRIMITIVE_POINTER || val.tt == SS_PRIMITIVE_STR || val.tt == SS_PRIMITIVE_OBJ) {
            ss_block_header *header = ss_alloc_header(val.value.pointer);
            if (--header->ref_count == 0) {
                if (val.tt == SS_PRIMITIVE_OBJ)
                    ss_object_foreach((ss_object *)val.value.pointer, ss_object_cleanup, NULL);
                free(header);
            }
        }
    }
}

ss_value ss_get(ss_state *state, ss_integer reg) {
    return ss_stack_get(&state->stack, (size_t)reg);
}

void ss_set(ss_state *state, ss_integer reg, ss_value value) {
    ss_stack_set(&state->stack, (size_t)reg, &value);
}

ss_value ss_loadconst(ss_state *state, ss_integer index) {
    return ss_unit_get_constant(state->unit, (uint64_t)index);
}

const ss_type_info *ss_gettype(ss_state *state, ss_integer reg) {
    return ss_typeinfo(ss_get(state, reg).tt);
}

void ss_print(ss_state *state, ss_integer reg) {
    ss_value val = ss_get(state, reg);
    sf_str str = ss_typeinfo(val.tt)->to_string(val);
    printf("PRNT %s\n", str.c_str);
    sf_str_free(str);
}

ss_status ss_dounit(ss_state *state, ss_unit *unit) {
    assert(unit->bytecode.count > 0);
    state->unit = unit;
    state->ip = unit->bytecode.data + unit->entry;

    register ss_opcode opcode;
    const register ss_instruction_info *ins;
    register ss_byte *ins_s;
    register size_t offset;

    while (state->ip < unit->bytecode.top) {
        ins_s = state->ip;
        offset = (size_t)(ins_s - unit->bytecode.data);
        opcode = *state->ip++;

        if (opcode >= SS_OP_UNKNOWN) {
            state->unit = NULL;
            return ss_status_err(sf_str_fmt("Unknown opcode: [%d]", opcode));
        }
        ins = ss_ins_get(opcode);

        #ifdef SS_DEBUG
        sf_str dasm = ss_disassemble(unit, offset);
        printf("[EXE] %s\n", dasm.c_str);
        sf_str_free(dasm);
        #endif

        ss_status res = ins->operation(state);
        if (!res.is_ok) {
            sf_str f = ss_disassemble(unit, offset);
            sf_str out = sf_str_fmt("[%s]\n", f.c_str);
            sf_str_free(f);

            f = sf_str_fmt(" ^\n %s\n", res.value.err.c_str);
            sf_str_append(&out, f);
            sf_str_free(f);

            f = ss_stackdump(state);
            sf_str_append(&out, f);
            sf_str_append(&out, sf_lit("\n"));
            sf_str_free(f);

            state->unit = NULL;
            sf_str_free(res.value.err);
            return ss_status_err(out);
        }
        if (res.is_ok && res.value.ok == SS_STATUS_RETURN) {
            // Temporary behavior, stop execution
            break;
        }
    }

    state->ip = NULL;
    state->unit = NULL;
    return ss_status_ok(SS_STATUS_OK);
}

void ss_push_instruction(ss_unit *unit, ss_opcode opcode, ss_value *operands) {
    ss_bytevec_push(&unit->bytecode, (uint8_t)opcode);
    const ss_instruction_info *ins = ss_ins_get(opcode);
    for (int i = 0; i < ins->operands.count; ++i)
        ss_bytevec_append(&unit->bytecode, (uint8_t *)&operands[i].value, ss_typeinfo(ins->operands.array[i].tt)->size);
}

sf_str ss_disassemble(ss_unit *unit, size_t offset) {
    ss_debug_chunk current_chunk = { .offset = UINT64_MAX };
    while (unit->dbg_chunks.count > 0 && unit->dbg_index < unit->dbg_chunks.count) {
        current_chunk = ss_dbgvec_get(&unit->dbg_chunks, unit->dbg_index);
        if (current_chunk.offset > offset) {
            unit->dbg_index--;
            current_chunk = ss_dbgvec_get(&unit->dbg_chunks, unit->dbg_index);
            if (current_chunk.offset < offset)
                break;
            continue;
        } else if (current_chunk.offset < offset && unit->dbg_index < unit->dbg_chunks.count - 1) {
            unit->dbg_index++;
            if (current_chunk.offset < offset)
                continue;
            else if (current_chunk.offset > offset) {
                unit->dbg_index--;
                break;
            } else break;
        } else break;
    }

    const ss_instruction_info *ins = ss_ins_get(SS_OP_UNKNOWN);
    const uint8_t *ip = unit->bytecode.data + offset;
    if (*ip < SS_OP_UNKNOWN) // Valid
        ins = ss_ins_get(*ip);

    sf_str output = sf_str_fmt("%04llu  ", offset);

    // Line Number (if possible)
    if (current_chunk.offset != UINT64_MAX) {
        sf_str line_no = sf_str_fmt("[%03llu]  ", current_chunk.line_number);
        sf_str_append(&output, line_no);
        sf_str_free(line_no);
    }

    // Instruction name
    sf_str_append(&output, sf_ref(ins->asm_name));
    ip++;

    // Operands
    for (int i = 0; i < ins->operands.count; ++i) {
        if (i == 0)
             sf_str_append(&output, sf_lit("  [ "));

        ss_operand oprnd = ins->operands.array[i];

        sf_str param = sf_str_fmt("%s: ", oprnd.asm_name);
        sf_str_append(&output, param);
        sf_str_free(param);

        const ss_type_info *ti = ss_typeinfo(oprnd.tt);
        ss_value val = { .tt = oprnd.tt };
        memcpy(&val.value, ip, ti->size);

        sf_str ops = ti->to_string(val);
        sf_str_append(&output, ops);
        sf_str_free(ops);

        if (i == ins->operands.count - 1)
            sf_str_append(&output, sf_lit(" ]"));
        else
            sf_str_append(&output, sf_lit(", "));

        ip += ti->size;
    }

    return output;
}

sf_str ss_disassemble_all(ss_unit *unit) {
    ss_byte *ip = unit->bytecode.data;
    sf_str output = SF_STR_EMPTY;
    while (ip < unit->bytecode.top) {
        sf_str line = ss_disassemble(unit, (size_t)(ip - unit->bytecode.data));
        sf_str_append(&output, line);
        sf_str_append(&output, sf_lit("\n"));
        sf_str_free(line);
    }
    return output;
}

sf_str ss_stackdump(ss_state *state) {
    sf_str out = sf_str_cdup("====STACK DUMP====\n");
    for (size_t i = 0; i < state->stack.count; ++i) {
        ss_value val = ss_stack_get(&state->stack, i);
        sf_str val_s = ss_typeinfo(val.tt)->to_string(val);
        sf_str line = sf_str_fmt("[%llu]: %s = %s\n", i, ss_typeinfo(val.tt)->name, val_s.c_str);
        sf_str_append(&out, line);
        sf_str_free(val_s);
        sf_str_free(line);
    }
    sf_str_append(&out, sf_lit("=================="));
    return out;
}
