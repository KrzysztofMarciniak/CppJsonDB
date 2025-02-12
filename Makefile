TARGET = dbms
CXX = g++
CXXFLAGS = -std=c++2b -Wall -lreadline -lcrypto -g -lfmt -O3
SRC_DIR = src
ROOT_DIR = $(SRC_DIR)

SRCS := $(shell find $(SRC_DIR) -name '*.cpp')
OBJS := $(SRCS:.cpp=.o)
DEPS := $(OBJS:.o=.d)

INCLUDES = -I$(ROOT_DIR)

DEPFLAGS = -MMD -MP

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET) -lreadline

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) $(INCLUDES) -c $< -o $@

-include $(DEPS)

clean:
	rm -f $(OBJS) $(DEPS) $(TARGET)

rebuild: clean $(TARGET)

all: $(TARGET)

.DEFAULT_GOAL := all
