# Makefile for SP ISS simulator
# Works with GCC (MinGW, MSYS2, WSL, or Linux)

CC      = gcc
CFLAGS  = -Wall -Wextra -O2 -std=c99
TARGET  = iss

all: $(TARGET)

$(TARGET): iss.c
	$(CC) $(CFLAGS) -o $(TARGET) iss.c

clean:
	del $(TARGET).exe 2>nul || rm -f $(TARGET)