# Makefile for VCParser Shared Library and Main Executable

# Default target builds both the shared library and main executable
all: bin/libvcparser.so main

#shared library for submission
bin/libvcparser.so: src/VCParser.o src/VCHelpers.o src/LinkedListAPI.o
	mkdir -p bin
	gcc -shared -o bin/libvcparser.so src/VCParser.o src/VCHelpers.o src/LinkedListAPI.o

#VCParsers
src/VCParser.o: src/VCParser.c
	gcc -Wall -Werror -fPIC -g -Iinclude -c src/VCParser.c -o src/VCParser.o

#VCHelpers
src/VCHelpers.o: src/VCHelpers.c
	gcc -Wall -Werror -fPIC -g -Iinclude -c src/VCHelpers.c -o src/VCHelpers.o

#LinkedListAPI
src/LinkedListAPI.o: src/LinkedListAPI.c
	gcc -Wall -Werror -fPIC -g -Iinclude -c src/LinkedListAPI.c -o src/LinkedListAPI.o

#shared library
main: main.o bin/libvcparser.so
	gcc -o main main.o -Lbin -lvcparser -Wl,-rpath=bin

#compile main executable for my own testing
main.o: src/main.c
	gcc -Wall -Werror -fPIC -g -Iinclude -c src/main.c -o main.o

#clean
clean:
	rm -f src/*.o bin/libvcparser.so main main.o
