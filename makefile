all: keys.o
	gcc -o output keys.o

keys.o: keys.c
	gcc -c keys.c

run:
	./output

clean:
	rm *.o
	rm output
