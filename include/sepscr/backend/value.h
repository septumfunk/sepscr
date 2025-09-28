#ifndef SS_VALUE_H
#define SS_VALUE_H

#include <stdint.h>
#include <sf/str.h>

typedef uint8_t ss_byte;
typedef double ss_number;
typedef int64_t ss_integer;
typedef void *ss_pointer;

/// Primitive types
typedef enum {
    SS_PRIMITIVE_BYTE,
    SS_PRIMITIVE_NUMBER,
    SS_PRIMITIVE_INTEGER,
    SS_PRIMITIVE_POINTER,
    SS_PRIMITIVE_STR,
    SS_PRIMITIVE_OBJ,

    SS_PRIMITIVE_COUNT,
} ss_type;

/// The inner union value of a primitive
typedef union {
    ss_byte byte;
    ss_number number;
    ss_integer integer;
    ss_pointer pointer;
} ss_innerval;

/// Primitive value
typedef struct {
    ss_type tt;
    ss_innerval value;
} ss_value;

/// Header for all allocated blocks denoting allocation info.
typedef struct {
    ss_type tt;
    uint64_t size;
    uint64_t ref_count;
} ss_block_header;

/// Primitive operations
typedef sf_str (*ss_p_tostring)(ss_value self);

typedef struct ss_pdata {
    const ss_type tt;
    const char *const name;
    const size_t size;
    const ss_p_tostring to_string;
    const ss_innerval default_value;
} ss_type_info;

extern const ss_type_info SS_PRIMITIVES[SS_PRIMITIVE_COUNT];
#define ss_typeinfo(tt) (&SS_PRIMITIVES[tt])

void *ss_alloc(ss_type tt, size_t size);
#define ss_alloc_header(ptr) ((ss_block_header *)((char *)ptr - sizeof(ss_block_header)))

#define MAP_NAME ss_object
#define MAP_K sf_str
#define MAP_V ss_value
#define EQUAL_FN sf_str_eq
#define HASH_FN sf_str_hash
#include <sf/containers/map.h>
void ss_object_cleanup(void *ud, sf_str key, ss_value val);

#endif // SS_VALUE