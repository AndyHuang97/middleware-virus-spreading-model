CC=mpicc
SRCDIR=src
OUT=obj
SRCS= $(SRCDIR)/main.c $(SRCDIR)/individual.c


compile: $(OBJ)
	$(CC) -o $(OUT)/main.out -g $(SRCS) -lm -I.