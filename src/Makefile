PROJ=ifj2023
TEAM=xkloub03

CC=gcc
CFLAGS=-Wall -Wextra -O2 -MMD -Werror -Wpedantic -g

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
		echo Testing $$test_exec; \
		./$$test_exec; \
		echo; \
	done

# doc: documentation.typ
# 	typst c $^

pack:
	zip -j $(ZIP_FILE) *.c *.h Makefile ../rozhodnuti.txt

-include $(DEPS)

.PHONY: clean
clean:
	rm $(PROJ) $(OBJS) $(DEPS)
