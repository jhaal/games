CXX      = clang++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -pedantic

TARGET = tanks

all: $(TARGET)

$(TARGET): main.cpp
	$(CXX) $(CXXFLAGS) main.cpp -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

test: $(TARGET)
	./$(TARGET) --self-test

clean:
	rm -f $(TARGET) pacman
