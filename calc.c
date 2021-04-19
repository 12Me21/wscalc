#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <setjmp.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <errno.h>

#include "calc.h"

#define when(n) break; case n
#define otherwise break; default
#define err(args...) fprintf(stderr, args)

#define CRITICAL ?:(err("Error"),/*perror(NULL), */exit(1), NULL)

#define ALLOC(name) name = malloc(sizeof(*name)) CRITICAL

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

OpDef* addOp(OpDef** listp, OpDef* def) {
	//printf("adding op %s\n", def->name);
	listp CRITICAL;
	def CRITICAL;
	def->name CRITICAL;
	def->func CRITICAL;
	
	OpDef* ALLOC(new);
	*new = *def;

	// keep list sorted by descending name length
	OpDef* prev = NULL;
	size_t len = strlen(def->name);
	OpDef* next = *listp;
	while (next && strlen(next->name) > len) {
		prev = next;
		next = next->next;
	}

	// insert item
	if (prev)
		prev->next = new;
	new->next = next;
	if (next == *listp) // if replacing list head
		*listp = new;
	
	return new;
}

/*
this is broken now because of sorting
void addAlias(OpDef** list, Str name) {
	list CRITICAL;
	name CRITICAL;
	
	addOp(list, *list)->name = name;
}*/

OpDef* prefix = NULL;
OpDef* infix = NULL;
OpDef* literal = NULL;
OpDef* format = NULL;

DEF_LIT(input,
	if (isatty(0))
		err("input number: ");
	Num res;
	if (scanDL(&res, stdin)==1)
		return res;
	THROW(4);
);
DEF_LIT(ans,
	return haveAns ? ans : lit_input(str);
);

DEF_FMT(decimal,
	DLprint(num, 10);
);

AUTORUN {
	ADD_OP(literal, "i", lit_input, "get number from stdin");
	ADD_OP(literal, "a", lit_ans, "value of previous expression");
	ADD_OP(format, "", fmt_decimal, "default number printer");
}



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
		return readExpr(str, (depth ?: 1)+1);
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
		return op(readValue(str, depth ?: 1), 0);
	// Implicit value (top level only)
	// "+1" is treated as "a+1", etc.
	// this only works if the expression is not otherwise valid.
	// so, "-1" is not treated as "a-1", etc.
	if (!depth)
		return lit_ans(str);
	// Error
	THROW(1);
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
		Num v = readValue(str, depth ?: 1);
		return readAfter(str, depth ?: 1, op(acc, v));
	}
	// Error
	THROW(2);
}

local Num readExpr(Str* str, int depth) {
	Num acc = readValue(str, depth);
	return readAfter(str, depth ?: 1, acc);
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
		Op printer = search(&expr, format) CRITICAL;
		Num res = readExpr(&expr, 0); //can longjump
		ans = res;
		haveAns = 1;
		if (interactive)
			printf("> ");
		printer((Num)res, (int)interactive);
		putchar('\n');
		return 0;
	} else { //returned via longjump
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
				err = err ?: errno;
				break;
			}
			if (line[c-1]=='\n')
				line[c-1] = '\0';
			err = err ?: doline(line, 0);
		}
		free(line);
		return err;
	}
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
