# Makefile

# Compiler
CC = clang++

# Compiler flags
CFLAGS = -std=c++20 -Wall -Wextra -Wno-missing-braces -g

# Linker flags
LDFLAGS = -lraylib 

# Target
TARGET = pang.o
FILES = main.cpp
COMPS = main.o

all: $(TARGET)

$(TARGET): main.cpp
	$(CC) $(CFLAGS) -c $(FILES) $(LDFLAGS)
	$(CC) $(CFLAGS) -o $(TARGET) $(COMPS) $(LDFLAGS)

run:
	./$(TARGET)

clean:
	rm -f $(COMPS) $(TARGET)

