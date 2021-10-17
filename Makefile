
OPTIMIZATION?=-O0
STD=-std=c99
WARN=-Wall -W -Wno-missing-field-initializers
OPT=$(OPTIMIZATION)

INC_DIRS = ./
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

CFLAGS=$(STD) $(WARN) $(OPT) $(DEBUG) $(INC_FLAGS)
LDFLAGS=$(DEBUG) -rdynamic
LDLIBS=-lyaml -lm
DEBUG=-g -ggdb
CC=gcc

TARGET ?= simple_yaml
SRC := $(wildcard *.c)
OBJS := $(SRC:.c=.o)

default: $(TARGET)

.c.o:
	$(CC) $(CFLAGS) $(INC_FLAGS) -c $<

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS) $(LDLIBS)

.PHONY: clean
clean:
	$(RM) $(TARGET) $(OBJS)
