# all .c files in ops/  must be tied up
srcs:= calc decimal $(patsubst %.c,%,$(wildcard ops/*.c))
output:= calc

libs:= m readline dl

W_warn = all extra
W_ignore = unused-parameter missing-field-initializers
W_error = implicit-function-declaration

CFLAGS+= -g -ftabstop=3
CFLAGS+= $(addprefix -W,$(W_warn))
CFLAGS+= $(addprefix -Wno-,$(W_ignore))
CFLAGS+= $(addprefix -Werror=,$(W_error))

include .Nice.mk

#%.so: %.c
#	$(cc) $(CFLAGS) -c -fPIC $< -o $(junkdir)/$(@:.so=.o)f
#	ld -shared $(junkdir)/$(@:.so=.o) -o $@ calc
#❤️💚💙
