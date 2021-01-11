CC = gcc
CFILES = client.c
TARGET = client

$(TARGET): $(CFILES)
	$(CC) $< -o $@

.PHONY: clean
clean:
	rm -rf $(TARGET)

.PHONY: run
run:
	./$(TARGET)
