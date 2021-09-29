voronoi1: main.o wt_ops.o dcel_ops.o
	gcc -Wall -o voronoi1 main.o wt_ops.o dcel_ops.o -g

main.o: main.c wt_ops.h dcel_ops.h
	gcc -Wall -o main.o main.c -c

wt_ops.o: wt_ops.c wt_ops.h
	gcc -Wall -o wt_ops.o wt_ops.c -c

dcel_ops.o: dcel_ops.c dcel_ops.h
	gcc -Wall -o dcel_ops.o dcel_ops.c -c

clean: voronoi1
	rm *.o voronoi1
