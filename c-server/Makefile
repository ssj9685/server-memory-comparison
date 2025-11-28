CC ?= cc
CFLAGS ?= -O2 -Wall -Wextra -Wpedantic
TARGET := http_server
SRC := server.c net.c http.c signals.c

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $(SRC)

clean:
	rm -f $(TARGET)
