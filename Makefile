all: capwall
capwall: capwall.c
	gcc -Wall -o capwall capwall.c -lutil
