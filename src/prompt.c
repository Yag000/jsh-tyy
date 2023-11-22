#include "prompt.h"
#include "command.h"
#include "string_utils.h"

void prompt() {
    while (1) {
        char *buf = readline("\001\033[0;32m\002$ \001\033[0m\002");
        add_history(buf);

        command_call *command = parse_command(buf);
        command_result *command_result;
        if (command != NULL) {
            command_result = execute_command(command);

            if (command_result != NULL) {
                destroy_command_result(command_result);
            } else {
                destroy_command_call(command);
            }
        }
        free(buf);
    }
}
