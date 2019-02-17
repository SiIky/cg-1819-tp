CXX := g++
EXEC := generate
SRC := \
    main.cpp       \
    generators.cpp \

OBJS := $(SRC:.cpp=.o)

OPT := -Og -g
CXXFLAGS := \
    $(OPT)       \
    -Wall        \
    -Wconversion \
    -Wextra      \
    -flto        \
    -pedantic    \
    -static      \

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(EXEC)

clean:
	$(RM) $(OBJS) $(EXEC)

.PHONY: all clean
