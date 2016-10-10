CC = clang
CFLAGS = -g -Wall

TARGET = clones

all: $(TARGET)

$(TARGET): main.c
	$(CC) $(CFLAGS) -o $(TARGET) main.c
