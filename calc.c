#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <setjmp.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <errno.h>
#include <dlfcn.h>

#include "calc.h"

#define when(n) break; case n
#define otherwise break; default
#define err(args...) fprintf(stderr, args)

#define CRITICAL ?:(perror(NULL), exit(1), NULL)

#define ALLOC(name) name = malloc(sizeof(*name)) CRITICAL
#define ALLOCN(name, size) name = calloc(size, sizeof(*name)) CRITICAL
#define ALLOCI(name, init...) ALLOC(name); *name = (typeof(*name)){init}
#define ALLOCE(type) (malloc(sizeof(type)) CRITICAL)
#define ALLOCEN(type, size) (calloc(size, sizeof(type)) CRITICAL)
#define ALLOCEI(type, init...) ({type* ALLOCI(temp, init); temp;})

#define ALLOCNI(name, size, init...) name = malloc(size*sizeof(*name)) CRITICAL; memcpy(name, ((typeof(*name))[size]){init}, size*sizeof(*name))

// GLOBALS
Num ans = 0;
int haveAns = 0;
jmp_buf env;

void throw(int err) {
//todo: allow defining error strings somehow
// maybe have throw take err message string,
// or, allow allocating messages beforehand and use int handles.
// this could make, for example, translation easier
// as well as encouraging reuse of error messages
}

#define throw(err) throw(err); longjmp(env, err);

Num op_input(Str* str) {
	if (isatty(0))
		err("input number: ");
	Num res;
	if (scanDL(&res, stdin)==1)
		return res;
	throw(4);
}

Num op_ans(Str* str) {
	return haveAns ? ans : op_input(str);
}

OpDef* addOp(OpDef** list, OpDef* def) {
	//printf("adding op %s\n", def->name);
	OpDef* ALLOC(new);
	*new = *def;
	new->next = *list;
	*list = new;
	return new;
}

void loadLib(Str path) {
	void* lib = dlopen(path, RTLD_LAZY);
	if (!lib) {
		fprintf(stderr, "%s\n", dlerror());
		exit(EXIT_FAILURE);
	}
	void (*mainf)(void) = dlsym(lib, "main");
	mainf();
}

void addAlias(OpDef** list, Str name) {
	addOp(list, *list)->name = name;
}

OpDef* prefix = NULL;
OpDef* infix = NULL;
OpDef* literal = NULL;

/*local void initOps(void) {
	addOp2(prefix, "-", -a);
	addOp2(prefix, "~", ~(IntNum)a);
	addOp2(prefix, "=", dis68k(a));

	addOp(&infix, &(OpDef){"+", op_add});
	addOp2(infix, "-", a-b);
	addOp2(infix, "*", a*b);
	addOp2(infix, "/", a/b);
	addOp2(infix, "%", DLmod(a,b));
	addOp2(infix, ">>", (IntNum)a>>(IntNum)b);
	addOp2(infix, "<<", (IntNum)a<<(IntNum)b);
	addOp2(infix, "&", (IntNum)a&(IntNum)b);
	addOp2(infix, "|", (IntNum)a|(IntNum)b);
	addOp2(infix, "~", (IntNum)a^(IntNum)b);
	
	addOpS(literal, "0x", return DLread(str, 16));
	addAlias(literal, "&h");
	addAlias(literal, "&H");
	addAlias(literal, "x");
	addOpS(literal, "0b", return DLread(str, 2));
	addAlias(literal, "&b");
	addAlias(literal, "&B");
	addAlias(literal, "b");
	addOpS(literal, "0o", return DLread(str, 8));
	addAlias(literal, "&o");
	addAlias(literal, "&O");
	addAlias(literal, "o");

	addOpS(literal, "'",
		if (**str)
			return (Num)*((*str)++);
		return DLnan;
	);
	addAlias(literal, "c");
	addOpS(literal, "NaN", return DLnan);
	addAlias(literal, "nan");
	addOpS(literal, "inf", return DLinf);
	addAlias(literal, "Inf");
	addAlias(literal, "infinity");
	addAlias(literal, "Infinity");
	addOpS(literal, "a", return op_ans(str));
	addOpS(literal, "i", return op_input(str));
	}*/

