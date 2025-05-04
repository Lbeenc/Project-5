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
    char action[10];
} Message;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <msgid>\n", argv[0]);
        exit(1);
    }

    int msgid = atoi(argv[1]);
    pid_t pid = getpid();
    srand(time(NULL) ^ pid);

    int owned[MAX_RESOURCES] = {0};
    int runtime_ns = 0;

    while (1) {
        usleep(250000); // simulate 250ms
        runtime_ns += 250000000;

        // Terminate after 2 simulated seconds
        if (runtime_ns > 2 * BILLION) {
            printf("WORKER %d terminating after 2s\n", pid);
            fflush(stdout);
            Message term_msg;
            term_msg.mtype = 1;
            term_msg.pid = pid;
            term_msg.resource_id = -1;
            strcpy(term_msg.action, "terminate");
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
            printf("WORKER %d requesting R%d\n", pid, msg.resource_id);
            fflush(stdout);
            msgsnd(msgid, &msg, sizeof(Message) - sizeof(long), 0);

            // wait to be granted
            Message response;
            if (msgrcv(msgid, &response, sizeof(Message) - sizeof(long), pid + 1, 0) > 0) {
                owned[msg.resource_id]++;
                printf("WORKER %d granted R%d\n", pid, msg.resource_id);
                fflush(stdout);
            }
        } else if (strcmp(msg.action, "release") == 0) {
            printf("WORKER %d releasing R%d\n", pid, msg.resource_id);
            fflush(stdout);
            msgsnd(msgid, &msg, sizeof(Message) - sizeof(long), 0);
            owned[msg.resource_id]--;
        }
    }

    return 0;
}
