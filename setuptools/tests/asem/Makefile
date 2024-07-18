CC = gcc
CFLAGS = -Wall -Wextra -lm -g -fsanitize=address
LDFLAGS = -fsanitize=address
LDLIBS =

SRC = main.c syscalls.c

OBJ = ${SRC:.c=.o}
EXE = ${SRC:.c=}

all: main 

main: ${OBJ}

.PHONY: clean
clean: 
	${RM} ${OBJ} ${EXE}

