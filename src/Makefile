PROJ=ifj2023
TEAM=xkloub03

CC=gcc
CFLAGS=-Wall -Wextra -O2 -MMD -Werror -Wpedantic -g

ZIP_FILE=$(TEAM).zip

SRCS=$(wildcard *.c)
OBJS=$(patsubst %.c,%.o,$(SRCS))
DEPS=$(patsubst %.c,%.d,$(SRCS))

$(PROJ): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(PROJ) $^

build: $(PROJ)

# doc: documentation.typ
# 	typst c $^

pack:
	zip -j $(ZIP_FILE) *.c *.h Makefile ../rozhodnuti.txt

-include $(DEPS)

.PHONY: clean
clean:
	rm $(PROJ) $(OBJS) $(DEPS)
