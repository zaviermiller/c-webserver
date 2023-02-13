LIBS = lib/socket.a lib/libfdr.a
INCLUDES = include/middleware.h
OBJS = obj/middleware.o obj/http.o obj/html.o obj/config.o obj/util.o obj/status.o obj/middlewares.o
BIN = bin/webserver bin/single_threaded_webserver
TEST_BIN = test/util_test test/http_test

CFLAGS = -O3 -Iinclude

all: $(BIN) $(LIBS)
.PHONY: all

test: FORCE $(TEST_BIN)

FORCE: ;

debug: CFLAGS += -g
debug: clean all

clean:
	rm -f obj/* bin/* .core* lib/* $(TEST_BIN)
.PHONY: clean

lib/libfdr.a: obj/dllist.o obj/jval.o obj/jrb.o obj/fields.o
	ar ru $@ $^
	ranlib $@

lib/socket.a: obj/socket.o
	ar ru $@ $<
	ranlib $@

test/%: test/%.c $(LIBS) $(OBJS)
	clang $(CFLAGS) -o $@ $< $(OBJS) $(LIBS)

bin/%: src/%.c $(LIBS) $(OBJS)
	clang $(CFLAGS) -o $@ $< $(OBJS) $(LIBS)

obj/%.o: src/%.c
	clang $(CFLAGS) -c -o $@ $<

obj/%.o: src/deps/%.c
	clang $(CFLAGS) -c -o $@ $<
