.PHONY: all clean

all: as

clean:
	rm -f as

as: src/*.c
	gcc $^ -o $@ -Wall -Wextra -Wno-unused-variable -Wno-unused-parameter -Wno-missing-field-initializers -pedantic -Isrc/ -g
