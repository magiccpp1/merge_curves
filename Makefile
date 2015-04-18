all: merge_curves
	
merge_curves: merge.o
	gcc -Wall -g merge.o -o merge_curves


merge.o: merge.c
	gcc -Wall -c -g merge.c

clean:
	rm *o hello