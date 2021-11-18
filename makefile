all: keys.o parser.o exec.o
	gcc -o keys keys.o parser.o exec.o

keys.o: keys.c parser.h exec.h
	gcc -c keys.c

parser.o: parser.c parser.h exec.h
	gcc -c parser.c

exec.o: exec.c exec.h
	gcc -c exec.c

run:
	./keys

clean:
	-rm *.o
	-rm keys
