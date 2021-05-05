#include "../calc.h"

local Num readBase(Str* str, int base) {
	Str start = *str;
	Num ret = DLread(str, base);
	if (start == *str)
		THROW(8); // todo: define this error
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

DEF_FMT(oct,
	if (interactive)
		printf("0o");
	DLprint(num, 8);
);

AUTORUN {
	ADD_OP(literal, "0x", lit_hex, "Hexadecimal prefix");
	ADD_OP(literal, "x", lit_hex, "Hexadecimal prefix");
	ADD_OP(literal, "&h", lit_hex, "Hexadecimal prefix");
	ADD_OP(literal, "&H", lit_hex, "Hexadecimal prefix");
	ADD_OP(literal, "&", lit_oct, "Hexadecimal prefix");
	
	ADD_OP(literal, "0o", lit_oct, "Octal prefix");
	ADD_OP(literal, "o", lit_oct, "Octal prefix");
	ADD_OP(literal, "&o", lit_oct, "Octal prefix");
	ADD_OP(literal, "&O", lit_oct, "Octal prefix");

	ADD_OP(literal, "0b", lit_bin, "Binary prefix");
	ADD_OP(literal, "b", lit_bin, "Binary prefix");
	ADD_OP(literal, "&b", lit_bin, "Binary prefix");
	ADD_OP(literal, "&B", lit_bin, "Binary prefix");

	ADD_OP(format, "?x", fmt_hex, "Print in hexadecimal");
	ADD_OP(format, "?o", fmt_oct, "Print in octal");
	ADD_OP(format, "?b", fmt_bin, "Print in binary");
}
