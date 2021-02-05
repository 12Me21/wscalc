#include <stdio.h>
#include <string.h>
#include <stdbool.h>

typedef _Decimal128 Num;
typedef _Bool Bool;

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
#define MIN_DIGIT 1e-6143dl

// Print a _Decimal128
void printDL(Num num) {
	if (num != num) {
		printf("NaN");
		return;
	}
	// test for negatives, including -0
	if (num<0 || (num==0 && 1/num<0)) {
		putchar('-');
		num = -num;
	}
	if (num >= MAX_DIGIT*10) {
		printf("Infinity");
		return;
	}
	
	int zeros = 0;
	Bool decimal = false;
	Bool leading = true;
	Num div;
	for (div=MAX_DIGIT; div>0; div/=10) {
		//This gets the digit in the highest possible place.
		int digit = num /MAX_DIGIT*MIN_DIGIT /MIN_DIGIT;
		// we take the number, and divide it
		// (in 2 steps because the divisor is too large)
		// so that the highest possible digit into the lowest possible digit's place,
		// (and all other digits are lost)
		// then multiply so that this digit is in the 1's place.

		// because it rounds instead of flooring, we have to subtract 1 if it rounded up
		if (num < digit * 1e6144dl)
			digit--;
		// now, we multiply this digit back into the uppermost place
		// subtract it from the number, and multiply the number by 10
		// to shift the next digit into place
		num -= digit * 1e6144dl;
		num *= 10;
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
			putchar(digit+'0');
		} else if (!leading) { // zero digit (except leading zeros)
			// if we're before the decimal point, always print non-leading zeros
			if (div>=1)
				putchar('0');
			// otherwise, keep track of how many zeros there are
			// so they can be printed if they are not trailing zeros
			else
				zeros++;
		}
		// if we're at the 1's place, set a flag that will
		// print a decimal point IF there are any nonzero digits after it
		if (div==1) {
			decimal = true;
			// if no digits have been printed yet, display a 0 before the decimal
			if (leading)
				putchar('0');
			leading = false;
		}
	}
}
