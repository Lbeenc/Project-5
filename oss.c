// CMP SCI 4760 - Assignment 5
// Author: Curtis Been
// Description: Resource Management Simulation with Deadlock Detection

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <time.h>

#define MAX_PROCESSES 18
#define MAX_TOTAL_PROCESSES 40
#define MAX_RESOURCES 5
#define MAX_INSTANCES 10
#define BILLION 1000000000

typedef struct {
    int seconds;
    int nanoseconds;
} SimClock;

typedef struct {
    long mtype;
    int pid;
    int resource_id;
    char action[10];
} Message;

typedef struct {
    int allocated[MAX_PROCESSES];
    int request[MAX_PROCESSES];
    int available;
    int max;
} ResourceDescriptor;

SimClock* clock_shm = (void *) -1;
int shm_clock_id;
ResourceDescriptor* resources = (void *) -1;
int shm_res_id;
int msgid;
int total_processes_launched = 0;
FILE* log_file;

void advance_clock(int sec, int nano) {
    clock_shm->nanoseconds += nano;
    clock_shm->seconds += sec + clock_shm->nanoseconds / BILLION;
    clock_shm->nanoseconds %= BILLION;
}

void cleanup(int sig) {
    msgctl(msgid, IPC_RMID, NULL);

    if (clock_shm != (void *) -1) shmdt(clock_shm);
    if (resources != (void *) -1) shmdt(resources);

    shmctl(shm_clock_id, IPC_RMID, NULL);
    shmctl(shm_res_id, IPC_RMID, NULL);

    if (log_file) fclose(log_file);

    kill(0, SIGTERM);
    printf("\nCleaned up resources. Exiting...\n");
    exit(0);
}

void setup_shared_memory() {
    shm_clock_id = shmget(IPC_PRIVATE, sizeof(SimClock), IPC_CREAT | 0666);
    clock_shm = (SimClock*) shmat(shm_clock_id, NULL, 0);
    clock_shm->seconds = 0;
    clock_shm->nanoseconds = 0;

    shm_res_id = shmget(IPC_PRIVATE, sizeof(ResourceDescriptor) * MAX_RESOURCES, IPC_CREAT | 0666);
    resources = (ResourceDescriptor*) shmat(shm_res_id, NULL, 0);

    for (int i = 0; i < MAX_RESOURCES; i++) {
        resources[i].available = MAX_INSTANCES;
        resources[i].max = MAX_INSTANCES;
        for (int j = 0; j < MAX_PROCESSES; j++) {
            resources[i].allocated[j] = 0;
            resources[i].request[j] = 0;
        }
    }
}

void fork_worker() {
    pid_t pid = fork();
    if (pid == 0) {
        char arg[16];
        snprintf(arg, sizeof(arg), "%d", msgid);
        execl("./worker", "worker", arg, NULL);
        perror("execl");
        exit(1);
    }
}

void handle_message(Message* msg) {
    int pid = msg->pid % MAX_PROCESSES;
    int rid = msg->resource_id;

    if (strcmp(msg->action, "request") == 0) {
        if (resources[rid].available > 0) {
            resources[rid].available--;
            resources[rid].allocated[pid]++;
            fprintf(log_file, "Granted R%d to P%d at time %d:%d\n", rid, pid, clock_shm->seconds, clock_shm->nanoseconds);
            msg->mtype = msg->pid + 1;
            msgsnd(msgid, msg, sizeof(Message) - sizeof(long), 0);
        } else {
            resources[rid].request[pid]++;
            fprintf(log_file, "P%d waiting for R%d at %d:%d\n", pid, rid, clock_shm->seconds, clock_shm->nanoseconds);
        }
    } else if (strcmp(msg->action, "release") == 0) {
        if (resources[rid].allocated[pid] > 0) {
            resources[rid].allocated[pid]--;
            resources[rid].available++;
            fprintf(log_file, "P%d released R%d at time %d:%d\n", pid, rid, clock_shm->seconds, clock_shm->nanoseconds);
        }
    } else if (strcmp(msg->action, "terminate") == 0) {
        for (int r = 0; r < MAX_RESOURCES; r++) {
            resources[r].available += resources[r].allocated[pid];
            resources[r].allocated[pid] = 0;
            resources[r].request[pid] = 0;
        }
        fprintf(log_file, "P%d terminated at time %d:%d\n", pid, clock_shm->seconds, clock_shm->nanoseconds);
    }
}

int main(int argc, char* argv[]) {
    signal(SIGINT, cleanup);
    signal(SIGTERM, cleanup);
    srand(time(NULL));

    log_file = fopen("oss.log", "w");
    setup_shared_memory();
    msgid = msgget(IPC_PRIVATE, IPC_CREAT | 0666);

    int active_children = 0;
    while ((total_processes_launched < MAX_TOTAL_PROCESSES || active_children > 0) && clock_shm->seconds < 5) {
        advance_clock(0, 100000); // advance 100us

        printf("OSS time: %d:%d | Active: %d | Launched: %d\n", 
            clock_shm->seconds, clock_shm->nanoseconds, active_children, total_processes_launched);
        fflush(stdout);

        Message msg;
        while (msgrcv(msgid, &msg, sizeof(Message) - sizeof(long), 1, IPC_NOWAIT) > 0) {
            handle_message(&msg);
        }

        if (total_processes_launched < MAX_TOTAL_PROCESSES && active_children < MAX_PROCESSES) {
            fork_worker();
            total_processes_launched++;
            active_children++;
        }

        int status;
        pid_t pid;
        while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
            active_children--;
            fprintf(log_file, "OSS: Reaped child PID %d at time %d:%d\n", pid, clock_shm->seconds, clock_shm->nanoseconds);
        }
    }

    printf("OSS terminating after timeout or all processes finished.\n");
    raise(SIGTERM); // safely call cleanup once via signal
    return 0;
}
