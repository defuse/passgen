CFLAGS += -std=c99 \
		  -pedantic \
		  -Werror \
		  -Wall \
		  -Wextra \
		  -Wwrite-strings \
		  -Winit-self \
		  -Wcast-align \
		  -Wcast-qual \
		  -Wpointer-arith \
		  -Wstrict-aliasing \
		  -Wformat=2 \
		  -Wmissing-declarations \
		  -Wmissing-include-dirs \
		  -Wno-unused-parameter \
		  -Wuninitialized \
		  -Wold-style-definition \
		  -Wstrict-prototypes \
		  -Wmissing-prototypes


# Often used when packaging software to copy files to a temp
# directory before tarballing. Defaults to none (/)
DESTDIR =

# Base directory the program will end up being installed to
PREFIX = /usr

BINDIR = $(PREFIX)/bin
DATADIR= $(PREFIX)/share



.PHONY: all
all: passgen passgen.1

passgen: passgen.o libs/ct32.o libs/ct_string.o libs/memset_s.o
	$(CC) -std=c99 -o $@ $^
	@echo '!!!'
	@echo '!!! --> Run `make test` and `make stat_test` to test the binary you just built!'
	@echo '!!!'

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

libs/wordlist.h: tools/generate_wordlist.rb libs/wordlist.txt
	ruby tools/generate_wordlist.rb libs/wordlist.txt > libs/wordlist.h

# Rebuild the wordlist from the original source.
.PHONY: wordlist
wordlist:
	wget world.std.com/~reinhold/diceware.wordlist.asc -O - | egrep "[0-9]{5}" | cut -f 2 | egrep '^[[:alpha:]]{3,}[[:alpha:]]*$$' | sort -u > libs/wordlist.txt

passgen.1: README.md
	ronn -r $< --pipe > $@

# NOTE: The `sort -u` serves a security purpose here:
# Without it, if a network attacker injected duplicate words into the downloaded
# list, passgen would over-report the size of the word set to the user.

# The $$ instead of $ in the egrep command is a Make-escaped $.

.PHONY: test
test:
	ruby tools/test.rb

.PHONY: stat_test
stat_test:
	ruby tools/statistical_test.rb

.PHONY: stat_test_fast
stat_test_fast:
	ruby tools/statistical_test.rb fast

.PHONY: install
install: passgen passgen.1
	install -m 755 -D passgen "$(DESTDIR)/$(BINDIR)/passgen"
	install -m 755 -D passgen.1 "$(DESTDIR)/$(DATADIR)/man/man1/passgen.1"

.PHONY: clean
clean:
	rm -f passgen passgen.o libs/ct32.o libs/ct_string.o libs/memset_s.o
	find -name '*.gcda' -o -name '*.gcno' -delete
