all: compile run

compile:
	gcc src/main.c -o build/test 

run:
	./build/test
