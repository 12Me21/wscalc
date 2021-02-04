#include <stdio.h>
#include <string.h>

typedef _Decimal128 Num;

int scanDL(Num* out, FILE* stream) {
	Num ret = 0;
	Num decimal = 0;
	int got=0;
	while(1) {
		int c = fgetc(stream);
		if (c>='0' && c<='9') {
			got=1;
			c-='0';
			if (decimal) {
				ret += c*decimal;
				decimal/=10;
			} else {
				ret *= 10;
				ret += c;
			}
		} else if (c=='.' && !decimal) {
			got=1;
			decimal = 0.1DL;
		} else if (c==EOF) {
			break;
		} else {
			ungetc(c, stream);
			break;
		}
	}
	*out = ret;
	return got;
}

Num strtoDL(char* s, char** end) {
	FILE* f=fmemopen(s, strlen(s), "r");
	Num n;
	int r = scanDL(&n, f);
	if (r)
		*end = s+ftell(f);
	else
		*end = NULL;
	return n;
}

void printDL(Num num) {
	Num div = _Generic(div,
		_Decimal128: 1e6144dl
	);
	if (num<0) {
		putchar('-');
		num = -num;
	}
	int zeros = 0;
	int decimal = 0;
	int firstd = 0;
	///dprintf(2, "<1 %d\n", num<1);
	for (; div; div/=10) {
		Num x = num/1e6144dl/1e6143dl*1e6143dl*1e6144dl;
		int digit = (x/1e6144dl);
		if (num < digit * 1e6144dl) //in case it rounds up
			digit--;
		num -= digit * 1e6144dl;
		num *= 10;
		if (digit) {
			for (; decimal; decimal=0)
				putchar('.');
			for (; zeros; zeros--)
				putchar('0');
			firstd = 1;
			putchar(digit+'0');
		} else if (firstd) {
			if (div>=1)
				putchar('0');
			else
				zeros++;
		}
		if (div==1) {
			decimal = 1;
			for (; !firstd; firstd=1)
				putchar('0');
		}
	}
}
