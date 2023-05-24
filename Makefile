CC = gcc

# Flags common between library and executable
CFLAGS  = -g -Iinclude -Wall -Werror -Wextra -Wno-parentheses -pedantic -fpic
LDFLAGS = -Llib -Wl,-rpath=$$PWD/lib -luaic

LIB = lib/libuaic.so
OBJS = \
	   src/uai_data.o \

BINS = \
	   examples/dataframe/load_csv.out \

all: $(LIB) $(BINS)

$(BINS): $(LIB)

lib:
	mkdir -p lib

$(LIB): lib $(OBJS)
	$(CC) -shared -o $(LIB) $(OBJS)

clean:
	rm -f $(OBJS) $(LIB) $(BINS)
