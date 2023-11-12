CC=gcc
CFLAGS=-Wall -Wextra
EXEC=jsh

SRCDIR=src
OBJDIR=obj

SRCFILES := $(shell find $(SRCDIR) -type f -name "*.c")
ALLFILES := $(SRCFILES) $(shell find $(SRCDIR) -type f -name "*.h")
OBJFILES := $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCFILES))

# Create obj directory at the beginning
$(shell mkdir -p $(OBJDIR))

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) -c -o $@ $< $(CFLAGS)


.PHONY: all, clean, format, test

all: $(EXEC)

test: 

jsh: $(OBJFILES) 
	$(CC) -o $@ $^ $(CFLAGS)

format:
	clang-format -i $(ALLFILES)

check-format:
	clang-format --dry-run --Werror $(ALLFILES)

clean:
	rm -rf $(OBJDIR) $(EXEC) test
