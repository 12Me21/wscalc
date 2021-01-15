CFLAGS += -lm -lreadline
name = calc
$(name):

.PHONY: clean
clean:
	$(RM) *.o $(name)
