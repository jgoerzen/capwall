all: capwall
CFLAGS ?= -Wall

capwall: capwall.o
	$(CC) -o capwall capwall.o -lutil
