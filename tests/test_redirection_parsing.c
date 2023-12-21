#include "../src/command.h"
#include "../src/internals.h"
#include "test_core.h"

#include <fcntl.h>
#include <strings.h>
#include <unistd.h>

void test_parse_multiple_same_redirections(test_info *info);

test_info *test_redirection_parsing() {

    // Test setup
    print_test_header("parse redirections");
    clock_t start = clock();
    test_info *info = create_test_info();

    // Add test here
    test_parse_multiple_same_redirections(info);

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

    command_call *command = parse_command("echo abcdefghijkl > tmp/output0 >> tmp/output1 >| tmp/output2");

    command_result *result = execute_command_call(command);

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
}
