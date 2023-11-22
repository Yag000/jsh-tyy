#include "../src/internals.h"
#include "../src/string_utils.h"
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

    // End of tests
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
    char *expected_lwd = calloc(PATH_MAX, sizeof(char));
    getcwd(expected_lwd, PATH_MAX);

    size_t size_argv;
    char **argv = split_string("cd", " ", &size_argv);

    command_call *call_cd_user_home = new_command_call(size_argv, argv);

    int res = cd(call_cd_user_home);

    destroy_command_call(call_cd_user_home);

    handle_int_test(0, res, __LINE__, __FILE__, info);

    char *new_cwd = calloc(PATH_MAX, sizeof(char));
    getcwd(new_cwd, PATH_MAX);

    handle_string_test(expected_cwd, new_cwd, __LINE__, __FILE__, info);
    handle_string_test(expected_lwd, lwd, __LINE__, __FILE__, info);

    free(expected_lwd);
    free(new_cwd);
}

static void test_cd_path_valid(test_info *info) {
    print_test_name("Testing `cd tmp/dir`");

    init_cwd_and_lwd();

    char target_path[] = "tmp/dir";

    char *expected_cwd = calloc(PATH_MAX, sizeof(char));
    realpath(target_path, expected_cwd);

    char *expected_lwd = calloc(PATH_MAX, sizeof(char));
    getcwd(expected_lwd, PATH_MAX);

    size_t size_argv;
    char **argv = split_string("cd tmp/dir", " ", &size_argv);

    command_call *call_cd_home = new_command_call(size_argv, argv);

    int res = cd(call_cd_home);

    destroy_command_call(call_cd_home);

    char *new_cwd = calloc(PATH_MAX, sizeof(char));
    getcwd(new_cwd, PATH_MAX);

    handle_int_test(0, res, __LINE__, __FILE__, info);
    handle_string_test(expected_cwd, new_cwd, __LINE__, __FILE__, info);
    handle_string_test(expected_lwd, lwd, __LINE__, __FILE__, info);

    free(new_cwd);
    free(expected_cwd);
    free(expected_lwd);
}

static void test_cd_previous_exists(test_info *info) {
    print_test_name("Testing `cd -` when `lwd` exists");

    init_cwd_and_lwd();

    char *expected_cwd = calloc(PATH_MAX, sizeof(char));
    getcwd(expected_cwd, PATH_MAX);

    char lwd_target[] = "tmp/dir/subdir";
    char *expected_lwd = calloc(PATH_MAX, sizeof(char));
    realpath(lwd_target, expected_lwd);

    size_t size_argv1;
    char **argv1 = split_string("cd tmp/dir/subdir", " ", &size_argv1);

    size_t size_argv2;
    char **argv2 = split_string("cd -", " ", &size_argv2);

    command_call *call_cd_subdir = new_command_call(size_argv1, argv1);
    command_call *call_cd_previous = new_command_call(size_argv2, argv2);

    cd(call_cd_subdir);
    int res = cd(call_cd_previous);

    destroy_command_call(call_cd_subdir);
    destroy_command_call(call_cd_previous);

    handle_int_test(0, res, __LINE__, __FILE__, info);
    handle_string_test(expected_lwd, lwd, __LINE__, __FILE__, info);

    free(expected_cwd);
    free(expected_lwd);
}

static void test_cd_previous_non_existent(test_info *info) {
    print_test_name("Testing `cd -` when `lwd` doesn't exist");

    init_cwd_and_lwd();

    char *expected_cwd = calloc(PATH_MAX, sizeof(char));
    getcwd(expected_cwd, PATH_MAX);

    char *expected_lwd = calloc(PATH_MAX, sizeof(char));
    getcwd(expected_lwd, PATH_MAX);

    size_t size_argv;
    char **argv = split_string("cd -", " ", &size_argv);

    command_call *call_cd_previous = new_command_call(size_argv, argv);

    int res = cd(call_cd_previous);

    destroy_command_call(call_cd_previous);

    handle_int_test(0, res, __LINE__, __FILE__, info);
    handle_string_test(expected_lwd, lwd, __LINE__, __FILE__, info);

    free(expected_cwd);
    free(expected_lwd);
}

static void test_cd_path_non_existent(test_info *info) {
    print_test_name("Testing with an non-existent dir");

    init_cwd_and_lwd();

    char *expected_cwd = calloc(PATH_MAX, sizeof(char));
    getcwd(expected_cwd, PATH_MAX);

    char *expected_lwd = calloc(PATH_MAX, sizeof(char));
    memmove(expected_lwd, lwd, PATH_MAX);

    size_t size_argv;
    char **argv = split_string("cd tmp/dir/does/not/exits", " ", &size_argv);

    int log_fd = open_test_file_to_write("cd_non_existent_dir.log");

    command_call *call_cd_non_existent = new_command_call(size_argv, argv);
    call_cd_non_existent->stderr = log_fd;

    int res = cd(call_cd_non_existent);
    close(log_fd);

    destroy_command_call(call_cd_non_existent);

    handle_int_test(1, res, __LINE__, __FILE__, info);
    handle_string_test(expected_lwd, lwd, __LINE__, __FILE__, info);

    free(expected_cwd);
    free(expected_lwd);
}

static void test_cd_path_is_not_dir(test_info *info) {
    print_test_name("Testing cd on a non-directory file");

    init_cwd_and_lwd();

    char *expected_cwd = calloc(PATH_MAX, sizeof(char));
    getcwd(expected_cwd, PATH_MAX);

    char *expected_lwd = calloc(PATH_MAX, sizeof(char));
    memmove(expected_lwd, lwd, PATH_MAX);

    size_t size_argv;
    char **argv = split_string("cd tmp/file", " ", &size_argv);

    int log_fd = open_test_file_to_write("cd_non_dir.log");

    command_call *call_cd_non_dir = new_command_call(size_argv, argv);
    call_cd_non_dir->stderr = log_fd;

    int res = cd(call_cd_non_dir);
    close(log_fd);

    destroy_command_call(call_cd_non_dir);

    handle_int_test(1, res, __LINE__, __FILE__, info);
    handle_string_test(expected_lwd, lwd, __LINE__, __FILE__, info);

    free(expected_cwd);
    free(expected_lwd);
}