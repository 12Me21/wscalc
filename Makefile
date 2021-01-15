CFLAGS += -lm -lreadline
name = calc

first: $(name)

clean:
	$(RM) *.o $(name)

.PHONY: clean first
