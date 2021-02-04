srcs:= calc decimal
output:= calc

libs:= m readline

CFLAGS+= -Wextra -Wall -g -ftabstop=3 -Wno-unused-parameter

include Nice.mk
