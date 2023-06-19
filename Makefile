CC = gcc

# Flags common between library and executable
CFLAGS  = -g -Iinclude -Wall -Werror -Wextra -Wno-parentheses -pedantic -fpic
LDFLAGS = -Llib -Wl,-rpath=$$PWD/lib -luaic -lm

LIB = lib/libuaic.so
OBJS = \
	   src/uai_data.o \
	   src/uai_logistic_regression.o \

BINS = \
	   examples/dataframe/load_csv.out \
	   examples/dataframe/convert.out \
	   examples/dataframe/export_csv.out \
	   examples/dataframe/split.out \
	   examples/dataframe/fill.out \
	   examples/dataframe/resize.out \
	   examples/dataframe/scale_data.out \
	   examples/logistic_regression/logistic_regression.out \

all: $(LIB) $(BINS)

lib:
	mkdir -p lib

$(LIB): lib $(OBJS)
	$(CC) -shared -o $(LIB) $(OBJS)

.c.out:
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f $(OBJS) $(LIB) $(BINS)