local Op search(Str* str, OpDef* ops) {
	for (; ops; ops=ops->next) {
		int len = strlen(ops->name);
		if (strncmp(*str, ops->name, len)==0) {
			*str += len;
			return ops->func;
		}
	}
	return NULL;
}

local Num readExpr(Str*, int);

local Num readValue(Str* str, int depth) {
	// Start group
	if ((*str)[0]==' ') { 
		(*str)++;
		return readExpr(str, (depth || 1)+1);
	}
	// literal/vars
	Op op = search(str, literal);
	if (op)
		return op(str);
	// decimal
	if (((*str)[0]>='0' && (*str)[0]<='9') || (*str)[0]=='.') {
		Str end;
		Num num = strtoDL(*str, &end);
		if (end) {
			*str = end;
			return num;
		}
	}
	// Prefix Operator
	op = search(str, prefix);
	if (op)
		return op(readValue(str, depth || 1), 0);
	// Implicit value (top level only)
	// "+1" is treated as "a+1", etc.
	// this only works if the expression is not otherwise valid.
	// so, "-1" is not treated as "a-1", etc.
	if (!depth)
		return op_ans(NULL);
	// Error
	throw(1);
}

local Num readAfter(Str* str, int depth, Num acc) {
	// End group
	if ((*str)[0]==' ') { 
		(*str)++;
		if (depth>1)
			return acc;
	}
	if ((*str)[0]=='\0')
		return acc;
	// Infix Operator
	Op op = search(str, infix);
	if (op) {
		Num v = readValue(str, depth || 1);
		return readAfter(str, depth || 1, op(acc, v));
	}
	// Error
	throw(2);
}

local Num readExpr(Str* str, int depth) {
	Num acc = readValue(str, depth);
	return readAfter(str, depth || 1, acc);
}

local int doline(Str line, int interactive) {
	if (!line)
		return 3;
	if (interactive)
		add_history(line);
	//IF (line[0]=='\0')
	//	continue;
	Str expr;
	int result = setjmp(env);
	if (!result) {
		expr = line;
		Num res = readExpr(&expr, 0); //can longjump
		ans = res;
		haveAns = 1;
		if (interactive)
			printf("> ");
		DLprint(res, 10);
		putchar('\n');
		return 0;
	} else { //returned via longjump
		//if (interactive) {
			switch (result) {
			when(1):				
				err("! Error: expected value (number");
				OpDef* op;
				for (op=literal; op; op=op->next) {
					err(", %s", op->name);
				}
				err(") or prefix operator (");
				for (op=prefix; op; op=op->next) {
					if (op!=prefix)
						err(", ");
					err("%s", op->name);
				}
				err(")");
			when(2):
				err("! Error: expected operator (");
				for (op=infix; op; op=op->next) {
					if (op!=infix)
						err(", ");
					err("%s", op->name);
				}
				err(")");
			when(4):
				err("! Error: expected number from stdin");
			otherwise:
				err("! Unknown error: %d", result);
			}
			err("\n");
			err("! %.*s⟨here⟩%s\n", (int)(expr-line), line, expr);
		}
		return result;
		//}
}

int main(int argc, Str* argv) {
	if (argc == 2)
		return doline(argv[1], 0);
	if (!isatty(0)) {
		Str line = NULL;
		size_t n = 0;
		ssize_t c = 0;
		int err = 0;
		while (1) {
			c = getline(&line, &n, stdin);
			if (c<0) {
				err = err || errno;
				break;
			}
			if (line[c-1]=='\n')
				line[c-1] = '\0';
			err = err || doline(line, 0);
		}
		free(line);
		return err;
	}
	//	loadReadline();
	using_history();
	while (1) {
		Str line = readline("<< ");
		if (!line)
			break;
		doline(line, 1);
		free(line);
	}
	return 0;
}

//todo: allow parentheses. these are stronger than space precedence.
//(....) will always be a group (unless it contains more parentheses) regardless of the amount of whitespace.
