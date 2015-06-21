LOTS_O_WARNINGS = -pedantic -Werror -Wall -Wextra -Wwrite-strings -Winit-self -Wcast-align -Wcast-qual -Wpointer-arith -Wstrict-aliasing -Wformat=2 -Wmissing-declarations -Wmissing-include-dirs -Wno-unused-parameter -Wuninitialized -Wold-style-definition -Wstrict-prototypes -Wmissing-prototypes

PREFIX=/usr/bin

.PHONY: all
all: passgen

passgen: passgen.o libs/ct32.o libs/ct_string.o libs/memset_s.o
	gcc -std=c99 $(EXTRA_GCC_FLAGS) $(LOTS_O_WARNINGS) libs/ct32.o libs/memset_s.o libs/ct_string.o passgen.o -o passgen
	@echo '!!!'
	@echo '!!! --> Run `make test` and `make stat_test` to test the binary you just built!'
	@echo '!!!'

passgen.o: passgen.c libs/wordlist.h
	gcc -std=c99 $(EXTRA_GCC_FLAGS) $(LOTS_O_WARNINGS) -c passgen.c -o passgen.o

libs/ct32.o: libs/ct32.c libs/ct32.h
	gcc -std=c99 $(EXTRA_GCC_FLAGS) $(LOTS_O_WARNINGS) -c libs/ct32.c -o libs/ct32.o

libs/ct_string.o: libs/ct_string.c libs/ct_string.h
	gcc -std=c99 $(EXTRA_GCC_FLAGS) $(LOTS_O_WARNINGS) -c libs/ct_string.c -o libs/ct_string.o

libs/memset_s.o: libs/memset_s.c libs/memset_s.h
	gcc -std=c99 $(EXTRA_GCC_FLAGS) $(LOTS_O_WARNINGS) -c libs/memset_s.c -o libs/memset_s.o

libs/wordlist.h: tools/generate_wordlist.rb libs/wordlist.txt
	ruby tools/generate_wordlist.rb libs/wordlist.txt > libs/wordlist.h

# Rebuild the wordlist from the original source.
.PHONY: wordlist
wordlist:
	wget world.std.com/~reinhold/diceware.wordlist.asc -O - | egrep "[0-9]{5}" | cut -f 2 | egrep '^[[:alpha:]]{3,}[[:alpha:]]*$$' | sort -u > libs/wordlist.txt

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
install: passgen
	install -m 755 -D passgen $(PREFIX)/passgen

.PHONY: clean
clean:
	rm -f passgen passgen.o libs/ct32.o libs/ct_string.o libs/memset_s.o
	find . -name '*.gcda' -o -name '*.gcno' -delete
