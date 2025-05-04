# Makefile for Assignment 5 - Resource Management
# Author: Curtis Been

CC = gcc
CFLAGS = -Wall -g

all: oss worker

oss: oss.c shared.h
	$(CC) $(CFLAGS) -o oss oss.c

worker: worker.c shared.h
	$(CC) $(CFLAGS) -o worker worker.c

clean:
	rm -f *.o oss worker *.log core
