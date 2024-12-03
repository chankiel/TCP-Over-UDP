# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -pthread
INCLUDES = -I./template-jarkom-main

# Source files
SRC = main.cpp client.cpp server.cpp segment.cpp socket.cpp

# Output binary
TARGET = node

# Default target: build the program
all: $(TARGET)

# Link object files to create the final executable
$(TARGET): $(SRC)
	$(CXX) $(SRC) -o $(TARGET) $(CXXFLAGS) $(INCLUDES)

# Clean up object files and binary
clean:
	rm -f $(TARGET)

# Run the program with arguments
run: $(TARGET)
	./$(TARGET) $(ARGS)

# Phony targets
.PHONY: all clean run