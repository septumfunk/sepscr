#include <assert.h>
#include <stdarg.h>
#include "sepscr/backend/instruction.h"
#include "sepscr/backend/state.h"

extern ss_status ss_ins_noop(ss_state *state);
extern ss_status ss_ins_const(ss_state *state);
extern ss_status ss_ins_pop(ss_state *state);
extern ss_status ss_ins_copy(ss_state *state);
extern ss_status ss_ins_ret(ss_state *state);
extern ss_status ss_ins_jump(ss_state *state);

extern ss_status ss_ins_str_add(ss_state *state);
extern ss_status ss_ins_str_from(ss_state *state);
extern ss_status ss_ins_str_print(ss_state *state);

extern ss_status ss_ins_num_new(ss_state *state);
extern ss_status ss_ins_num_add(ss_state *state);
extern ss_status ss_ins_num_sub(ss_state *state);
extern ss_status ss_ins_num_from(ss_state *state);

extern ss_status ss_ins_int_new(ss_state *state);
extern ss_status ss_ins_int_add(ss_state *state);
extern ss_status ss_ins_int_sub(ss_state *state);
extern ss_status ss_ins_int_from(ss_state *state);

extern ss_status ss_ins_ptr_new(ss_state *state);
extern ss_status ss_ins_ptr_copy(ss_state *state);

extern ss_status ss_ins_obj_new(ss_state *state);
extern ss_status ss_ins_obj_set(ss_state *state);
extern ss_status ss_ins_obj_get(ss_state *state);

