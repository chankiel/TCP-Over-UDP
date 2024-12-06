CXX = g++
CXXFLAGS = -std=c++17 -Wall

# Define the source files and the object files
SOURCES = $(wildcard */*.cpp) $(wildcard *.cpp)
OBJECTS = $(SOURCES:.cpp=.o)

# Define the output executable
EXEC = main

# Default target to build the project
all: $(EXEC)

# Link object files to create the final executable
$(TARGET): $(SRC)
	$(CXX) $(SRC) -o $(TARGET) $(CXXFLAGS) $(INCLUDES)

# Clean up object files and binary
clean:
	rm -f $(TARGET)

# Rule to clean and rebuild everything
rebuild: clean all

# Run the main program with the specified host and port arguments
run: $(EXEC)
	./$(EXEC) node $(host) $(port)

# Declare phony targets
.PHONY: all clean rebuild run