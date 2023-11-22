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

void prompt();

#endif // PROMPT_H
