#include <stdint.h>
#include "decimal.h"
#include <stdbool.h>
#include <setjmp.h>

#define local static
typedef uint64_t U64;
typedef uint32_t U32;
typedef uint16_t U16;
typedef long long IntNum;
typedef char* Str;
typedef Num (*Op)(/*any args lol*/); //typedef Op(*) -> Num
struct OpDef;
typedef struct OpDef {Str name; Op func; Str doc; struct OpDef* next;} OpDef;

#define AUTORUN __attribute__((constructor)) static void init_##__line__(void)

extern jmp_buf env;
void throw(int err);
#define THROW(err) throw(err), longjmp(env, err);

#define DEF_OP_L(name, expr...) local Num op_##name(Num a, Num b) { expr; }
#define DEF_OP(name, expr...) DEF_OP_L(name, return (expr);)
#define DEF_LIT(name, expr...) local Num lit_##name(Str* str) { expr; }
#define DEF_FMT(name, expr...) local Num fmt_##name(Num num, int interactive) { expr; return DLnan; }
// wait why don't I make constructors in here,

extern OpDef* infix;
extern OpDef* prefix;
extern OpDef* literal;
extern OpDef* format;

OpDef* addOp(OpDef** list, OpDef* def);
#define ADD_OP(list, args...) addOp(&(list), &(OpDef){ args });
void addAlias(OpDef** list, Str name);
#define ADD_ALIAS(list, name) addAlias(&(list), name);
