CC=g++
CPPFLAGS=-std=c++17 -g
BINNAME=lltable

build: $(BINNAME)

run: $(BINNAME)
	./$(BINNAME)

$(BINNAME): main.o grammar.o llgen.o
	$(CC) $(CPPFLAGS) -o $(BINNAME) $^

main.o: grammar.h llgen.h

grammar.o: grammar.cpp grammar.h

llgen.o: llgen.cpp llgen.h

.PHONY: clean

clean:
	rm -rf $(BINNAME) main.o grammar.o llgen.o vgcore*

