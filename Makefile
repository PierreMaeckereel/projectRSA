CC=gcc
CFLAGS=-Wall
# LDFLAGS= utils math
EXEC=main
SRC= $(wildcard *.c)
OBJ= $(SRC:.cpp=.o)

all: clean $(EXEC)


$(EXEC): $(OBJ)
	$(CC) -o $@ $^

%.o: %.c %.h
	$(CC) -o $@ -c $< $(CFLAGS)



clean:
	rm -rf *.o

mrproper: clean
	rm -rf $(EXEC)
