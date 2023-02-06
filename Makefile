# Name: Emma Baudo, Thomas Vogt, and Daniel Pijeira
# Professor: Andy Wang, PhD
# Class: COP 4610
# Project: 1
# Description: This is the Project 1 makefile.

BINS = shell
C = gcc
CFLAGS = -std=gnu99 -Wall -pedantic -g

all: $(BINS)

shell: main.c
	$(C) $(CFLAGS) -o shell main.c

clean:
	rm $(BINS)
