#ifndef JOBS_H
#define JOBS_H

#include "command.h"
#include <linux/limits.h>
#include <sys/types.h>

#define UNINITIALIZED_JOB_ID 0;

typedef enum job_status { RUNNING, STOPPED, DETACHED, KILLED, DONE } job_status;

/** Returns a string representation of the job status. */
char *job_status_to_string(job_status);

typedef enum job_type { FOREGROUND, BACKGROUND } job_type;

typedef struct job {
    command_call *command;
    pid_t pid;
    size_t id; // Job number, 1-indexed
    job_status last_status;
    job_type type;
} job;

/** Returns a new job with the given command call, pid, last status and type. */
job *new_job(command_call *, pid_t, job_status, job_type);

/** Frees the memory allocated for the job.
 *  Calls `destroy_command_call` to free `job-> call`.
 */
void destroy_job(job *);

/** Prints the job to stdout, following the format:
 *  [id] pid status command
 */
void print_job(job *);

/** Original capacity of the job table. Every time the job table expands
 *  it will be expanded by this number */
#define INITIAL_JOB_TABLE_CAPACITY 64

extern job **job_table;

/** Amount of jobs in the job table. */
extern size_t job_table_size;

/** Capacity of the job table. */
extern size_t job_table_capacity;

/** Initializes the job table with the initial size.
 *  It will destroy the job table if it was already initialized,
 *  using `destroy_job_table`.
 * */
void init_job_table();

/** Adds a new job to the job table. Expands it
 * if needed.
 *
 * Returns the job number of the added job.
 * If there was an error, returns -1.
 */
int add_job(job *);

/** Removes the job with the job number given as argument
 *  from the job table.
 *
 *  Returns 0 if the job was removed, 1 otherwise.
 */
int remove_job(size_t);

/** Destroys the job table. */
void destroy_job_table();

#endif // JOBS_H
