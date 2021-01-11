CC = gcc

TARGETS = client server
CFILES = client.c server.c message.c
OBJS = $(CFILES:.c=.o)
DEPS = message.h

.PHONY: all
all: $(TARGETS)

$(TARGETS): %: %.c message.h
	$(CC) -o $@ $< message.c

.PHONY: clean
clean:
	rm -rf $(TARGETS)
