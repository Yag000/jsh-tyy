#ifndef PROMPT_H
#define PROMPT_H

#include "command.h"
#include "internals.h"
#include "string_utils.h"
#include <readline/history.h>
#include <readline/readline.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/** The maximum size a prompt can have. */
extern const size_t LIMIT_PROMPT_SIZE;

void prompt();
char *get_prompt_string();

#endif // PROMPT_H
