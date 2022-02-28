# Makefile for Writing Make Files Example

# *****************************************************
# Variables to control Makefile operation

CC = g++
CFLAGS = -Wall -pedantic -std=c++17 -Wshadow -Wno-long-long -Werror
DBFLAGS = $(CFLAGS) -fsanitize=address -g

# The build target
TARGET = main

# ****************************************************
# Targets needed to bring the executable up to date

main: main.o
	$(CC) $(DBFLAGS) -o $(TARGET) $(TARGET).o

main.o: main.cpp
	$(CC) $(DBFLAGS) -c $(TARGET).cpp

all: $(TARGET)

clean:
	$(RM) $(TARGET)
	$(RM) $(TARGET).o

