LIBS = lib/socket.a lib/libfdr.a
INCLUDES = include/middleware.h
OBJS = obj/middleware.o obj/http.o obj/html.o obj/config.o
BIN = bin/webserver

CFLAGS = -O3 -Iinclude

all: $(BIN) $(LIBS)
.PHONY: all

debug: CFLAGS += -g
debug: clean all

clean:
	rm -f obj/* bin/* .core* lib/*
.PHONY: clean

lib/libfdr.a: obj/dllist.o obj/jval.o obj/jrb.o obj/fields.o
	ar ru $@ $^
	ranlib $@

lib/socket.a: obj/socket.o
	ar ru $@ $<
	ranlib $@

bin/%: src/%.c $(LIBS) $(OBJS)
	gcc $(CFLAGS) -o $@ $< $(OBJS) $(LIBS)

obj/%.o: src/%.c
	gcc $(CFLAGS) -c -o $@ $<


obj/%.o: src/deps/%.c
	gcc $(CFLAGS) -c -o $@ $<
