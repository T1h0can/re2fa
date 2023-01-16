PLATFORM = $(shell uname -s)
CC:= gcc
CFLAGS+= -std=c99 -Wall -Werror -pedantic -O3
CFLAGS+= -include stddef.h -include stdint.h -include inttypes.h
CFLAGS+= -Wl,-rpath,'$$ORIGIN/lib'
ifeq ($(PLATFORM), Linux)
	CFLAGS+= -Wl,-allow-shlib-undefined
endif

SUBDIR:= lib

#CFLAGS+= $(SUBDIR:%=-L%)
LIBS:= $(SUBDIR:%=-L%)
LIBS+= -lrefa
LIBS+= -pthread
ifeq ($(PLATFORM), Darwin)
	LIBS+= -largp
endif

VER:= version_info.h

BIN:= re2fa

all: $(SUBDIR) $(BIN)

re2fa: main.c $(VER)
	#$(CC) $(CFLAGS) $< -o $@ $(LIBS)
	$(CC) $(CFLAGS) $(LIBS) $< -o $@

$(VER): .FORCE
	echo 'const char *git_version = "dev";' >> version_info.h
#	echo -n 'const char *git_version = "'	>>  version_info.h
#	echo -n `git describe`			>> version_info.h
#	echo '";'				>> version_info.h

.FORCE:

$(SUBDIR):
	$(MAKE) -C $@

.PHONY: $(SUBDIR)

clean:
	@rm -vf *.o *.dylib $(BIN) $(VER); \
	for dir in $(SUBDIR); do \
	  $(MAKE) -C $$dir -f Makefile $@; \
	done
