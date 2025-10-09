#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "sepscr/frontend/asm.h"
#include "sepscr/backend/instruction.h"
#include "sepscr/backend/value.h"
#include "sf/str.h"

ss_assembler ss_assembler_new(void) {
    ss_assembler asmb = {
        .ppt = ss_preprocessor_new(),
        .const_count = 0,
        .constants = ss_strint_new(),
        .labels = ss_strint_new(),

        .opcodes = ss_opmap_new(),
        .types = ss_pmap_new(),
    };

    for (uint64_t i = 0; i < SS_OP_COUNT; ++i) {
        const ss_instruction_info *ins = ss_ins_get(i);
        ss_opmap_set(&asmb.opcodes, sf_ref(ins->asm_name), ins);
    }

    for (uint64_t i = 0; i < SS_PRIMITIVE_COUNT; ++i) {
        const ss_type_info *pt = ss_typeinfo(i);
        ss_pmap_set(&asmb.types, sf_ref(pt->name), pt);
    }

    return asmb;
}

void _ppt_cleanup(void *_, sf_str key, sf_str value) {
    sf_str_free(key);
    sf_str_free(value);
}
void _strint_cleanup(void *_, sf_str key, ss_integer value) {
    (void)value;
    sf_str_free(key);
}

void ss_assembler_delete(ss_assembler *assembler) {
    ss_preprocessor_foreach(&assembler->ppt, _ppt_cleanup, NULL);
    ss_preprocessor_free(&assembler->ppt);

    ss_strint_foreach(&assembler->labels, _strint_cleanup, NULL);
    ss_strint_free(&assembler->labels);
    ss_strint_foreach(&assembler->constants, _strint_cleanup, NULL);
    ss_strint_free(&assembler->constants);

    ss_opmap_free(&assembler->opcodes);
    ss_pmap_free(&assembler->types);
}

