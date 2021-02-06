# all .c files in ops/  must be tied up
srcs:= calc decimal $(patsubst %.c,%,$(wildcard ops/*.c))
output:= calc

libs:= m readline dl

CFLAGS+= -Wextra -Wall -g -ftabstop=3 -Wno-unused-parameter -Wno-missing-field-initializers

include .Nice.mk

#%.so: %.c
#	$(cc) $(CFLAGS) -c -fPIC $< -o $(junkdir)/$(@:.so=.o)f
#	ld -shared $(junkdir)/$(@:.so=.o) -o $@ calc
