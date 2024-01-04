#include "../src/internals.h"
#include "test_core.h"
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static char project_dir[PATH_MAX];

void init_cwd_and_lwd();

static void test_cd_user_home(test_info *);
static void test_cd_path_valid(test_info *);
static void test_cd_previous_exists(test_info *);
static void test_cd_previous_non_existent(test_info *);
static void test_cd_path_non_existent(test_info *);
static void test_cd_path_is_not_dir(test_info *);
static void test_cd_symlink(test_info *);

test_info *test_cd() {
    // Test setup
    print_test_header("cd");
    clock_t start = clock();
    test_info *info = create_test_info();

    // Remember project directory
    memset(project_dir, '\0', PATH_MAX);
    getcwd(project_dir, PATH_MAX);

    // Add tests here
    test_cd_user_home(info);
    test_cd_path_valid(info);
    test_cd_previous_exists(info);
    test_cd_previous_non_existent(info);
    test_cd_path_non_existent(info);
    test_cd_path_is_not_dir(info);
    test_cd_symlink(info);

    // End of tests
    init_cwd_and_lwd();
    info->time = clock_ticks_to_seconds(clock() - start);
    print_test_footer("cd", info);
    return info;
}

void init_cwd_and_lwd() {
    chdir(project_dir);
    memset(lwd, '\0', PATH_MAX);
}

static void test_cd_user_home(test_info *info) {
    print_test_name("Testing `cd`");

    init_cwd_and_lwd();

    char *expected_cwd = getenv("HOME");
    char *expected_lwd = getcwd(NULL, PATH_MAX);

    command *call_cd_user_home = parse_command("cd");
    command_result *result = execute_command(call_cd_user_home);

    handle_int_test(0, result->exit_code, __LINE__, __FILE__, info);

    destroy_command_result(result);

    char *new_cwd = getcwd(NULL, PATH_MAX);

    handle_string_test(expected_cwd, new_cwd, __LINE__, __FILE__, info);
    handle_string_test(expected_lwd, lwd, __LINE__, __FILE__, info);

    free(expected_lwd);
    free(new_cwd);
}

static void test_cd_path_valid(test_info *info) {
    print_test_name("Testing `cd tmp/dir`");

    init_cwd_and_lwd();

    char target_path[] = "tmp/dir";

    char *expected_cwd = realpath(target_path, NULL);
    char *expected_lwd = getcwd(NULL, PATH_MAX);

    command *call_cd_home = parse_command("cd tmp/dir");
    command_result *res = execute_command(call_cd_home);

    char *new_cwd = getcwd(NULL, PATH_MAX);

    handle_int_test(0, res->exit_code, __LINE__, __FILE__, info);
    handle_string_test(expected_cwd, new_cwd, __LINE__, __FILE__, info);
    handle_string_test(expected_lwd, lwd, __LINE__, __FILE__, info);

    free(new_cwd);
    free(expected_cwd);
    free(expected_lwd);

    destroy_command_result(res);
}

static void test_cd_previous_exists(test_info *info) {
    print_test_name("Testing `cd -` when `lwd` exists");

    init_cwd_and_lwd();

    char *expected_cwd = getcwd(NULL, PATH_MAX);

    char lwd_target[] = "tmp/dir/subdir";
    char *expected_lwd = realpath(lwd_target, NULL);

    command *call_cd_subdir = parse_command("cd tmp/dir/subdir");
    command *call_cd_previous = parse_command("cd -");

    destroy_command_result(execute_command(call_cd_subdir));
    command_result *res = execute_command(call_cd_previous);

    char *new_cwd = getcwd(NULL, PATH_MAX);

    handle_int_test(0, res->exit_code, __LINE__, __FILE__, info);
    handle_string_test(expected_cwd, new_cwd, __LINE__, __FILE__, info);
    handle_string_test(expected_lwd, lwd, __LINE__, __FILE__, info);

    free(new_cwd);
    free(expected_cwd);
    free(expected_lwd);

    destroy_command_result(res);
}

