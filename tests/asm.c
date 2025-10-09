#include "sepscr/backend/state.h"
#include "sf/fs.h"
#include "sf/result.h"
#include <sepscr/frontend/asm.h>
#include <stdio.h>

int main(void) {
    ss_state *state = ss_state_new();
    ss_unit unit = ss_unit_new();
    ss_assembler asm = ss_assembler_new();

    assert(sf_file_exists(sf_lit("test.ss")));
    size_t fs = (size_t)sf_file_size(sf_lit("test.ss"));
    uint8_t *f = malloc(fs + 1);
    f[fs] = '\0';
    sf_result res = sf_load_file(f, sf_lit("test.ss"));
    assert(res.is_ok);
    sf_discard(res);

    ss_status stat = ss_assemble(&asm, &unit, sf_own((char *)f));
    if (!stat.is_ok) {
        fprintf(stderr, "%s\n", stat.value.err.c_str);
        sf_str_free(stat.value.err);
    }

    ss_assembler_delete(&asm);
    stat = ss_dounit(state, &unit);
    if (!stat.is_ok) {
        fprintf(stderr, "%s\n", stat.value.err.c_str);
        sf_str_free(stat.value.err);
    }

    ss_state_delete(state);
    ss_unit_free(&unit);

    free(f);
    
}