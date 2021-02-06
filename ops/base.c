#include "../calc.h"

DEF_LIT(hex, return DLread(str, 16));
DEF_LIT(oct, return DLread(str, 8));
DEF_LIT(bin, return DLread(str, 2));

AUTORUN {
	ADD_OP(literal, "0x", lit_hex);
	ADD_OP(literal, "x", lit_hex);
	ADD_OP(literal, "&h", lit_hex);
	ADD_OP(literal, "&H", lit_hex);
	
	ADD_OP(literal, "0o", lit_oct);
	ADD_OP(literal, "&o", lit_oct);
	ADD_OP(literal, "&O", lit_oct);

	ADD_OP(literal, "0b", lit_bin);
	ADD_OP(literal, "b", lit_bin);
	ADD_OP(literal, "&b", lit_bin);
	ADD_OP(literal, "&B", lit_bin);
}
