// shared.h - Shared Structures
// Author: Curtis Been

#ifndef SHARED_H
#define SHARED_H

#define MAX_PROCESSES 18
#define MAX_RESOURCES 5
#define MAX_INSTANCES 10
#define BILLION 1000000000

typedef struct {
    int seconds;
    int nanoseconds;
} SimClock;

typedef struct {
    int allocated[MAX_PROCESSES];
    int request[MAX_PROCESSES];
    int available;
    int max;
} ResourceDescriptor;

typedef struct {
    long mtype;
    int pid;
    int resource_id;
    char action[8];
} Message;

#endif
