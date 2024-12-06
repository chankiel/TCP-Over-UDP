CXX = g++
CXXFLAGS = -std=c++17 -Wall -g

# Define source files and corresponding object files
SOURCES = $(wildcard */*.cpp) $(wildcard *.cpp)
OBJECTS = $(SOURCES:.cpp=.o)

# Define the output executable
EXEC = main

# Default target to build the project
all: $(EXEC)

# Rule to link object files and create the final executable
$(EXEC): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(EXEC) $(CXXFLAGS)

# Rule to compile source files into object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up object files and binary
clean:
	rm -f $(OBJECTS) $(EXEC)

# Rule to clean and rebuild everything
rebuild: clean all

# Run the main program with the specified host and port arguments
run: $(EXEC)
	./$(EXEC) $(host) $(port)

# Declare phony targets
.PHONY: all clean rebuild run
