#include "prompt.h"
#include "command.h"
#include "internals.h"
#include "utils.h"
#include <readline/readline.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

char *get_prompt_string() {
    char *prompt_string;
    char *total_jobs = "[0]";
    char *jobs = in_yellow(total_jobs);
    char *pwd = get_current_wd();
    char *path = in_blue(pwd);
    char *trunc_sign = "...";
    char *dots = in_blue("...");
    char *dollar_sign = last_exit_code ? in_red("$ ") : in_green("$ ");
    size_t max_size = LIMIT_PROMPT_SIZE - strlen(total_jobs) - strlen(trunc_sign) - strlen("$ ");
    int truncated = strlen(pwd) > max_size;

    if (jobs == NULL || pwd == NULL || dollar_sign == NULL) {
        perror("malloc(): prompt");
        return NULL;
    }

    prompt_string = malloc(3 * LIMIT_PROMPT_SIZE * sizeof(char));
    if (prompt_string == NULL) {
        perror("malloc(): prompt");
        free(jobs);
        free(pwd);
        free(path);
        return dollar_sign;
    }

    if (truncated) {
        if (dots == NULL || path == NULL) {
            perror("malloc(): prompt");
            return NULL;
        }

        char *trunc_pwd = truncated_cwd(max_size);

        if (trunc_pwd == NULL) {
            perror("malloc(): prompt");
            return NULL;
        }

        char *blue_pwd = in_blue(trunc_pwd);
        if (blue_pwd == NULL) {
            perror("malloc(): prompt");
            return NULL;
        }

        sprintf(prompt_string, "%s%s%s%s", jobs, dots, blue_pwd, dollar_sign);
        free(blue_pwd);
        free(trunc_pwd);

    } else {
        sprintf(prompt_string, "%s%s%s", jobs, path, dollar_sign);
    }

    free(jobs);
    free(dots);
    free(pwd);
    free(path);
    free(dollar_sign);

    return prompt_string;
}

void prompt() {
    rl_outstream = stderr;

    while (!should_exit) {
        char *prompt_string = get_prompt_string();
        char *buf = readline(prompt_string);
        if (buf == NULL) {
            return;
        }
        add_history(buf);

        free(prompt_string);

        command_call *command = parse_command(buf);
        command_result *command_result;
        if (command != NULL) {
            command_result = execute_command_call(command);

            if (command_result != NULL) {
                destroy_command_result(command_result);
            } else {
                destroy_command_call(command);
            }
        }
        free(buf);
    }
}
