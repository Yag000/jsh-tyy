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

#define NUM_TEST 7

static void test_cd_user_home(test_info *);
static void test_cd_path_valid(test_info *);
static void test_cd_previous_exists(test_info *);
static void test_cd_previous_non_existent(test_info *);
static void test_cd_path_non_existent(test_info *);
static void test_cd_path_is_not_dir(test_info *);
static void test_cd_symlink(test_info *);

test_info *test_cd() {
    // Remember project directory
    memset(project_dir, '\0', PATH_MAX);
    getcwd(project_dir, PATH_MAX);

    test_case cases[NUM_TEST] = {QUICK_CASE("Testing `cd`", test_cd_user_home),
                          QUICK_CASE("Testing `cd tmp/dir`", test_cd_path_valid),
                          QUICK_CASE("Testing `cd -` when `lwd` exists", test_cd_previous_exists),
                          QUICK_CASE("Testing `cd -` when `lwd` doesn't exist", test_cd_previous_non_existent),
                          QUICK_CASE("Testing with an non-existent dir", test_cd_path_non_existent),
                          QUICK_CASE("Testing cd on a non-directory file", test_cd_path_is_not_dir),
                          QUICK_CASE("Testing cd on a symlink", test_cd_symlink)};

    test_info *info = run_cases("cd", cases, NUM_TEST);

    init_cwd_and_lwd();
    return info;
}

void init_cwd_and_lwd() {
    chdir(project_dir);
    memset(lwd, '\0', PATH_MAX);
}

static void test_cd_user_home(test_info *info) {
    init_cwd_and_lwd();

    char *expected_cwd = getenv("HOME");
    char *expected_lwd = getcwd(NULL, PATH_MAX);

    command *call_cd_user_home = parse_command("cd");
    command_result *result = execute_command(call_cd_user_home);

    CINTA_ASSERT_INT(0, result->exit_code, info);

    destroy_command_result(result);

    char *new_cwd = getcwd(NULL, PATH_MAX);

    CINTA_ASSERT_STRING(expected_cwd, new_cwd, info);
    CINTA_ASSERT_STRING(expected_lwd, lwd, info);

    free(expected_lwd);
    free(new_cwd);
}

static void test_cd_path_valid(test_info *info) {
    init_cwd_and_lwd();

    char target_path[] = "tmp/dir";

    char *expected_cwd = realpath(target_path, NULL);
    char *expected_lwd = getcwd(NULL, PATH_MAX);

    command *call_cd_home = parse_command("cd tmp/dir");
    command_result *res = execute_command(call_cd_home);

    char *new_cwd = getcwd(NULL, PATH_MAX);

    CINTA_ASSERT_INT(0, res->exit_code, info);
    CINTA_ASSERT_STRING(expected_cwd, new_cwd, info);
    CINTA_ASSERT_STRING(expected_lwd, lwd, info);

    free(new_cwd);
    free(expected_cwd);
    free(expected_lwd);

    destroy_command_result(res);
}

static void test_cd_previous_exists(test_info *info) {
    init_cwd_and_lwd();

    char *expected_cwd = getcwd(NULL, PATH_MAX);

    char lwd_target[] = "tmp/dir/subdir";
    char *expected_lwd = realpath(lwd_target, NULL);

    command *call_cd_subdir = parse_command("cd tmp/dir/subdir");
    command *call_cd_previous = parse_command("cd -");

    destroy_command_result(execute_command(call_cd_subdir));
    command_result *res = execute_command(call_cd_previous);

    char *new_cwd = getcwd(NULL, PATH_MAX);

    CINTA_ASSERT_INT(0, res->exit_code, info);
    CINTA_ASSERT_STRING(expected_cwd, new_cwd, info);
    CINTA_ASSERT_STRING(expected_lwd, lwd, info);

    free(new_cwd);
    free(expected_cwd);
    free(expected_lwd);

    destroy_command_result(res);
}

static void test_cd_previous_non_existent(test_info *info) {
    init_cwd_and_lwd();

    char *expected_cwd = getcwd(NULL, PATH_MAX);
    char *expected_lwd = getcwd(NULL, PATH_MAX);

    command *call_cd_previous = parse_command("cd -");
    command_result *res = execute_command(call_cd_previous);

    char *new_cwd = getcwd(NULL, PATH_MAX);

    CINTA_ASSERT_INT(0, res->exit_code, info);
    CINTA_ASSERT_STRING(expected_cwd, new_cwd, info);
    CINTA_ASSERT_STRING(expected_lwd, lwd, info);
    CINTA_ASSERT_STRING(new_cwd, lwd, info);

    free(expected_cwd);
    free(expected_lwd);
    free(new_cwd);

    destroy_command_result(res);
}

static void test_cd_path_non_existent(test_info *info) {
    init_cwd_and_lwd();

    char *expected_cwd = getcwd(NULL, PATH_MAX);
    char *expected_lwd = strdup(lwd);

    int log_fd = open_test_file_to_write("cd_non_existent_dir.log");

    command *call_cd_non_existent = parse_command("cd tmp/dir/does/not/exits");
    call_cd_non_existent->command_calls[0]->stderr = log_fd;

    command_result *res = execute_command(call_cd_non_existent);

    char *new_cwd = getcwd(NULL, PATH_MAX);

    CINTA_ASSERT_INT(1, res->exit_code, info);
    CINTA_ASSERT_STRING(expected_cwd, new_cwd, info);
    CINTA_ASSERT_STRING(expected_lwd, lwd, info);

    free(expected_cwd);
    free(expected_lwd);
    free(new_cwd);

    destroy_command_result(res);
}

static void test_cd_path_is_not_dir(test_info *info) {
    init_cwd_and_lwd();

    char *expected_cwd = getcwd(NULL, PATH_MAX);
    char *expected_lwd = strdup(lwd);

    int log_fd = open_test_file_to_write("cd_non_dir.log");

    command *call_cd_non_dir = parse_command("cd tmp/file");
    call_cd_non_dir->command_calls[0]->stderr = log_fd;

    command_result *res = execute_command(call_cd_non_dir);

    char *new_cwd = getcwd(NULL, PATH_MAX);

    CINTA_ASSERT_INT(1, res->exit_code, info);
    CINTA_ASSERT_STRING(expected_cwd, new_cwd, info);
    CINTA_ASSERT_STRING(expected_lwd, lwd, info);

    free(expected_cwd);
    free(expected_lwd);
    free(new_cwd);

    destroy_command_result(res);
}

static void test_cd_symlink(test_info *info) {
    init_cwd_and_lwd();

    char target_path[] = "tmp/dir/symlink_target";
    char *expected_cwd = realpath(target_path, NULL);
    char *expected_lwd = getcwd(NULL, PATH_MAX);

    int log_fd = open_test_file_to_write("cd_symlink.log");

    command *call_cd_symlink = parse_command("cd tmp/dir/subdir/symlink_source");
    call_cd_symlink->command_calls[0]->stderr = log_fd;

    command_result *res = execute_command(call_cd_symlink);

    char *new_cwd = getcwd(NULL, PATH_MAX);

    CINTA_ASSERT_INT(0, res->exit_code, info);
    CINTA_ASSERT_STRING(expected_cwd, new_cwd, info);
    CINTA_ASSERT_STRING(expected_lwd, lwd, info);

    free(expected_cwd);
    free(expected_lwd);
    free(new_cwd);

    destroy_command_result(res);
}
