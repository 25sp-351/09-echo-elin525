CC = gcc
CFLAGS = -Wall -Wextra -pthread

SRC = echoserver.c
TARGET = echoserver

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)