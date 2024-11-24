# Compiler and Flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall `sdl2-config --cflags`
LDFLAGS = `sdl2-config --libs`

# Source Files
SRC = $(wildcard *.cpp)
OBJ = $(SRC:.cpp=.o)

# Target Executable
TARGET = chip8_emu

# Build Rules
all: $(TARGET)

$(TARGET): $(OBJ)
	@$(CXX) $(OBJ) -o $(TARGET) $(LDFLAGS)
	@echo "Build complete. Running $(TARGET)..."
	@./$(TARGET) "./roms/test_opcode.ch8"

%.o: %.cpp
	@$(CXX) $(CXXFLAGS) -c $< -o $@

run:$(TARGET)
	./chip8_emu "./roms/test_opcode.ch8"

# Clean Up
clean:
	@rm -f $(OBJ) $(TARGET)
	@echo "Cleaned up build files."

debug:CXXFLAGS += -DDEBUG

debug:$(OBJ)
	@$(CXX) $(OBJ) -o $(TARGET) $(LDFLAGS)
	@echo "Build complete. Running $(TARGET)..."
	@./$(TARGET) "./roms/test_opcode.ch8"



	
