#include "../src/command.h"
#include "../src/internals.h"
#include "../src/jobs.h"
#include "test_core.h"

#include <fcntl.h>
#include <strings.h>
#include <unistd.h>

void test_parse_multiple_same_redirections(test_info *info);
void test_parse_errors(test_info *info);
void test_pipelines_and_redirections(test_info *info);
void test_substitutions_and_redirections(test_info *info);

test_info *test_redirection_parsing() {

    // Test setup
    print_test_header("parse redirections");
    clock_t start = clock();
    test_info *info = create_test_info();

    // Add test here
    test_parse_multiple_same_redirections(info);
    test_parse_errors(info);
    test_pipelines_and_redirections(info);
    test_substitutions_and_redirections(info);

    // End of tests
    info->time = clock_ticks_to_seconds(clock() - start);
    print_test_footer("parse redirections", info);
    return info;
}

int assert_file_is_empty(const int fd) {
    char buf[10];
    int nb_read = read(fd, buf, 10);

    return nb_read == 0;
}

void test_parse_multiple_same_redirections(test_info *info) {

    if (!allow_slow) {
        return;
    }

    print_test_name("Parse redirections | Sequence of same redirections");

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
    handle_boolean_test(1, fd_output0 >= 0, __LINE__, __FILE__, info);

    int fd_output1 = open("tmp/output1", O_RDONLY);
    handle_boolean_test(1, fd_output1 >= 0, __LINE__, __FILE__, info);

    int fd_output2 = open("tmp/output2", O_RDONLY);
    handle_boolean_test(1, fd_output2 >= 0, __LINE__, __FILE__, info);

    handle_boolean_test(1, assert_file_is_empty(fd_output0), __LINE__, __FILE__, info);
    handle_boolean_test(1, assert_file_is_empty(fd_output1), __LINE__, __FILE__, info);

    char *buf = "abcdefghijkl";
    char buffer[strlen(buf) + 1];
    read(fd_output2, buffer, 255);
    buffer[strlen(buf)] = '\0';
    handle_string_test(buffer, buf, __LINE__, __FILE__, info);

    close(fd_output0);
    close(fd_output1);
    close(fd_output2);

    destroy_command_result(result);

    init_job_table();
}

void test_parse_errors(test_info *info) {
    print_test_name("Parse redirections | Errors");

    command *command = parse_command(" > ");
    handle_null_test(command, __LINE__, __FILE__, info);

    command = parse_command(" < /tmp/1");
    handle_null_test(command, __LINE__, __FILE__, info);

    command = parse_command(" > /tmp/1");
    handle_null_test(command, __LINE__, __FILE__, info);

    command = parse_command(" echo < ");
    handle_null_test(command, __LINE__, __FILE__, info);

    command = parse_command("| echo");
    handle_null_test(command, __LINE__, __FILE__, info);

    command = parse_command("<( echo )");
    handle_null_test(command, __LINE__, __FILE__, info);

    command = parse_command(" echo >| ");
    handle_null_test(command, __LINE__, __FILE__, info);

    command = parse_command(" echo >| /tmp/1 < ");
    handle_null_test(command, __LINE__, __FILE__, info);

    command = parse_command(" echo >| /tmp/1 < /tmp/2 2>");
    handle_null_test(command, __LINE__, __FILE__, info);

    command = parse_command(" echo | ");
    handle_null_test(command, __LINE__, __FILE__, info);

    command = parse_command(" echo > | ");
    handle_null_test(command, __LINE__, __FILE__, info);

    command = parse_command(" echo |  sdasd <");
    handle_null_test(command, __LINE__, __FILE__, info);

    command = parse_command(" echo | >");
    handle_null_test(command, __LINE__, __FILE__, info);

    command = parse_command(" echo <( dqwdqw ");
    handle_null_test(command, __LINE__, __FILE__, info);

    command = parse_command(" echo <( a | b");
    handle_null_test(command, __LINE__, __FILE__, info);

    command = parse_command(" echo | a <( | b");
    handle_null_test(command, __LINE__, __FILE__, info);

    command = parse_command(" echo | | | b");
    handle_null_test(command, __LINE__, __FILE__, info);
}

void test_pipelines_and_redirections(test_info *info) {
    print_test_name("Parse redirections | Pipelines and redirections");

    command *command = parse_command("echo a | echo b > /tmp/1 | echo c");
    handle_null_test(command, __LINE__, __FILE__, info);

    command = parse_command("echo a | echo b < /tmp/1 | echo c");
    handle_null_test(command, __LINE__, __FILE__, info);

    command = parse_command("echo a | echo b > /tmp/1 | echo c > /tmp/2");
    handle_null_test(command, __LINE__, __FILE__, info);

    command = parse_command("echo a < /tmp/1 | echo b > /tmp/2 | echo c ");
    handle_null_test(command, __LINE__, __FILE__, info);

    command = parse_command("echo a | echo b | echo c < /tmp/1");
    handle_null_test(command, __LINE__, __FILE__, info);

    command = parse_command("echo a > /tmp/1 | echo b | echo c");
    handle_null_test(command, __LINE__, __FILE__, info);

    command = parse_command("echo a > /tmp/1 | echo b | echo c < /tmp/2");
    handle_null_test(command, __LINE__, __FILE__, info);

    command = parse_command("echo a > /tmp/1 | echo b > /tmp/2 | echo c < /tmp/3");
    handle_null_test(command, __LINE__, __FILE__, info);
}

void test_substitutions_and_redirections(test_info *info) {
    print_test_name("Parse redirections | Substitutions and redirections");

    command *command = parse_command("cat <( echo b > /tmp/1 )");
    handle_null_test(command, __LINE__, __FILE__, info);

    command = parse_command("cat <( echo b > /tmp/1 ) > /tmp/2");
    handle_null_test(command, __LINE__, __FILE__, info);

    command = parse_command("cat <( echo b > /tmp/1 ) > /tmp/2 < /tmp/3");
    handle_null_test(command, __LINE__, __FILE__, info);

    command = parse_command("cat  <( echo y | echo > /tmp/1 )");
    handle_null_test(command, __LINE__, __FILE__, info);

    command = parse_command("cat  <( echo y | echo > /tmp/1 ) < /tmp/2");
    handle_null_test(command, __LINE__, __FILE__, info);

    command = parse_command("cat  <( echo y ) | cat <( echo y | echo > /tmp/1 ) < /tmp/2");
    handle_null_test(command, __LINE__, __FILE__, info);
}
