CC=gcc
CFLAGS=-Wall -Wextra -lreadline
EXEC=jsh


TEST=test

SRCDIR=src
OBJDIR=obj
TMPDIR=tmp

SRCOBJDIR=$(OBJDIR)/$(SRCDIR)

TESTDIR=tests
TESTOBJDIR=$(OBJDIR)/$(TESTDIR)

SCRIPTSDIR=$(TESTDIR)/scripts

TEST_SCRIPT=$(SCRIPTSDIR)/test.sh
TEST_SETUP=$(SCRIPTSDIR)/test_setup.sh
TEST_VALGRIND=$(SCRIPTSDIR)/test_valgrind.sh
TEST_PROFESSOR=$(SCRIPTSDIR)/test_professor.sh
TEST_TYY=$(SCRIPTSDIR)/test_tyy.sh

CINTA=cinta
CINTAOBJDIR=$(OBJDIR)/cinta
CINTAOBJ=$(CINTAOBJDIR)/cinta.o

SRCFILES := $(shell find $(SRCDIR) -type f -name "*.c")
CINTAFILES := $(shell find $(CINTA) -type f -name "*.c")
TESTFILES := $(shell find $(TESTDIR) -type f -name "*.c") 

OBJFILES := $(patsubst $(SRCDIR)/%.c,$(SRCOBJDIR)/%.o,$(SRCFILES))
CINTAOBJFILES := $(patsubst $(CINTA)/%.c,$(CINTAOBJDIR)/%.o,$(CINTAFILES))
TESTOBJFILES := $(patsubst $(TESTDIR)/%.c,$(TESTOBJDIR)/%.o,$(TESTFILES)) 


ALLFILES := $(SRCFILES) $(TESTFILES) $(shell find $(SRCDIR) $(TESTDIR) -type f -name "*.h") 

# Create obj directory at the beginning
$(shell mkdir -p $(SRCOBJDIR))
$(shell mkdir -p $(TESTOBJDIR))
$(shell mkdir -p $(CINTAOBJDIR))

$(SRCOBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) -c -o $@ $< $(CFLAGS)

$(TESTOBJDIR)/%.o: $(TESTDIR)/%.c
	$(CC) -c -o $@ $< $(CFLAGS)

$(CINTAOBJDIR)/%.o: $(CINTA)/%.c
	$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: all, clean, format, test

all: $(EXEC)

$(EXEC): $(OBJFILES) 
	$(CC) -o $@ $^ $(CFLAGS)

test-quick: test-unit-quick test-valgrind-quick test-professor

test-unit-quick: compile_tests setup_test_env
	./$(TEST_SCRIPT) -q

test-valgrind-quick: compile_tests_valgrind setup_test_env
	./$(TEST_VALGRIND) -q

test-valgrind-quick-fds: compile_tests_valgrind setup_test_env
	./$(TEST_VALGRIND) -f -q

$(TEST): test-unit test-valgrind test-tyy test-professor

test-unit: compile_tests setup_test_env
	./$(TEST_SCRIPT)

test-valgrind: compile_tests_valgrind setup_test_env
	./$(TEST_VALGRIND)

test-valgrind-fds: compile_tests_valgrind setup_test_env
	./$(TEST_VALGRIND) -f

test-professor:
	./$(TEST_PROFESSOR)

test-tyy:
	./$(TEST_TYY) 

compile_tests_valgrind: $(filter-out $(SRCOBJDIR)/$(EXEC).o, $(OBJFILES)) $(TESTOBJFILES) $(CINTAOBJFILES)
	$(CC) -o $(TEST) $^ -g $(CFLAGS)

compile_tests: $(filter-out $(SRCOBJDIR)/$(EXEC).o, $(OBJFILES)) $(TESTOBJFILES) $(CINTAOBJFILES)
	$(CC) -o $(TEST) $^ $(CFLAGS)

setup_test_env:
	$(shell mkdir -p $(TMPDIR))
	./$(TEST_SETUP)

format fmt:
	clang-format -i $(ALLFILES)

check-format:
	clang-format --dry-run --Werror $(ALLFILES)

clean:
	rm -rf $(OBJDIR) $(EXEC) $(TESTOBJDIR) $(TEST) $(TMPDIR)
