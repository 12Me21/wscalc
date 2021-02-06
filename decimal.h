#include <stdio.h>
#include <math.h>

typedef _Decimal128 Num;
typedef _Bool Bool;
typedef char Char;
typedef Char* Str;

#define DLnan ((Num)NAN)
#define DLinf ((Num)INFINITY)

Bool scanDL(Num* out, FILE* stream);
Num strtoDL(char* s, char** end);
void DLprint(Num x, int base);
Num DLread(Str* str, int base);
Num DLfloor(Num a);
Num DLmod(Num a, Num b);
int DLexponent(Num num);
