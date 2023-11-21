CC=gcc
CFLAGS=-Wall -Wextra
EXEC=jsh
TEST=test

SRCDIR=src
OBJDIR=obj
TMPDIR=tmp

SRCOBJDIR=$(OBJDIR)/$(SRCDIR)

TESTDIR=tests
TESTOBJDIR=$(OBJDIR)/$(TESTDIR)

SRCFILES := $(shell find $(SRCDIR) -type f -name "*.c")
TESTFILES := $(shell find $(TESTDIR) -type f -name "*.c")

OBJFILES := $(patsubst $(SRCDIR)/%.c,$(SRCOBJDIR)/%.o,$(SRCFILES))
TESTOBJFILES := $(patsubst $(TESTDIR)/%.c,$(TESTOBJDIR)/%.o,$(TESTFILES))

ALLFILES := $(SRCFILES) $(TESTFILES) $(shell find $(SRCDIR) $(TESTDIR) -type f -name "*.h") 

# Create obj directory at the beginning
$(shell mkdir -p $(SRCOBJDIR))
$(shell mkdir -p $(TESTOBJDIR))
$(shell mkdir -p $(TMPDIR))

$(SRCOBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) -c -o $@ $< $(CFLAGS)

$(TESTOBJDIR)/%.o: $(TESTDIR)/%.c
	$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: all, clean, format, test

all: $(EXEC)

$(EXEC): $(OBJFILES) 
	$(CC) -o $@ $^ $(CFLAGS)

$(TEST): compile_tests
	./$(TEST)

test-valgrind: compile_tests
	valgrind --leak-check=full --show-leak-kinds=all --errors-for-leak-kinds=all --error-exitcode=1 ./$(TEST)

compile_tests: $(filter-out $(SRCOBJDIR)/$(EXEC).o, $(OBJFILES)) $(TESTOBJFILES)
	$(CC) -o $(TEST) $^ $(CFLAGS)

format fmt:
	clang-format -i $(ALLFILES)

check-format:
	clang-format --dry-run --Werror $(ALLFILES)

clean:
	rm -rf $(OBJDIR) $(EXEC) $(TESTOBJDIR) $(TEST) $(TMPDIR)
