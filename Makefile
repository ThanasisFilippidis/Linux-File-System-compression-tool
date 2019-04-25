MYDIZ = mydiz.o x.o c.o
OUTMYDIZ = mydiz

CC = gcc
FLAGS  = -c -g

all: $(OUTMYDIZ)

$(OUTMYDIZ): $(MYDIZ)
	$(CC) -g $(MYDIZ) -o $(OUTMYDIZ)

mydiz.o: mydiz.c
	$(CC) $(FLAGS) mydiz.c

x.o: x.c
	$(CC) $(FLAGS) x.c

c.o: c.c
	$(CC) $(FLAGS) c.c

clean:
	rm -f $(OUTMYDIZ)
