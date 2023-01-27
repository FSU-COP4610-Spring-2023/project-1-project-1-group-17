# Name: Jeffrey Abbinante, BS
# Professor: Andy Wang, PhD
# Class: COP 4610
# Project: 1
# Description: This is the Project 1 makefile (parser demo).

BINS = parser
C = gcc
CFLAGS = -std=gnu99 -Wall -pedantic -g

all: $(BINS)

parser: parser.c
	$(C) $(CFLAGS) -o parser parser.c

clean:
	rm $(BINS)