static void test_cd_previous_non_existent(test_info *info) {
    print_test_name("Testing `cd -` when `lwd` doesn't exist");

    init_cwd_and_lwd();

    char *expected_cwd = getcwd(NULL, PATH_MAX);
    char *expected_lwd = getcwd(NULL, PATH_MAX);

    command *call_cd_previous = parse_command("cd -");
    command_result *res = execute_command(call_cd_previous);

    char *new_cwd = getcwd(NULL, PATH_MAX);

    handle_int_test(0, res->exit_code, __LINE__, __FILE__, info);
    handle_string_test(expected_cwd, new_cwd, __LINE__, __FILE__, info);
    handle_string_test(expected_lwd, lwd, __LINE__, __FILE__, info);
    handle_string_test(new_cwd, lwd, __LINE__, __FILE__, info);

    free(expected_cwd);
    free(expected_lwd);
    free(new_cwd);

    destroy_command_result(res);
}

static void test_cd_path_non_existent(test_info *info) {
    print_test_name("Testing with an non-existent dir");

    init_cwd_and_lwd();

    char *expected_cwd = getcwd(NULL, PATH_MAX);
    char *expected_lwd = strdup(lwd);

    int log_fd = open_test_file_to_write("cd_non_existent_dir.log");

    command *call_cd_non_existent = parse_command("cd tmp/dir/does/not/exits");
    call_cd_non_existent->call->stderr = log_fd;

    command_result *res = execute_command(call_cd_non_existent);

    char *new_cwd = getcwd(NULL, PATH_MAX);

    handle_int_test(1, res->exit_code, __LINE__, __FILE__, info);
    handle_string_test(expected_cwd, new_cwd, __LINE__, __FILE__, info);
    handle_string_test(expected_lwd, lwd, __LINE__, __FILE__, info);

    free(expected_cwd);
    free(expected_lwd);
    free(new_cwd);

    destroy_command_result(res);
}

static void test_cd_path_is_not_dir(test_info *info) {
    print_test_name("Testing cd on a non-directory file");

    init_cwd_and_lwd();

    char *expected_cwd = getcwd(NULL, PATH_MAX);
    char *expected_lwd = strdup(lwd);

    int log_fd = open_test_file_to_write("cd_non_dir.log");

    command *call_cd_non_dir = parse_command("cd tmp/file");
    call_cd_non_dir->call->stderr = log_fd;

    command_result *res = execute_command(call_cd_non_dir);

    char *new_cwd = getcwd(NULL, PATH_MAX);

    handle_int_test(1, res->exit_code, __LINE__, __FILE__, info);
    handle_string_test(expected_cwd, new_cwd, __LINE__, __FILE__, info);
    handle_string_test(expected_lwd, lwd, __LINE__, __FILE__, info);

    free(expected_cwd);
    free(expected_lwd);
    free(new_cwd);

    destroy_command_result(res);
}

static void test_cd_symlink(test_info *info) {
    print_test_name("Testing cd on a symlink");

    init_cwd_and_lwd();

    char target_path[] = "tmp/dir/symlink_target";
    char *expected_cwd = realpath(target_path, NULL);
    char *expected_lwd = getcwd(NULL, PATH_MAX);

    int log_fd = open_test_file_to_write("cd_symlink.log");

    command *call_cd_symlink = parse_command("cd tmp/dir/subdir/symlink_source");
    call_cd_symlink->call->stderr = log_fd;

    command_result *res = execute_command(call_cd_symlink);

    char *new_cwd = getcwd(NULL, PATH_MAX);

    handle_int_test(0, res->exit_code, __LINE__, __FILE__, info);
    handle_string_test(expected_cwd, new_cwd, __LINE__, __FILE__, info);
    handle_string_test(expected_lwd, lwd, __LINE__, __FILE__, info);

    free(expected_cwd);
    free(expected_lwd);
    free(new_cwd);

    destroy_command_result(res);
}
