CC := cc
EXEC := generate
SRC := \
    main.c       \
    generators.c \

OBJS := $(SRC:.c=.o)

OPT := -Og -g
CFLAGS := \
    $(OPT)       \
    -Wall        \
    -Wconversion \
    -Wextra      \
    -flto        \
    -pedantic    \
    -static      \
    -std=c18     \

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(EXEC)

clean:
	$(RM) $(OBJS) $(EXEC)

.PHONY: all clean
