#include "../calc.h"
#include "m68kd.h"

DEF_OP_L(dis68k,
	U64 data = a;
	U64 highestN(U64 data, int bits) {
		return data>>(64-bits);
	}
	while (data && !highestN(data, 16))
		data<<=16;
	//printf("data: [%016lX]\n", data);
	U16 nextWord() {
		U16 ret = highestN(data, 16);
		data<<=16;
		//printf("word: %04X\n", ret);
		return ret;
	}
	U32 nextLong() {
		U32 ret = highestN(data, 32);
		data<<=32;
		//printf("long: %08X\n", ret);
		return ret;
	}
	do {
		Str x = M68KDisasm(nextWord, nextLong);
		printf("%s\n", x);
	} while (data);
	return a;
);

AUTORUN {
	ADD_OP(prefix, "#", op_dis68k, "Disassemble 68000 code");
}
