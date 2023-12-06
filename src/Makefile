PROJ=ifj2023
TEAM=xkloub03

CC=gcc
DEFINES=-DPRINT_INT_ERR
CFLAGS=-Wall -Wextra -O2 -MMD -Werror -Wpedantic -g $(DEFINES)

ZIP_FILE=$(TEAM).zip
TEST_DIR=test

SRCS=$(wildcard *.c)
TEST=$(wildcard $(TEST_DIR)/*.c)
OBJS=$(patsubst %.c,%.o,$(SRCS))
DEPS=$(patsubst %.c,%.d,$(SRCS))
TEST_EXECUTABLES = $(patsubst $(TEST_DIR)/%.c, %, $(TEST))
TEST_OBJS = $(subst main.o,, $(OBJS))

$(PROJ): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(PROJ) $^

build: $(PROJ)

# Compile each test file to its executable
$(TEST_EXECUTABLES): %: $(TEST_OBJS) $(TEST_DIR)/%.c
	$(CC) $(CFLAGS) $(TEST_OBJS) $(TEST_DIR)/$*.c -o $@

# Run all tests
test: $(TEST_EXECUTABLES)
	@for test_exec in $(TEST_EXECUTABLES); do \
		echo; \
		echo Testing $$test_exec; \
		./$$test_exec; \
	done

# doc: documentation.typ
# 	typst c $^

doc:
	typst c ../doc/documentation.typ ./dokumentace.pdf

pack: doc
	zip -j $(ZIP_FILE) *.c *.h Makefile ../rozdeleni ../rozsireni dokumentace.pdf

-include $(DEPS)

.PHONY: clean
clean:
	rm $(PROJ) $(OBJS) $(DEPS) $(TEST_EXECUTABLES)
