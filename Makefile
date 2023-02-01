# Name: Jeffrey Abbinante, BS
# Professor: Andy Wang, PhD
# Class: COP 4610
# Project: 1
# Description: This is the Project 1 makefile (parser demo).

BINS = main
C = gcc
CFLAGS = -std=gnu99 -Wall -pedantic -g

all: $(BINS)

main: main.c
	$(C) $(CFLAGS) -o main main.c

clean:
	rm $(BINS)
