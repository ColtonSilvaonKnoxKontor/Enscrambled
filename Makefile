CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2 -g
LDFLAGS := -lcrypto -lcurl -lpthread -lSDL2

# Client sources
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

# Server sources
SERVER_SOURCES := server/main.cpp
SERVER_OBJECTS := $(SERVER_SOURCES:.cpp=.o)
SERVER_TARGET := enscrambled_server

all: $(TARGET) $(SERVER_TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJECTS) $(LDFLAGS)

$(SERVER_TARGET): $(SERVER_OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $(SERVER_OBJECTS) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(SERVER_TARGET) $(OBJECTS) $(SERVER_OBJECTS)

.PHONY: all clean

