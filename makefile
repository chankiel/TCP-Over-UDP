# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall

# Define the source files and the object files
SOURCES = $(wildcard *.cpp)
OBJECTS = $(SOURCES:.cpp=.o)

# Define the output executable
EXEC = main

# Default target to build the project
all: $(EXEC)

# Rule to compile the source files into object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Rule to link the object files into the final executable
$(EXEC): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(EXEC) $(OBJECTS)

# Rule to clean up generated files
clean:
	rm -f $(OBJECTS) $(EXEC)

# Run the main program with the specified host and port arguments
run: $(EXEC)
	./$(EXEC) node $(host) $(port)

