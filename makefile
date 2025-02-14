# Makefile
# Muhammad Ali
# Student ID: 1115336

CC = gcc
CFLAGS = -Wall -std=c11 -g
LDFLAGS = -L.
INC = include/
SRC = src/
BIN = bin/

all: parser

parser: $(BIN)libvcparser.so

$(BIN)libvcparser.so: $(BIN)VCParser.o $(BIN)VCHelpers.o $(BIN)LinkedListAPI.o
	mkdir -p $(BIN)
	$(CC) $(CFLAGS) $(LDFLAGS) -shared -o $(BIN)libvcparser.so $(BIN)VCParser.o $(BIN)VCHelpers.o $(BIN)LinkedListAPI.o

$(BIN)VCParser.o: $(SRC)VCParser.c $(INC)VCParser.h
	mkdir -p $(BIN)
	$(CC) $(CFLAGS) -I$(INC) -fPIC -c $(SRC)VCParser.c -o $(BIN)VCParser.o

$(BIN)VCHelpers.o: $(SRC)VCHelpers.c $(INC)VCHelpers.h
	mkdir -p $(BIN)
	$(CC) $(CFLAGS) -I$(INC) -fPIC -c $(SRC)VCHelpers.c -o $(BIN)VCHelpers.o

$(BIN)LinkedListAPI.o: $(SRC)LinkedListAPI.c $(INC)LinkedListAPI.h
	mkdir -p $(BIN)
	$(CC) $(CFLAGS) -I$(INC) -fPIC -c $(SRC)LinkedListAPI.c -o $(BIN)LinkedListAPI.o

clean:
	rm -rf $(BIN)*.o $(BIN)*.so *.o *.so
