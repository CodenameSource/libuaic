CC = gcc

# Flags common between library and executable
CFLAGS  = -g -Iinclude -Wall -Werror -Wextra -pedantic -fpic
LDFLAGS =

LIB = lib/libuaic.so
OBJS = \
	   src/uai_data.o \

all: lib/libuaic.so

lib:
	mkdir -p lib

$(LIB): lib $(OBJS)
	$(CC) -shared -o $(LIB) $(OBJS)

clean:
	rm -f $(OBJS) $(LIB)
