CC_FLAGS = -O2 -Wall -Wpedantic
LDLIBS = -lcue

.PHONY: all clean install

all: cue2vorbis

clean:
	rm -rf *.o cue2vorbis

install: cue2vorbis
	install -m 0755 cue2vorbis /usr/local/bin

cue2vorbis: cue2vorbis.o
