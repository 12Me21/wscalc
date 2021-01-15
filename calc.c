#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <setjmp.h>
#include <readline/readline.h>
#include <readline/history.h>

#define when(n) break; case n

typedef double Num;
typedef char* Str;
typedef Num (*Op)(/*any args lol*/); //typedef Op
typedef struct {Str name; Op func;} OpDef;

#define OPDEF(name, expr) Num op_##name(Num a, Num b) { return (expr); }
OPDEF(neg,-a);
OpDef prefix[] = {
	{"-",op_neg},
	{NULL, NULL},
};
OPDEF(add,a+b);OPDEF(sub,a-b);OPDEF(mul,a*b);OPDEF(div,a/b);OPDEF(mod,fmod(a,b));OPDEF(pow,pow(a,b));
OpDef infix[] = {
	{"+",op_add},{"-",op_sub},{"*",op_mul},{"/",op_div},{"%",op_mod},{"^",op_pow},
	{NULL, NULL},
};

Num ans = 0;

Op search(Str* str, OpDef* ops) {
	for (; ops->name ; ops++) {
		int len = strlen(ops->name);
		if (strncmp(*str, ops->name, len)==0) {
			*str += len;
			return ops->func;
		}
	}
	return NULL;
}

Num readExpr(Str*, int, jmp_buf);

Num readValue(Str* str, int depth, jmp_buf env) {
	if ((*str)[0]>='0' && (*str)[0]<='9' || (*str)[0]=='.') {
		Str end;
		Num num = strtod(*str, &end);
		if (end) {
			*str = end;
			return num;
		}
	}
	if ((*str)[0]=='a') {
		(*str)++;
		return ans;
	}
	if ((*str)[0]==' ') {
		(*str)++;
		return readExpr(str, depth+1, env);
	}
	Op op = search(str, prefix);
	if (op)
		return op(readValue(str, depth, env), 0);
	longjmp(env, 1);
}

Num readAfter(Str* str, int depth, Num acc, jmp_buf env) {
	if ((*str)[0]==' ') {
		(*str)++;
		if (depth>0)
			return acc;
	}
	if ((*str)[0]=='\0')
		return acc;
	Op op = search(str, infix);
	if (op) {
		Num v = readValue(str, depth, env);
		return readAfter(str, depth, op(acc, v), env);
	}
	longjmp(env, 2);
}

Num readExpr(Str* str, int depth, jmp_buf env) {
	Num acc = readValue(str, depth, env);
	return readAfter(str, depth, acc, env);
}

int main(int argc, Str* argv) {
	while (1) {
		Str line = readline("<< ");
		add_history(line);
		if (!line)
			break;
		//if (line[0]=='\0')
		//	continue;
		jmp_buf env;
		switch (setjmp(env)) {
		when(0):;
			Str expr = line;
			Num res = readExpr(&expr, 0, env);
			ans = res;
			printf("> %.15g\n", res);
		when(1):;
			printf("! Error: expected value (number, a) or prefix operator (");
			OpDef* op;
			for (op=prefix; op->name; op++) {
				if (op!=prefix)
					printf(", ");
				printf("%s", op->name);
			}
			printf(")\n");
			goto err;
		when(2):;
			printf("! Error: expected operator (");
			for (op=infix; op->name; op++) {
				if (op!=infix)
					printf(", ");
				printf("%s", op->name);
			}
			printf(")\n");
			goto err;
		err:;
			printf("! %.*s⟨here⟩%s\n", (int)(expr-line), line, expr);
		}
		free(line);
	}
}
