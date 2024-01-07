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

typedef struct subjob {
    char *command;
    pid_t pid;
    job_status last_status;
} subjob;

typedef struct job {
    size_t id; // Job number, 1-indexed
    size_t subjobs_size;
    pid_t pgid; // Process group id for all the subjobs
    subjob **subjobs;
    job_type type;
} job;

/** Returns a new subjob with the given command call, pid, last status and type. */
subjob *new_subjob(command_call *, pid_t, job_status);

/** Frees the memory allocated for the subjob.
 *  Calls `destroy_command_call` to free `subjob-> command`.
 */
void destroy_subjob(subjob *);

/** Returns a new job with the given subjobs size, the
 * subjobs will be set to NULL. */
job *new_job(size_t, job_type);

/** Frees the memory allocated for the job.
 *  Calls `destroy_subjob` on all the job's subjobs.
 */
void destroy_job(job *);

/** Prints the job to `fd`, following the format:
 *  [id] pid status command
 */
void print_job(job *, int fd);

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

/** Updates the jobs in the job table and prints the ones that
 *  have finished.
 */
void update_jobs();

/** Return 1 if there are jobs running or stopped, 0 otherwise.
 * It looks the last status of the jobs in the job table, so it
 * could not be accurate, should be used after a call to `update_jobs`
 * for accurate results.
 * */
int are_jobs_running();

/**
 * Puts the job to the foreground.
 * Returns 1 if the job was put in the foreground,
 * 0 otherwise.
 */
int put_job_in_foreground(job *);

/**
 * Continues the job int the background.
 * Returns 1 if we were able to continue the job in the background,
 * 0 otherwise.
 */
int continue_job_in_background(job *);

/**
 * Returns the job with the given job number.
 * If there is no job with that job number or an error occured,
 * returns NULL.
 */
job *get_job(char *job_id_string, int error_fd);

#endif // JOBS_H
