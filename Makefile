CC=gcc
CFLAGS=-Wall -Wextra
EXEC=jsh

SRCDIR=src
OBJDIR=obj

SRCFILES := $(shell find $(SRCDIR) -type f -name "*.c")
OBJFILES := $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCFILES))

# Create obj directory at the beginning
$(shell mkdir -p $(OBJDIR))

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) -c -o $@ $< $(CFLAGS)


.PHONY: all, clean, format

all: $(EXEC)

jsh: $(OBJFILES) 
	$(CC) -o $@ $^ $(CFLAGS)

format:
	clang-format -i $(SRCFILES)

clean:
	rm -rf $(OBJDIR) $(EXEC) test
