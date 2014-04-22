
LOTS_O_WARNINGS = -pedantic -Werror -Wall -Wextra -Wwrite-strings -Winit-self -Wcast-align -Wcast-qual -Wpointer-arith -Wstrict-aliasing -Wformat=2 -Wmissing-declarations -Wmissing-include-dirs -Wno-unused-parameter -Wuninitialized -Wold-style-definition -Wstrict-prototypes -Wmissing-prototypes

passgen: passgen.c wordlist.h
	gcc -std=c99 $(LOTS_O_WARNINGS) passgen.c -o passgen

testbuild: passgen.c wordlist.h
	gcc -fprofile-arcs -ftest-coverage -std=c99 $(LOTS_O_WARNINGS) passgen.c -o passgen

wordlist.h: generate_wordlist.rb wordlist.txt
	ruby generate_wordlist.rb > wordlist.h

# Rebuild the wordlist from the original source.
wordlist:
	wget world.std.com/~reinhold/diceware.wordlist.asc -O - | egrep "[0-9]{5}" | cut -f 2 | egrep '^[[:alpha:]]{3,}[[:alpha:]]*$$' | sort -u > wordlist.txt

# NOTE: The `sort -u` serves a security purpose here:
# If a network attacker injects duplicate words as we download the list,
# then passgen will over-report the size of the word set to the user.

# The $$ instead of $ in the egrep command is a Make-escaped $.

test:
	ruby test.rb

stat_test:
	ruby statistical_test.rb

stat_test_fast:
	ruby statistical_test.rb fast

clean:
	rm passgen
	find -name '*.gcda' -o -name '*.gcno' -delete
