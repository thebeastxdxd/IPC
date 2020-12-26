CC = gcc

TARGETS = server client
CFILES = server.c client.c message.c
OBJS = $(CFILES:.c=.o)
DEPS = message.h

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $<

.PHONY: all
all: $(TARGETS)

$(TARGETS): %: %.c 
	$(CC) -o $@ $< message.c

clean:
	rm -rf $(TARGETS)
