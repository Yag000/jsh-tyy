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

    size_t total_commands;
    command_call **call_cd_user_home = parse_command("cd", &total_commands);
    int res = cd(call_cd_user_home[0]);

    handle_int_test(0, res, __LINE__, __FILE__, info);

    destroy_command_call(call_cd_user_home[0]);

    char *new_cwd = getcwd(NULL, PATH_MAX);

    handle_string_test(expected_cwd, new_cwd, __LINE__, __FILE__, info);
    handle_string_test(expected_lwd, lwd, __LINE__, __FILE__, info);

    free(call_cd_user_home);
    free(expected_lwd);
    free(new_cwd);
}

static void test_cd_path_valid(test_info *info) {
    print_test_name("Testing `cd tmp/dir`");

    init_cwd_and_lwd();

    char target_path[] = "tmp/dir";

    char *expected_cwd = realpath(target_path, NULL);
    char *expected_lwd = getcwd(NULL, PATH_MAX);

    size_t total_commands;
    command_call **call_cd_home = parse_command("cd tmp/dir", &total_commands);

    int res = cd(call_cd_home[0]);

    destroy_command_call(call_cd_home[0]);

    char *new_cwd = getcwd(NULL, PATH_MAX);

    handle_int_test(0, res, __LINE__, __FILE__, info);
    handle_string_test(expected_cwd, new_cwd, __LINE__, __FILE__, info);
    handle_string_test(expected_lwd, lwd, __LINE__, __FILE__, info);

    free(call_cd_home);
    free(new_cwd);
    free(expected_cwd);
    free(expected_lwd);
}

static void test_cd_previous_exists(test_info *info) {
    print_test_name("Testing `cd -` when `lwd` exists");

    init_cwd_and_lwd();

    char *expected_cwd = getcwd(NULL, PATH_MAX);

    char lwd_target[] = "tmp/dir/subdir";
    char *expected_lwd = realpath(lwd_target, NULL);

    size_t total_commands;
    command_call **call_cd_subdir = parse_command("cd tmp/dir/subdir", &total_commands);
    command_call **call_cd_previous = parse_command("cd -", &total_commands);

    cd(call_cd_subdir[0]);
    int res = cd(call_cd_previous[0]);

    destroy_command_call(call_cd_subdir[0]);
    destroy_command_call(call_cd_previous[0]);

    char *new_cwd = getcwd(NULL, PATH_MAX);

    handle_int_test(0, res, __LINE__, __FILE__, info);
    handle_string_test(expected_cwd, new_cwd, __LINE__, __FILE__, info);
    handle_string_test(expected_lwd, lwd, __LINE__, __FILE__, info);

    free(call_cd_subdir);
    free(call_cd_previous);
    free(new_cwd);
    free(expected_cwd);
    free(expected_lwd);
}

static void test_cd_previous_non_existent(test_info *info) {
    print_test_name("Testing `cd -` when `lwd` doesn't exist");

    init_cwd_and_lwd();

    char *expected_cwd = getcwd(NULL, PATH_MAX);
    char *expected_lwd = getcwd(NULL, PATH_MAX);

    size_t total_commands;
    command_call **call_cd_previous = parse_command("cd -", &total_commands);

    int res = cd(call_cd_previous[0]);

    destroy_command_call(call_cd_previous[0]);

    char *new_cwd = getcwd(NULL, PATH_MAX);

    handle_int_test(0, res, __LINE__, __FILE__, info);
    handle_string_test(expected_cwd, new_cwd, __LINE__, __FILE__, info);
    handle_string_test(expected_lwd, lwd, __LINE__, __FILE__, info);
    handle_string_test(new_cwd, lwd, __LINE__, __FILE__, info);

    free(call_cd_previous);
    free(expected_cwd);
    free(expected_lwd);
    free(new_cwd);
}

static void test_cd_path_non_existent(test_info *info) {
    print_test_name("Testing with an non-existent dir");

    init_cwd_and_lwd();

    char *expected_cwd = getcwd(NULL, PATH_MAX);
    char *expected_lwd = strdup(lwd);

    int log_fd = open_test_file_to_write("cd_non_existent_dir.log");

    size_t total_commands;
    command_call **call_cd_non_existent = parse_command("cd tmp/dir/does/not/exits", &total_commands);
    call_cd_non_existent[0]->stderr = log_fd;

    int res = cd(call_cd_non_existent[0]);
    close(log_fd);

    destroy_command_call(call_cd_non_existent[0]);

    char *new_cwd = getcwd(NULL, PATH_MAX);

    handle_int_test(1, res, __LINE__, __FILE__, info);
    handle_string_test(expected_cwd, new_cwd, __LINE__, __FILE__, info);
    handle_string_test(expected_lwd, lwd, __LINE__, __FILE__, info);

    free(call_cd_non_existent);
    free(expected_cwd);
    free(expected_lwd);
    free(new_cwd);
}

static void test_cd_path_is_not_dir(test_info *info) {
    print_test_name("Testing cd on a non-directory file");

    init_cwd_and_lwd();

    char *expected_cwd = getcwd(NULL, PATH_MAX);
    char *expected_lwd = strdup(lwd);

    int log_fd = open_test_file_to_write("cd_non_dir.log");

    size_t total_commands;
    command_call **call_cd_non_dir = parse_command("cd tmp/file", &total_commands);
    call_cd_non_dir[0]->stderr = log_fd;

    int res = cd(call_cd_non_dir[0]);
    close(log_fd);

    destroy_command_call(call_cd_non_dir[0]);

    char *new_cwd = getcwd(NULL, PATH_MAX);

    handle_int_test(1, res, __LINE__, __FILE__, info);
    handle_string_test(expected_cwd, new_cwd, __LINE__, __FILE__, info);
    handle_string_test(expected_lwd, lwd, __LINE__, __FILE__, info);

    free(call_cd_non_dir);
    free(expected_cwd);
    free(expected_lwd);
    free(new_cwd);
}

static void test_cd_symlink(test_info *info) {
    print_test_name("Testing cd on a symlink");

    init_cwd_and_lwd();

    char target_path[] = "tmp/dir/symlink_target";
    char *expected_cwd = realpath(target_path, NULL);
    char *expected_lwd = getcwd(NULL, PATH_MAX);

    int log_fd = open_test_file_to_write("cd_symlink.log");

    size_t total_commands;
    command_call **call_cd_symlink = parse_command("cd tmp/dir/subdir/symlink_source", &total_commands);
    call_cd_symlink[0]->stderr = log_fd;

    int res = cd(call_cd_symlink[0]);
    close(log_fd);

    destroy_command_call(call_cd_symlink[0]);

    char *new_cwd = getcwd(NULL, PATH_MAX);

    handle_int_test(0, res, __LINE__, __FILE__, info);
    handle_string_test(expected_cwd, new_cwd, __LINE__, __FILE__, info);
    handle_string_test(expected_lwd, lwd, __LINE__, __FILE__, info);

    free(call_cd_symlink);
    free(expected_cwd);
    free(expected_lwd);
    free(new_cwd);
}
