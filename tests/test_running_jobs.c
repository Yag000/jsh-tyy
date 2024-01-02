#include <assert.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../src/internals.h"
#include "../src/jobs.h"
#include "test_core.h"
#include "utils.h"

void test_case_are_jobs_running_no_jobs(test_info *);
void test_case_are_jobs_running_one_job(test_info *);
void test_case_are_jobs_running_one_instant_job(test_info *);
void test_case_are_jobs_running_kill(test_info *);
void test_case_are_jobs_running_stop(test_info *);

test_info *test_running_jobs() {
    // Test setup
    print_test_header("running jobs");
    clock_t start = clock();
    test_info *info = create_test_info();

    // Add tests here
    test_case_are_jobs_running_no_jobs(info);
    test_case_are_jobs_running_one_job(info);
    test_case_are_jobs_running_one_instant_job(info);
    test_case_are_jobs_running_kill(info);
    test_case_are_jobs_running_stop(info);

    // End of tests
    init_job_table();
    info->time = clock_ticks_to_seconds(clock() - start);
    print_test_footer("running jobs", info);
    return info;
}

void test_case_are_jobs_running_no_jobs(test_info *info) {
    print_test_name("Testing are_jobs_running - No jobs");

    init_job_table();

    handle_boolean_test(0, are_jobs_running(), __LINE__, __FILE__, info);
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

command_result *helper_execute_bg(char *command) {
    command_call *command_call = parse_command(command);
    command_call->background = 1;
    command_result *result = mute_command_execution(command_call);
    return result;
}

void test_case_are_jobs_running_one_job(test_info *info) {
    print_test_name("Testing are_jobs_running - One running job");

    init_job_table();

    command_result *result = helper_execute_bg("sleep 100");

    helper_mute_update_jobs("test_running_jobs_one_job.log");

    handle_int_test(1, are_jobs_running(), __LINE__, __FILE__, info);

    handle_int_test(RUNNING, job_table[0]->subjobs[0]->last_status, __LINE__, __FILE__, info);

    kill(result->pid, SIGKILL);

    destroy_command_result(result);
    init_job_table();
}

void test_case_are_jobs_running_one_instant_job(test_info *info) {
    if (!allow_slow) {
        return;
    }

    print_test_name("Testing are_jobs_running - One instant job");

    init_job_table();

    int fd = open_test_file_to_write("test_running_jobs_one_instant_job_out.log");
    command_call *command_call = parse_command("ls");
    command_call->background = 1;
    command_call->stdout = fd;
    command_result *result = execute_command_call(command_call);

    // Let the job finish
    sleep(1);

    helper_mute_update_jobs("test_running_jobs_one_instant_job.log");

    handle_int_test(0, are_jobs_running(), __LINE__, __FILE__, info);

    destroy_command_result(result);
    init_job_table();
}

void test_case_are_jobs_running_kill(test_info *info) {
    if (!allow_slow) {
        return;
    }

    print_test_name("Testing are_jobs_running - One killed job");

    init_job_table();

    command_result *result = helper_execute_bg("sleep 100");

    kill(result->pid, SIGKILL);

    // Let the job finish
    sleep(1);

    helper_mute_update_jobs("test_running_jobs_kill.log");

    handle_int_test(0, are_jobs_running(), __LINE__, __FILE__, info);

    destroy_command_result(result);
    init_job_table();
}

void test_case_are_jobs_running_stop(test_info *info) {
    if (!allow_slow) {
        return;
    }

    print_test_name("Testing are_jobs_running - One stopped job");

    init_job_table();

    command_result *result = helper_execute_bg("sleep 100");

    kill(result->pid, SIGSTOP);

    // Let the job finish
    sleep(1);

    helper_mute_update_jobs("test_running_jobs_stop.log");

    handle_int_test(1, are_jobs_running(), __LINE__, __FILE__, info);
    handle_int_test(STOPPED, job_table[0]->subjobs[0]->last_status, __LINE__, __FILE__, info);

    kill(result->pid, SIGKILL);

    destroy_command_result(result);
    init_job_table();
}
