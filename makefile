CC = gcc
CFLAGS = -I.
DEPS = parser.h
OBJ = next.o program.o eval.o main.o

%.o : %.c $(DEPS)
	$(CC) -c -w -m32 -o $@ $< $(CFLAGS)

parser: $(OBJ)
	$(CC) -m32 -w -o $@ $^ $(CFLAGS)

clean:
	rm -f *.o
