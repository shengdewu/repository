CC = gcc

CFLAGS = -g -Wall

main : main.o max_heapify.o
	$(CC) main.o max_heapify.o -o main
main.o : main.c max_heapify.h
	$(CC) $(CFLAGS) -c main.c -o main.o
max_heapify.o : max_heapify.c
	$(CC) $(CFLAGS) -c max_heapify.c -o max_heapify.o
		
clean:
	  rm -rf *.o
	