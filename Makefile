# Compiler
CXX := c++
CXXFLAGS := -Wextra -std=c++17 -I lib -I utils -I app

# Source and Object files
SRC := app/main.cpp
OBJ := $(SRC:.cpp=.o)

# Executable name
TARGET := main

# Phony targets
.PHONY: all clean

# Default target
all: $(TARGET)

# Linking
$(TARGET): $(OBJ)
	$(CXX) -o $@ $^

# Compiling source files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up build artifacts
clean:
	rm -f $(OBJ) $(TARGET)
