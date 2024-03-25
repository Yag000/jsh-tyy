#include "../src/command.h"
#include "../src/internals.h"
#include "../src/jobs.h"
#include "test_core.h"

#include <fcntl.h>
#include <strings.h>
#include <unistd.h>

#define NUM_TEST 4

void test_parse_multiple_same_redirections(test_info *info);
void test_parse_errors(test_info *info);
void test_pipelines_and_redirections(test_info *info);
void test_substitutions_and_redirections(test_info *info);

test_info *test_redirection_parsing() {
    test_case cases[NUM_TEST] = {SLOW_CASE("Parse multiple same redirections", test_parse_multiple_same_redirections),
                                 QUICK_CASE("Parse errors", test_parse_errors),
                                 QUICK_CASE("Pipelines and redirections", test_pipelines_and_redirections),
                                 QUICK_CASE("Substitutions and redirections", test_substitutions_and_redirections)};

    return cinta_run_cases("redirection_parsing", cases, NUM_TEST);
}

int assert_file_is_empty(const int fd) {
    char buf[10];
    int nb_read = read(fd, buf, 10);

    return nb_read == 0;
}

void test_parse_multiple_same_redirections(test_info *info) {
    if (fork() == 0) {
        execlp("rm", "rm", "tmp/output0", "tmp/output1", "tmp/output2", NULL);
        exit(2);
    }

    // Let the child process finish
    sleep(1);

    command *command = parse_command("echo abcdefghijkl > tmp/output0 >> tmp/output1 >| tmp/output2");

    command->background = 1;

    command_result *result = execute_command(command);

    // Let the child process finish
    sleep(1);

    int fd_output0 = open("tmp/output0", O_RDONLY);
    CINTA_ASSERT_INT(1, fd_output0 >= 0, info);

    int fd_output1 = open("tmp/output1", O_RDONLY);
    CINTA_ASSERT_INT(1, fd_output1 >= 0, info);

    int fd_output2 = open("tmp/output2", O_RDONLY);
    CINTA_ASSERT_INT(1, fd_output2 >= 0, info);

    CINTA_ASSERT_INT(1, assert_file_is_empty(fd_output0), info);
    CINTA_ASSERT_INT(1, assert_file_is_empty(fd_output1), info);

    char *buf = "abcdefghijkl";
    char buffer[strlen(buf) + 1];
    read(fd_output2, buffer, 255);
    buffer[strlen(buf)] = '\0';
    CINTA_ASSERT_STRING(buffer, buf, info);

    close(fd_output0);
    close(fd_output1);
    close(fd_output2);

    destroy_command_result(result);

    init_job_table();
}

void test_parse_errors(test_info *info) {

    command *command = parse_command(" > ");
    CINTA_ASSERT_NULL(command, info);

    command = parse_command(" < /tmp/1");
    CINTA_ASSERT_NULL(command, info);

    command = parse_command(" > /tmp/1");
    CINTA_ASSERT_NULL(command, info);

    command = parse_command(" echo < ");
    CINTA_ASSERT_NULL(command, info);

    command = parse_command("| echo");
    CINTA_ASSERT_NULL(command, info);

    command = parse_command("<( echo )");
    CINTA_ASSERT_NULL(command, info);

    command = parse_command(" echo >| ");
    CINTA_ASSERT_NULL(command, info);

    command = parse_command(" echo >| /tmp/1 < ");
    CINTA_ASSERT_NULL(command, info);

    command = parse_command(" echo >| /tmp/1 < /tmp/2 2>");
    CINTA_ASSERT_NULL(command, info);

    command = parse_command(" echo | ");
    CINTA_ASSERT_NULL(command, info);

    command = parse_command(" echo > | ");
    CINTA_ASSERT_NULL(command, info);

    command = parse_command(" echo |  sdasd <");
    CINTA_ASSERT_NULL(command, info);

    command = parse_command(" echo | >");
    CINTA_ASSERT_NULL(command, info);

    command = parse_command(" echo <( dqwdqw ");
    CINTA_ASSERT_NULL(command, info);

    command = parse_command(" echo <( a | b");
    CINTA_ASSERT_NULL(command, info);

    command = parse_command(" echo | a <( | b");
    CINTA_ASSERT_NULL(command, info);

    command = parse_command(" echo | | | b");
    CINTA_ASSERT_NULL(command, info);
}

void test_pipelines_and_redirections(test_info *info) {

    command *command = parse_command("echo a | echo b > /tmp/1 | echo c");
    CINTA_ASSERT_NULL(command, info);

    command = parse_command("echo a | echo b < /tmp/1 | echo c");
    CINTA_ASSERT_NULL(command, info);

    command = parse_command("echo a | echo b > /tmp/1 | echo c > /tmp/2");
    CINTA_ASSERT_NULL(command, info);

    command = parse_command("echo a < /tmp/1 | echo b > /tmp/2 | echo c ");
    CINTA_ASSERT_NULL(command, info);

    command = parse_command("echo a | echo b | echo c < /tmp/1");
    CINTA_ASSERT_NULL(command, info);

    command = parse_command("echo a > /tmp/1 | echo b | echo c");
    CINTA_ASSERT_NULL(command, info);

    command = parse_command("echo a > /tmp/1 | echo b | echo c < /tmp/2");
    CINTA_ASSERT_NULL(command, info);

    command = parse_command("echo a > /tmp/1 | echo b > /tmp/2 | echo c < /tmp/3");
    CINTA_ASSERT_NULL(command, info);
}

void test_substitutions_and_redirections(test_info *info) {

    command *command = parse_command("cat <( echo b > /tmp/1 )");
    CINTA_ASSERT_NULL(command, info);

    command = parse_command("cat <( echo b > /tmp/1 ) > /tmp/2");
    CINTA_ASSERT_NULL(command, info);

    command = parse_command("cat <( echo b > /tmp/1 ) > /tmp/2 < /tmp/3");
    CINTA_ASSERT_NULL(command, info);

    command = parse_command("cat  <( echo y | echo > /tmp/1 )");
    CINTA_ASSERT_NULL(command, info);

    command = parse_command("cat  <( echo y | echo > /tmp/1 ) < /tmp/2");
    CINTA_ASSERT_NULL(command, info);

    command = parse_command("cat  <( echo y ) | cat <( echo y | echo > /tmp/1 ) < /tmp/2");
    CINTA_ASSERT_NULL(command, info);
}
