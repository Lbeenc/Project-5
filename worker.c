// worker.c - Simulated User Process
// Author: Curtis Been
// Description: Sends resource requests/releases/terminate messages to OSS

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <time.h>

#define MAX_RESOURCES 5
#define BILLION 1000000000

typedef struct {
    long mtype;
    int pid;
    int resource_id;
    char action[8];
} Message;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <msgid>\n", argv[0]);
        exit(1);
    }

    int msgid = atoi(argv[1]);
    pid_t pid = getpid();
    srand(pid);

    int owned[MAX_RESOURCES] = {0};
    int runtime_ns = 0;

    while (1) {
        usleep(250000); // simulate 250ms
        runtime_ns += 250000000;

        // 20% chance to terminate after 1 second
        if (runtime_ns > BILLION && rand() % 5 == 0) {
            Message term_msg = {1, pid, -1, "terminate"};
            msgsnd(msgid, &term_msg, sizeof(Message) - sizeof(long), 0);
            break;
        }

        Message msg;
        msg.mtype = 1;
        msg.pid = pid;
        msg.resource_id = rand() % MAX_RESOURCES;

        // 80% request, 20% release if owns any
        if ((rand() % 10 < 8) || owned[msg.resource_id] == 0) {
            strcpy(msg.action, "request");
        } else {
            strcpy(msg.action, "release");
        }

        // prevent over-requesting
        if (strcmp(msg.action, "request") == 0 && owned[msg.resource_id] < 10) {
            msgsnd(msgid, &msg, sizeof(Message) - sizeof(long), 0);

            // wait to be granted
            Message response;
            msgrcv(msgid, &response, sizeof(Message) - sizeof(long), pid + 1, 0);
            owned[msg.resource_id]++;
        } else if (strcmp(msg.action, "release") == 0) {
            msgsnd(msgid, &msg, sizeof(Message) - sizeof(long), 0);
            owned[msg.resource_id]--;
        }
    }

    return 0;
}
