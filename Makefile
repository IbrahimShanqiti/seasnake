CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic $(shell sdl2-config --cflags)
LDFLAGS = $(shell sdl2-config --libs)

TARGET = snake
SRC = snake.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

clean:
	rm -f $(TARGET)

.PHONY: all clean

