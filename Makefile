# Makefile do Trabalho 2
#
#   make           -> compila
#   make run       -> compila e roda os testes
#   make valgrind  -> roda os testes procurando vazamentos de memória
#   make clean     -> apaga os arquivos gerados
#   make dist      -> gera o trabalho2.tar.gz para entregar no Moodle

CC     = gcc
# -Wall -Wextra -pedantic: liga quase todos os avisos (o trabalho deve
#                          compilar SEM warnings)
# -std=c99: usa o padrão C99
# -g: inclui informações de depuração (o valgrind mostra as linhas do erro)
CFLAGS = -Wall -Wextra -pedantic -std=c99 -g

EXEC = teste
OBJS = main.o mymemory.o

all: $(EXEC)

# Junta os .o num executável
$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

# Compila cada .c em .o (recompila se o .c ou o mymemory.h mudar)
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
