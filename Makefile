CC=gcc
CFLAGS=-Wall
# LDFLAGS= utils math
EXEC=myAdBlock
SRC= $(wildcard *.c)
OBJ= $(SRC:.cpp=.o)

all: clean $(EXEC)


$(EXEC): $(OBJ)
	$(CC) -o $@ $^

%.o: %.c %.h
	$(CC) -o $@ -c $< $(CFLAGS)

run: 
	./myAdBlock 8080

clean:
	rm -rf *.o

rmproper: clean
	rm -rf $(EXEC)
