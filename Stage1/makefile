all:
	gcc hash.c -c -o hash.o
	gcc lexer.c -c -o lexer.o
	gcc parser.c -c -o parser.o
	gcc driver.c -c -o driver.o
	gcc -o stage1exe hash.o lexer.o driver.o parser.o

