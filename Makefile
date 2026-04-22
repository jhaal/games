CXX      = clang++
CXXFLAGS = -std=c++17 -O2 -Wall
SFML_INC = /opt/homebrew/include
SFML_LIB = /opt/homebrew/lib
LIBS     = -lsfml-graphics -lsfml-window -lsfml-audio -lsfml-system

TARGET = pacman

all: $(TARGET)

$(TARGET): main.cpp
	$(CXX) $(CXXFLAGS) -I$(SFML_INC) -L$(SFML_LIB) main.cpp -o $(TARGET) $(LIBS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
