all: keys.o exec.o ast.o input.o
	gcc -o keys keys.o exec.o ast.o input.o

keys.o: keys.c input.h ast.h
	gcc -c keys.c

input.o: input.c input.h
	gcc -c input.c
	
# parser.o: parser.c parser.h exec.h
# 	gcc -c parser.c

exec.o: exec.c exec.h
	gcc -c exec.c

ast.o: ast.c ast.h exec.h
	gcc -c ast.c

run:
	./keys

clean:
	-rm *.o
	-rm keys
