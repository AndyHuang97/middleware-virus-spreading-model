CC=mpicc
SRCDIR=src
OUT=obj
SRCS= $(SRCDIR)/main.c $(SRCDIR)/individual.c $(SRCDIR)/utils.c $(SRCDIR)/cell_list.c $(SRCDIR)/country_stats.c


compile: $(OBJ)
	$(CC) -o $(OUT)/main.out -g $(SRCS) -lm -I.