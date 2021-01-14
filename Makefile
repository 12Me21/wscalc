CFLAGS += -lm
name = calc
$(name):

.PHONY: clean
clean:
	$(RM) *.o $(name)
