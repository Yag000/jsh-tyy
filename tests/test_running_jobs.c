#include <assert.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../src/internals.h"
#include "../src/jobs.h"
#include "test_core.h"
#include "utils.h"

#define NUM_TEST 5

void test_case_are_jobs_running_no_jobs(test_info *);
void test_case_are_jobs_running_one_job(test_info *);
void test_case_are_jobs_running_one_instant_job(test_info *);
void test_case_are_jobs_running_kill(test_info *);
void test_case_are_jobs_running_stop(test_info *);

test_info *test_running_jobs() {
    test_case cases[NUM_TEST] = {
        QUICK_CASE("Testing are_jobs_running - No jobs", test_case_are_jobs_running_no_jobs),
        QUICK_CASE("Testing are_jobs_running - One running job", test_case_are_jobs_running_one_job),
        SLOW_CASE("Testing are_jobs_running - One instant job", test_case_are_jobs_running_one_instant_job),
        SLOW_CASE("Testing are_jobs_running - One killed job", test_case_are_jobs_running_kill),
        SLOW_CASE("Testing are_jobs_running - One stopped job", test_case_are_jobs_running_stop)};

    test_info *info = cinta_run_cases("running jobs", cases, NUM_TEST);

    init_job_table();
    return info;
}

void test_case_are_jobs_running_no_jobs(test_info *info) {

    init_job_table();

    CINTA_ASSERT_INT(0, are_jobs_running(), info);
}

void helper_mute_update_jobs(char *file_name) {
    int fd = open_test_file_to_write(file_name);
    assert(fd != -1);
    int current_stderr = dup(STDERR_FILENO);
    dup2(fd, STDERR_FILENO);
    update_jobs();
    dup2(current_stderr, STDERR_FILENO);
    close(current_stderr);
    close(fd);
}

command_result *helper_execute_bg(char *scommand) {
    command *command = parse_command(scommand);
    command->background = 1;
    command_result *result = mute_command_execution(command);
    return result;
}

void test_case_are_jobs_running_one_job(test_info *info) {

    init_job_table();

    command_result *result = helper_execute_bg("sleep 100");

    helper_mute_update_jobs("test_running_jobs_one_job.log");

    CINTA_ASSERT_INT(1, are_jobs_running(), info);

    CINTA_ASSERT_INT(RUNNING, job_table[0]->subjobs[0]->last_status, info);

    kill(result->pid, SIGKILL);

    destroy_command_result(result);
    init_job_table();
}

void test_case_are_jobs_running_one_instant_job(test_info *info) {
    init_job_table();

    int fd = open_test_file_to_write("test_running_jobs_one_instant_job_out.log");
    command *command = parse_command("ls");
    command->background = 1;
    command->command_calls[0]->stdout = fd;
    command_result *result = execute_command(command);

    // Let the job finish
    sleep(1);

    helper_mute_update_jobs("test_running_jobs_one_instant_job.log");

    CINTA_ASSERT_INT(0, are_jobs_running(), info);

    destroy_command_result(result);
    init_job_table();
}

void test_case_are_jobs_running_kill(test_info *info) {
    init_job_table();

    command_result *result = helper_execute_bg("sleep 100");

    kill(result->pid, SIGKILL);

    // Let the job finish
    sleep(1);

    helper_mute_update_jobs("test_running_jobs_kill.log");

    CINTA_ASSERT_INT(0, are_jobs_running(), info);

    destroy_command_result(result);
    init_job_table();
}

void test_case_are_jobs_running_stop(test_info *info) {
    init_job_table();

    command_result *result = helper_execute_bg("sleep 100");

    kill(result->pid, SIGSTOP);

    // Let the job finish
    sleep(1);

    helper_mute_update_jobs("test_running_jobs_stop.log");

    CINTA_ASSERT_INT(1, are_jobs_running(), info);
    CINTA_ASSERT_INT(STOPPED, job_table[0]->subjobs[0]->last_status, info);

    kill(result->pid, SIGKILL);

    destroy_command_result(result);
    init_job_table();
}
