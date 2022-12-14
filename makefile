LIBS = lib/socket.a lib/libfdr.a
INCLUDES = include/middleware.h
OBJS = obj/middleware.o obj/http.o obj/html.o
BIN = bin/webserver

CFLAGS = -O3 -Iinclude

all: $(BIN)
.PHONY: all

debug: CFLAGS += -g
debug: clean all

clean:
	rm -f obj/* bin/* .core*
.PHONY: clean

bin/%: src/%.c $(LIBS) $(OBJS)
	gcc $(CFLAGS) -o $@ $< $(OBJS) $(LIBS)

obj/%.o: src/%.c
	gcc $(CFLAGS) -c -o $@ $<
