CC = gcc

CFLAGS = -g -I../include -Wall -Werror -Wextra -pedantic
LDFLAGS = -L../lib -Wl,-rpath=$$PWD/../lib -luaic

BINS = \
	   dataframe/load_csv \

all: $(BINS)

$(BINS): ../lib/libuaic.so

../lib/libuaic.so:
	cd .. && $(MAKE) lib/libuaic.so

clean:
	rm -f $(BINS)
