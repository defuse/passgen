passgen: passgen.cpp wordlist.txt generate_wordlist.rb
	ruby generate_wordlist.rb > wordlist.hpp
	g++ passgen.cpp -o passgen

clean:
	rm wordlist.hpp
	rm passgen
