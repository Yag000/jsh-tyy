#include "jobs.h"
#include "command.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void destroy_job_table();

job **job_table;
size_t job_table_size;
size_t job_table_capacity;

char *job_status_to_string(job_status status) {
    switch (status) {
        case RUNNING:
            return "Running";
        case STOPPED:
            return "Stopped";
        case DONE:
            return "Done";
        case KILLED:
            return "Killed";
        case DETACHED:
            return "Detached";
    }
    perror("Invalid job status");
    exit(1);
}

job *new_job(command_call *command, pid_t pid, job_status status, job_type type) {
    job *j = malloc(sizeof(job));

    if (j == NULL) {
        perror("malloc");
        return NULL;
    }

    j->command = command;
    j->pid = pid;
    j->id = UNINITIALIZED_JOB_ID;
    j->last_status = status;
    j->type = type;

    return j;
}

void destroy_job(job *j) {
    if (j != NULL) {
        destroy_command_call(j->command);
    }
    free(j);
}

void print_job(job *j) {
    dprintf(STDOUT_FILENO, "[%ld]\t%d\t%s\t", j->id, j->pid, job_status_to_string(j->last_status));
    command_call_print(j->command);
    dprintf(STDOUT_FILENO, "\n");
}

void init_job_table() {
    destroy_job_table();
    job_table = malloc(INITIAL_JOB_TABLE_CAPACITY * sizeof(job *));
    if (job_table == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    job_table_capacity = INITIAL_JOB_TABLE_CAPACITY;
    job_table_size = 0;

    for (size_t i = 0; i < job_table_capacity; i++) {
        job_table[i] = NULL;
    }
}

void destroy_job_table() {
    if (job_table == NULL) {
        return;
    }
    for (size_t i = 0; i < job_table_capacity; i++) {
        destroy_job(job_table[i]);
    }
    free(job_table);
}

int add_job(job *j) {
    if (j == NULL) {
        return -1;
    }
    if (job_table_size == job_table_capacity) {
        job_table_capacity += INITIAL_JOB_TABLE_CAPACITY;
        job_table = reallocarray(job_table, job_table_capacity, sizeof(job *));
        if (job_table == NULL) {
            perror("reallocarray");
            exit(EXIT_FAILURE);
        }
        for (size_t i = job_table_size; i < job_table_capacity; i++) {
            job_table[i] = NULL;
        }
    }
    for (size_t i = 0; i < job_table_capacity; i++) {
        if (job_table[i] == NULL) {
            j->id = i + 1;
            job_table[i] = j;
            job_table_size++;
            return i + 1;
        }
    }

    return -1;
}

int remove_job(size_t id) {
    if (id < 1 || id > job_table_capacity || job_table_size == 0 || job_table == NULL) {
        return 1;
    }

    if (job_table[id - 1] == NULL) {
        return 1;
    }

    destroy_job(job_table[id - 1]);

    job_table[id - 1] = NULL;

    job_table_size--;

    return 0;
}
