#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <stdint.h>

#include "decimal.h"

/*int DLexponent(Num num) {
	__int128 bits = (union {Num num; __int128 i;}){.num=num}.i;
	uint32_t top = bits>>(128-32);
	int i;
	for(i=127; i>=0; i--) {
		putchar('0'+(bits>>i&1));
	}
	puts("");
	Bool sign = top>>31;
	int exp = top>>(32-17+1) & (1<<12)-1;
	int comb = top>>(32-5-1) & 0b11111;
	int sig;
	printf("%d\n", comb);
	switch (comb) {
	case 0b00000 ... 0b10111:
		exp |= comb>>3<<12;
		sig = comb & 0b111;
		break;
	case 0b11000 ... 0b11101:
		exp |= (comb>>1 & 0b11)<<12;
		sig = comb & 0b1;
		break;
	case 0b11110:; //infinity
		
	break;
	case 0b11111:; //nan
		
	}
	return exp - 6176;
	}*/

Num DLread(Str* str, int base) {
	Str start = *str;
	Num num = 0;
	Num place = -1;
	while (1) {
		Char c = **str;
		if (!c)
			break;
		if (c=='.' && place<0) {
			place = 1.dl/base;
		} else {
			if (c>='0' && c<='9')
				c -= '0';
			else if (c>='A' && c<='Z')
				c -= 'A'-10;
			else if (c>='a' && c<='z')
				c -= 'a'-10;
			else
				break;
			if (c >= base)
				break;
			if (place > 0) {
				num += c*place;
				place /= base;
			} else
				num = num*base + c;
		}
		(*str)++;
	}
	Num definitely_no_zero_here = 0;
	if (*str == start)
		return 1.0dl/definitely_no_zero_here*0;
	return num;
}

Bool scanDL(Num* out, FILE* stream) {
	Num ret = 0;
	Num decimal = 0;
	Bool got = false;
	while(1) {
		int c = fgetc(stream);
		if (c>='0' && c<='9') {
			got = true;
			c-='0';
			if (decimal) {
				ret += c*decimal;
				decimal/=10;
			} else {
				ret *= 10;
				ret += c;
			}
		} else if (c=='.' && !decimal) {
			got = true;
			decimal = 0.1dl;
		} else if (c==EOF) {
			break;
		} else if (c=='e') {
			c = fgetc(stream);
				
			
		} else {
			ungetc(c, stream);
			break;
		}
	}
	*out = ret;
	return got;
}

// this is a hack because i wrote the stream version first...
Num strtoDL(char* s, char** end) {
	FILE* f=fmemopen(s, strlen(s), "r");
	Num n;
	Bool r = scanDL(&n, f);
	if (r)
		*end = s+ftell(f);
	else
		*end = NULL;
	return n;
}

#define MAX_DIGIT 1e6144dl
#define MIN_DIGIT 1e-6176dl

// Print a _Decimal128
void DLprint(Num num, int base) {
	// NaN
	if (num != num) {
		printf("NaN");
		return;
	}
	// test for negatives, including -0
	if (num<0 || (num==0 && 1/num<0)) {
		putchar('-');
		num = -num;
	}
	// Infinity
	if (num >= DLinf) {
		printf("Infinity");
		return;
	}
	
	int zeros = 0;
	Bool decimal = false;
	Bool leading = true;
	Num place = 1;
	if (base == 10) {
		place = MAX_DIGIT;
	} else {
		Num prev = place;
		while (place < 1e32dl) {
			prev = place;
			place *= base;
		}
		place = prev;
		if (num >= place*10) {
			printf("<too large, printing in decimal> ");
			base = 10;
			place = MAX_DIGIT;
		}
	}
	for (; place>0; place/=base) {
		/*if (place > num)
		  continue;
		  if (!num)
		  break;*/
		int digit;
		for (digit=0; digit<base; digit++) {
			if (num < place)
				break;
			num -= place;
		}
		// nonzero digits
		if (digit) {
			// if we're past the decimal point:
			if (decimal)
				putchar('.');
			decimal = false;
			// if there were zeros right before this digit, print them:
			for (; zeros; zeros--)
				putchar('0');
			leading = false;
			if (digit < 10)
				putchar(digit+'0');
			else
				putchar(digit-10+'A');
		} else if (!leading) { // zero digit (except leading zeros)
			// if we're before the decimal point, always print non-leading zeros
			if (place>=1)
				putchar('0');
			// otherwise, keep track of how many zeros there are
			// so they can be printed if a nonzero digit shows up later
			else
				zeros++;
		}
		// if we're at the 1's place, set a flag that will
		// print a decimal point IF there are any nonzero digits after it
		if (place==1) {
			decimal = true;
			// if no digits have been printed yet, display a 0 before the decimal
			if (leading)
				putchar('0');
			leading = false;
		}
	}
}

Num DLfloor(Num a) {
	Num b = a*MIN_DIGIT/MIN_DIGIT;
	return b<=a ? b : b-1;
}

Num DLmod(Num a, Num b) {
	return a-DLfloor(a/b)*b;
}
