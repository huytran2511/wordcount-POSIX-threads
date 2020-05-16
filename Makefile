CC=gcc
CCFLAGS=-std=c11 -g

wordcount : wordcount.o
	$(CC) $(CCFLAGS) -o wordcount $^ -lpthread

wordcount.o : wordcount.c

clean :
	rm *.o