ss_status ss_assemble(ss_assembler *assembler, ss_unit *unit, sf_str string) {
    // Remove windows line breaks
    sf_str lb_removed = sf_str_dup(string);
    for (char *c = lb_removed.c_str; c < lb_removed.c_str + lb_removed.len; ++c)
        *c = *c == '\r' ? ' ' : *c;

    sf_str cm_removed = SF_STR_EMPTY;
    for (char *line = strtok(lb_removed.c_str, "\n"); line; line = strtok(NULL, "\n")) {
        if (!line)
            return ss_status_err(sf_lit("Unexpected EOF"));

        for (char *c = line; c < line + strlen(line); ++c) {
            if (*c == ';')
                goto next;
            char ap[2] = { *c, '\0' };
            sf_str_append(&cm_removed, sf_ref(ap));
        }
    next:
        sf_str_append(&cm_removed, sf_lit(" "));
        continue;
    }
    sf_str_free(lb_removed);

    // Escape
    for (char *c = cm_removed.c_str; c < cm_removed.c_str + cm_removed.len; ++c) {
        if (*c == '\\') {
            if (*(c + 1) == 'n') { // Line Break
                *c = '\n';
                memmove(c + 1, c + 2, strlen(c) - 1);
            }
        }
    }

    // Preprocess
    sf_str preprocessed = SF_STR_EMPTY;
    for (char *token = strtok(cm_removed.c_str, " "); token; token = strtok(NULL, " ")) {
        if (!token)
            return ss_status_err(sf_lit("Unexpected EOF"));

        if (strcmp(token, "#define") == 0) {
            char *k = strtok(NULL, " ");
            char *v = strtok(NULL, "\n");
            if (!k || !v)
                return ss_status_err(sf_lit("Unexpected EOF"));

            sf_str key = sf_str_cdup(k);
            sf_str value = sf_str_cdup(v);
            ss_preprocessor_set(&assembler->ppt, key, value);
            continue;
        }

        sf_str repl;
        if (ss_preprocessor_get(&assembler->ppt, sf_ref(token), &repl)) {
            sf_str_append(&preprocessed, repl);
            continue;
        }
        sf_str_append(&preprocessed, sf_ref(token));
        sf_str_append(&preprocessed, sf_lit(" "));
    }
    sf_str_free(cm_removed);

    // Assemble
    sf_str string_work = SF_STR_EMPTY;
    char *sw_name = NULL;

    for (char *token = strtok(preprocessed.c_str, " "); token; token = strtok(NULL, " ")) {
        if (!token)
            return ss_status_err(sf_lit("Unexpected EOF"));
        if (sw_name)
            goto string_continue;

        if (strcmp(token, "#const") == 0) {
            char *name = strtok(NULL, " ");
            char *type = strtok(NULL, " ");

            if (!name || !type)
                return ss_status_err(sf_lit("Unexpected EOF"));

            const ss_type_info *pd;
            if (!ss_pmap_get(&assembler->types, sf_ref(type), &pd))
                return ss_status_err(sf_str_fmt("Compile Error: Unknown Primitive '%s'\n", type));

            char *value = strtok(NULL, " ");
            if (!value)
                return ss_status_err(sf_lit("Unexpected EOF"));

            ss_value v = {.tt = pd->tt};
            switch (v.tt) {
                case SS_PRIMITIVE_BYTE:
                    v.value.byte = (ss_byte)atoll(value);
                    break;
                case SS_PRIMITIVE_NUMBER:
                    v.value.number = (ss_number)atof(value);
                    break;
                case SS_PRIMITIVE_INTEGER:
                    v.value.integer = (ss_integer)atoll(value);
                    break;
                case SS_PRIMITIVE_POINTER:
                    v.value.pointer = (ss_pointer)atoll(value); // NOLINT(performance-no-int-to-ptr)
                    break;
                case SS_PRIMITIVE_STR:
                string_continue: {
                    char *tk = token;
                    if (!sw_name) {
                        tk = value;
                        if (tk[0] != '"')
                            return ss_status_err(sf_lit("String constants require opening quotes."));
                        sw_name = name;
                        if (strlen(tk) == 1)
                            continue;
                    }

                    if (tk[0] == '"') {
                        tk = tk + 1;
                    } else sf_str_append(&string_work, sf_lit(" "));

                    if (tk[strlen(tk) - 1] == '"') {
                        tk[strlen(tk) - 1] = '\0';

                        __lsan_disable(); /// String stored safely in the constant table.
                        sf_str_append(&string_work, sf_ref(tk));
                        sf_str *s = ss_alloc(SS_PRIMITIVE_STR, sizeof(sf_str));
                        *s = string_work;
                        ss_strint_set(&assembler->constants, sf_str_cdup(sw_name), (ss_integer)assembler->const_count++);
                        ss_unit_push_constant(unit, (ss_value){.tt = SS_PRIMITIVE_STR, .value.pointer = s});
                        __lsan_enable();

                        string_work = SF_STR_EMPTY;
                        sw_name = NULL;
                        continue;
                    }
                    sf_str_append(&string_work, sf_ref(tk));
                    break;
                }
                default:
                    return ss_status_err(sf_str_fmt("Compile Error: Unknown Const Primitive [%d]\n", pd->tt));
            }

            if (!string_work.c_str) {
                ss_strint_set(&assembler->constants, sf_str_cdup(name), (ss_integer)assembler->const_count++);
                ss_unit_push_constant(unit, v);
            }
            continue;
        }

        if (*(token + strlen(token) - 1) == ':') { // Label
            *(token + strlen(token) - 1) = 0;
            ss_strint_set(&assembler->labels, sf_str_cdup(token), (ss_integer) { (ss_integer)unit->bytecode.count });
            continue;
        }

        const ss_instruction_info *ins;
        if (!ss_opmap_get(&assembler->opcodes, sf_ref(token), &ins)) // Unknown opcode
            return ss_status_err(sf_str_fmt("Compile Error: Unknown Opcode '%s'\n", token));

        ss_bytevec_push(&unit->bytecode, (uint8_t)ins->opcode);
        for (uint64_t i = 0; i < ins->operands.count; ++i) {
            const ss_operand *oprnd = &ins->operands.array[i];
            char *value = strtok(NULL, " ");
            if (!value)
                return ss_status_err(sf_lit("Unexpected EOF"));

            ss_integer idx;
            if (ss_strint_get(&assembler->constants, sf_ref(value), &idx))
                ss_bytevec_append(&unit->bytecode, (uint8_t *)&idx, sizeof(ss_integer));
            else switch (oprnd->tt) {
                case SS_PRIMITIVE_BYTE: {
                    ss_byte b = (ss_byte)atoll(value);
                    ss_bytevec_append(&unit->bytecode, &b, sizeof(b));
                    break;
                }
                case SS_PRIMITIVE_NUMBER: {
                    ss_number num = (ss_number)atof(value);
                    ss_bytevec_append(&unit->bytecode, (uint8_t *)&num, sizeof(num));
                    break;
                }
                case SS_PRIMITIVE_INTEGER: {
                    ss_integer offset;
                    if (ss_strint_get(&assembler->labels, sf_ref(value), &offset)) {
                        ss_bytevec_append(&unit->bytecode, (uint8_t *)&offset, sizeof(ss_integer));
                        break;
                    }
                    ss_integer in = (ss_integer)atoll(value);
                    ss_bytevec_append(&unit->bytecode, (uint8_t *)&in, sizeof(in));
                    break;
                }
                case SS_PRIMITIVE_POINTER:
                case SS_PRIMITIVE_STR:
                case SS_PRIMITIVE_OBJ: {
                    ss_pointer ptr = (ss_pointer)strtoull(value, NULL, 0); // NOLINT(performance-no-int-to-ptr)
                    ss_bytevec_append(&unit->bytecode, (uint8_t *)&ptr, sizeof(ptr));
                    break;
                }
                default: return ss_status_err(sf_str_fmt("Compile Error: Unknown Primitive [%d]\n", oprnd->tt));; //! WHAT
            }
        }
    }

    if (string_work.c_str)
        return ss_status_err(sf_lit("Unterminated String"));

    // Entry
    ss_integer main;
    unit->entry = ss_strint_get(&assembler->labels, sf_lit("main"), &main) ? main : 0;

    sf_str_free(preprocessed);
    return ss_status_ok(SS_STATUS_OK);
}
