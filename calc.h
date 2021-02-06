#include <stdint.h>
#include "decimal.h"
#include <stdbool.h>

#define local static
typedef uint64_t U64;
typedef uint32_t U32;
typedef uint16_t U16;
typedef long long IntNum;
typedef char* Str;
typedef Num (*Op)(/*any args lol*/); //typedef Op(*) -> Num
struct OpDef;
typedef struct OpDef {Str name; Op func; Str doc; struct OpDef* next;} OpDef;

OpDef* addOp(OpDef** list, OpDef* def);

extern OpDef* infix;
extern OpDef* prefix;
extern OpDef* literal;

#define DEF_OP_L(name, expr...) local Num op_##name(Num a, Num b) { expr; }
#define DEF_OP(name, expr...) DEF_OP_L(name, return (expr);)
#define DEF_LIT(name, expr...) local Num lit_##name(Str* str) { expr; }

#define AUTORUN __attribute__((constructor)) static void init_##__line__(void)

#define ADD_OP(list, args...) addOp(&(list), &(OpDef){ args });
