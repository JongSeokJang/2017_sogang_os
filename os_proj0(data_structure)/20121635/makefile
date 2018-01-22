CC = gcc
TARGET = testlib

$(TARGET): main.o bitmap.o hash.o list.o
	$(CC) -o $(TARGET) main.o bitmap.o hash.o list.o -lm

main.o: main.c bitmap.h hash.h list.h
	$(CC) -c main.c

bitmap.o: bitmap.c stdio.c bitmap.h limits.h round.h
	$(CC) -c -w bitmap.c

hash.o: hash.c hash.h
	$(CC) -c hash.c -lm

list.o: list.c list.h
	$(CC) -c list.c

clean:
	rm *.o $(TARGET)
