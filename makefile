.DEFAULT_GOAL := run

CC?=clang

CFLAGS=-g -I. -I./deps/arena -I./deps/utf8

SOURCES = $(wildcard *.c)
SOURCES += $(foreach X,$(shell ls deps), $(wildcard ./deps/$(X)/*.c))
OBJECTS=$(SOURCES:%.c=%.o)

target?=

debug?=-g

CFLAGS+= $(debug)
CFLAGS+= -Wall -Wpedantic -Wextra -Wpedantic -std=c17
CFLAGS+= -I./
CFLAGS+= $(foreach X,$(shell ls deps), -I./deps/$(X))
CFLAGS+= -fPIE


%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(OBJECTS) tests

tests: ./t/tests.c $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $< $(OBJECTS)

run: clean $(OBJECTS) tests
	./tests
