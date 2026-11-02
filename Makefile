CC     = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c99 -g

EXEC = teste
OBJS = main.o mymemory.o

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

%.o: %.c mymemory.h
	$(CC) $(CFLAGS) -c $<

run: $(EXEC)
	./$(EXEC)

valgrind: $(EXEC)
	valgrind --leak-check=full --show-leak-kinds=all --error-exitcode=1 ./$(EXEC)

clean:
	rm -f $(OBJS) $(EXEC) trabalho2.tar.gz

dist: clean
	tar -czf trabalho2.tar.gz mymemory.h mymemory.c main.c Makefile README.md

.PHONY: all run valgrind clean dist