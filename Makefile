passgen: passgen.cpp wordlist.txt generate_wordlist.rb
	ruby generate_wordlist.rb > wordlist.hpp
	g++ passgen.cpp -o passgen

# Rebuild the wordlist from the original source.
wordlist:
	wget world.std.com/~reinhold/diceware.wordlist.asc -O - | egrep "[0-9]{5}" | cut -f 2 | egrep '^[[:alpha:]]{3,}[[:alpha:]]*$$' | sort -u > wordlist.txt

# NOTE: The `sort -u` serves a security purpose here:
# If a network attacker injects duplicate words as we download the list,
# then passgen will over-report the size of the word set to the user.

# The $$ instead of $ in the egrep command is a Make-escaped $.

clean:
	rm wordlist.hpp
	rm passgen