const ss_instruction_info SS_INSTRUCTION_SET[SS_OP_COUNT] = {
    [SS_OP_NOOP] = {
        .opcode = SS_OP_NOOP,
        .operation = ss_ins_noop,
        .asm_name = "noop",
        .operands = {
            .count = 0,
            .array = NULL,
        }
    },
    [SS_OP_CONST] = {
        .opcode = SS_OP_CONST,
        .operation = ss_ins_const,
        .asm_name = "const",
        .operands = {
            .count = 1,
            .array = (const ss_operand[]) {
                {
                    .tt = SS_PRIMITIVE_INTEGER,
                    .asm_name = "index",
                },
            },
        }
    },
    [SS_OP_POP] = {
        .opcode = SS_OP_POP,
        .operation = ss_ins_pop,
        .asm_name = "pop",
        .operands = {
            .count = 1,
            .array = (const ss_operand[]) {
                {
                    .tt = SS_PRIMITIVE_INTEGER,
                    .asm_name = "count",
                },
            },
        }
    },
    [SS_OP_COPY] = {
        .opcode = SS_OP_COPY,
        .operation = ss_ins_copy,
        .asm_name = "copy",
        .operands = {
            .count = 1,
            .array = (const ss_operand[]) {
                {
                    .tt = SS_PRIMITIVE_INTEGER,
                    .asm_name = "reg",
                },
            },
        }
    },
    [SS_OP_RET] = {
        .opcode = SS_OP_RET,
        .operation = ss_ins_ret,
        .asm_name = "ret",
        .operands = {
            .count = 0,
            .array = NULL,
        }
    },
    [SS_OP_JUMP] = {
        .opcode = SS_OP_JUMP,
        .operation = ss_ins_jump,
        .asm_name = "jump",
        .operands = {
            .count = 1,
            .array = (const ss_operand[]) {
                {
                    .tt = SS_PRIMITIVE_INTEGER,
                    .asm_name = "loc",
                },
            },
        }
    },


    [SS_OP_STR_ADD] = {
        .opcode = SS_OP_STR_ADD,
        .operation = ss_ins_str_add,
        .asm_name = "str.add",
        .operands = {
            .count = 2,
            .array = (const ss_operand[]) {
                {
                    .tt = SS_PRIMITIVE_INTEGER,
                    .asm_name = "reg1",
                },
                {
                    .tt = SS_PRIMITIVE_INTEGER,
                    .asm_name = "reg2",
                }
            },
        }
    },
    [SS_OP_STR_FROM] = {
        .opcode = SS_OP_STR_FROM,
        .operation = ss_ins_str_from,
        .asm_name = "str.from",
        .operands = {
            .count = 1,
            .array = (const ss_operand[]) {
                {
                    .tt = SS_PRIMITIVE_INTEGER,
                    .asm_name = "reg",
                }
            }
        },
    },
    [SS_OP_STR_PRINT] = {
        .opcode = SS_OP_STR_PRINT,
        .operation = ss_ins_str_print,
        .asm_name = "str.print",
        .operands = {
            .count = 1,
            .array = (const ss_operand[]) {
                {
                    .tt = SS_PRIMITIVE_INTEGER,
                    .asm_name = "reg",
                }
            },
        }
    },


    [SS_OP_NUM_NEW] = {
        .opcode = SS_OP_NUM_NEW,
        .operation = ss_ins_num_new,
        .asm_name = "num.new",
        .operands = {
            .count = 1,
            .array = (const ss_operand[]) {
                {
                    .tt = SS_PRIMITIVE_NUMBER,
                    .asm_name = "value",
                }
            }
        },
    },
    [SS_OP_NUM_ADD] = {
        .opcode = SS_OP_NUM_ADD,
        .operation = ss_ins_num_add,
        .asm_name = "num.add",
        .operands = {
            .count = 2,
            .array = (const ss_operand[]) {
                {
                    .tt = SS_PRIMITIVE_INTEGER,
                    .asm_name = "reg1",
                },
                {
                    .tt = SS_PRIMITIVE_INTEGER,
                    .asm_name = "reg2",
                }
            },
        }
    },
    [SS_OP_NUM_SUB] = {
        .opcode = SS_OP_NUM_SUB,
        .operation = ss_ins_num_sub,
        .asm_name = "num.sub",
        .operands = {
            .count = 2,
            .array = (const ss_operand[]) {
                {
                    .tt = SS_PRIMITIVE_INTEGER,
                    .asm_name = "reg1",
                },
                {
                    .tt = SS_PRIMITIVE_INTEGER,
                    .asm_name = "reg2",
                }
            },
        }
    },
    [SS_OP_NUM_FROM] = {
        .opcode = SS_OP_NUM_FROM,
        .operation = ss_ins_num_from,
        .asm_name = "num.from",
        .operands = {
            .count = 1,
            .array = (const ss_operand[]) {
                {
                    .tt = SS_PRIMITIVE_INTEGER,
                    .asm_name = "reg",
                }
            }
        },
    },


    [SS_OP_INT_NEW] = {
        .opcode = SS_OP_INT_NEW,
        .operation = ss_ins_int_new,
        .asm_name = "int.new",
        .operands = {
            .count = 1,
            .array = (const ss_operand[]) {
                {
                    .tt = SS_PRIMITIVE_INTEGER,
                    .asm_name = "value",
                }
            }
        },
    },
    [SS_OP_INT_ADD] = {
        .opcode = SS_OP_INT_ADD,
        .operation = ss_ins_int_add,
        .asm_name = "int.add",
        .operands = {
            .count = 2,
            .array = (const ss_operand[]) {
                {
                    .tt = SS_PRIMITIVE_INTEGER,
                    .asm_name = "reg1",
                },
                {
                    .tt = SS_PRIMITIVE_INTEGER,
                    .asm_name = "reg2",
                }
            },
        }
    },
    [SS_OP_INT_SUB] = {
        .opcode = SS_OP_INT_SUB,
        .operation = ss_ins_int_sub,
        .asm_name = "int.sub",
        .operands = {
            .count = 2,
            .array = (const ss_operand[]) {
                {
                    .tt = SS_PRIMITIVE_INTEGER,
                    .asm_name = "reg1",
                },
                {
                    .tt = SS_PRIMITIVE_INTEGER,
                    .asm_name = "reg2",
                }
            },
        }
    },
    [SS_OP_INT_FROM] = {
        .opcode = SS_OP_INT_FROM,
        .operation = ss_ins_int_from,
        .asm_name = "int.from",
        .operands = {
            .count = 1,
            .array = (const ss_operand[]) {
                {
                    .tt = SS_PRIMITIVE_INTEGER,
                    .asm_name = "reg",
                }
            }
        },
    },

    [SS_OP_PTR_NEW] = {
        .opcode = SS_OP_PTR_NEW,
        .operation = ss_ins_ptr_new,
        .asm_name = "ptr.new",
        .operands = {
            .count = 2,
            .array = (const ss_operand[]) {
                {
                    .tt = SS_PRIMITIVE_INTEGER,
                    .asm_name = "tt",
                },
                {
                    .tt = SS_PRIMITIVE_INTEGER,
                    .asm_name = "size",
                }
            }
        },
    },
    [SS_OP_PTR_COPY] = {
        .opcode = SS_OP_PTR_COPY,
        .operation = ss_ins_ptr_copy,
        .asm_name = "ptr.copy",
        .operands = {
            .count = 3,
            .array = (const ss_operand[]) {
                {
                    .tt = SS_PRIMITIVE_INTEGER,
                    .asm_name = "ptr_reg",
                },
                {
                    .tt = SS_PRIMITIVE_INTEGER,
                    .asm_name = "value_reg",
                },
                {
                    .tt = SS_PRIMITIVE_INTEGER,
                    .asm_name = "offset",
                }
            }
        },
    },


    [SS_OP_OBJ_NEW] = {
        .opcode = SS_OP_OBJ_NEW,
        .operation = ss_ins_obj_new,
        .asm_name = "obj.new",
        .operands = {
            .count = 0,
            .array = NULL,
        },
    },
    [SS_OP_OBJ_SET] = {
        .opcode = SS_OP_OBJ_SET,
        .operation = ss_ins_obj_set,
        .asm_name = "obj.set",
        .operands = {
            .count = 3,
            .array = (const ss_operand[]) {
                {
                    .tt = SS_PRIMITIVE_INTEGER,
                    .asm_name = "obj_reg",
                },
                {
                    .tt = SS_PRIMITIVE_INTEGER,
                    .asm_name = "key_reg",
                },
                {
                    .tt = SS_PRIMITIVE_INTEGER,
                    .asm_name = "value_reg",
                }
            },
        },
    },
    [SS_OP_OBJ_GET] = {
        .opcode = SS_OP_OBJ_GET,
        .operation = ss_ins_obj_get,
        .asm_name = "obj.get",
        .operands = {
            .count = 2,
            .array = (const ss_operand[]) {
                {
                    .tt = SS_PRIMITIVE_INTEGER,
                    .asm_name = "obj_reg",
                },
                {
                    .tt = SS_PRIMITIVE_INTEGER,
                    .asm_name = "key_reg",
                }
            },
        },
    },


    [SS_OP_UNKNOWN] = {
        .opcode = SS_OP_UNKNOWN,
        .operation = ss_ins_noop,
        .asm_name = "???",
        .operands = {
            .count = 0,
            .array = NULL,
        }
    },
};
