CC = gcc

TARGETS = server client
CFILES = server.c client.c
OBJS = $(CFILES:.c=.o)

%.o: %.c
	$(CC) -c -o $@ $<

.PHONY: all
all: $(TARGETS)

$(TARGETS): %: %.c
	$(CC) -o $@ $<

clean:
	rm -rf $(TARGETS)
