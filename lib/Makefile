PLATFORM:= $(shell uname -s)
CC:=gcc
CFLAGS+= -std=c99 -Wall -Werror -pedantic -O3
CFLAGS+= -include stddef.h -include stdint.h -include inttypes.h
CFLAGS+= -D_USE_ZLIB

SRC-refa:= \
	parser.c \
	parser_inner.c \
	nfa.c \
	dfa.c \
	tree_to_nfa.c \
	nfa_to_dfa.c \
	dfa_to_nfa.c \
	dfa_to_d2fa.c \
	d2fa.c \
	simple_list.c\

SRC-shared:= $(SRC-refa)
SRC:= $(SRC-shared)

TARGET:= librefa.so
ifeq ($(PLATFORM), Darwin)
	TARGET= librefa.dylib
endif

all: $(TARGET)

-include $(SRC:.c=.d)

librefa.so: $(SRC-refa:.c=.o)
	$(CC) -shared $^ -Wl,-rpath,'$$ORIGIN' -o $@ -lz

librefa.dylib: $(SRC-refa:.c=.o)
	$(CC) -dynamiclib $^ -Wl,-rpath,'$$ORIGIN' -o $@ -lz
	cp $@ ../

$(SRC-shared:.c=.o):
	$(CC) $(CFLAGS) -fPIC -c -o $@ $<

%.d: %.c
	@set -e; rm -f $@; \
	$(CC) -M $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

clean:
	@rm -f *.o *.a *.d *.so *.dylib
