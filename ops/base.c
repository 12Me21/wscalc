#include "../calc.h"

local Num readBase(Str* str, int base) {
	Str start = *str;
	Num ret = DLread(str, 16);
	if (start == *str)
		THROW(8);
	return ret;
}

DEF_LIT(hex, return readBase(str, 16));
DEF_LIT(oct, return readBase(str, 8));
DEF_LIT(bin, return readBase(str, 2));

DEF_FMT(hex,
	//todo: print prefix in different color
	if (interactive)
		printf("0x");
	DLprint(num, 16);
);

DEF_FMT(bin,
	if (interactive)
		printf("0b");
	DLprint(num, 2);
);

AUTORUN {
	ADD_OP(literal, "0x", lit_hex, "Hexadecimal prefix");
	ADD_ALIAS(literal, "x");
	ADD_ALIAS(literal, "&h");
	ADD_ALIAS(literal, "&H");
	
	ADD_OP(literal, "0o", lit_oct, "Octal prefix");
	ADD_ALIAS(literal, "o");
	ADD_ALIAS(literal, "&o");
	ADD_ALIAS(literal, "&O");

	ADD_OP(literal, "0b", lit_bin, "Binary prefix");
	ADD_ALIAS(literal, "b");
	ADD_ALIAS(literal, "&b");
	ADD_ALIAS(literal, "&B");

	ADD_OP(format, "?x", fmt_hex, "Print in hexadecimal");
	ADD_OP(format, "?b", fmt_bin, "Print in binary");
}
