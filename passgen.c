/*
 * Command-line password generator tool.
 * Copyright (C) 2011  Taylor Hornby
 * Web: https://defuse.ca/passgen.htm
 * GitHub: https://github.com/defuse/passgen
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Modifications made by Stephen Chavez (dicesoft.net)
 * 1. Better piping support for linux/unix oses
 * 2. Added password-count option
 * 3. Added quick mode for linux/unix oses (now removed - /dev/urandom only)
 * 4. There is now proper command line parsing
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <limits.h>

/* Automatically generated file containing the wordlist array. */
#include "wordlist.h"

#define PASSWORD_LENGTH 64
#define WORD_COUNT 10

#define CHARSET_HEX "0123456789ABCDEF"
#define CHARSET_ALPHANUMERIC "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"
#define CHARSET_ASCII "!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"

int getRandom(void* buffer, unsigned long bufferlength);
int getRandomUnsignedLong(unsigned long *random);
void showHelp(void);
unsigned long getLeastCoveringMask(unsigned long toRepresent);
int getPassword(const char *set, unsigned long setLength, char *password, unsigned long passwordLength);
int showRandomWords(void);
int runtimeTests(void);

static struct option long_options[] = {{
    {"help",              no_argument,       NULL, 'h' },
    {"hex",               no_argument,       NULL, 'x' },
    {"alpha",             no_argument,       NULL, 'n' },
    {"ascii",             no_argument,       NULL, 'a' },
    {"words",             no_argument,       NULL, 'w' },
    {"dont-use-this",     no_argument,       NULL, 'z' },
    {"password-count",    required_argument, NULL, 'p' },
    {NULL, 0, NULL, 0 }
};

int main(int argc, char* argv[])
{
    if(argc < 2) {
        showHelp();
        return EXIT_FAILURE;
    }
    
    /* Options */
    const char *set; 
    unsigned int numberOfPasswords = 1;
    int generateWordPassword = 0;
    int skipSelfTest = 0;

    /* Variables used while parsing. */
    int optionCharacter = 0;
    int isPasswordTypeSet = 0;
    int isPasswordCountSet = 0;
    while((optionCharacter = getopt_long(argc, argv, "hzxnwap:", long_options, NULL)) != -1) {
            switch(optionCharacter)
            {
                case 'h': /* help */
                    showHelp();
                    return EXIT_SUCCESS;

                case 'x': /* hex password */
                    if(isPasswordTypeSet == 0) {
                        set = CHARSET_HEX;
                        isPasswordTypeSet = 1;
                    } else {
                        showHelp();
                        return EXIT_FAILURE;
                    }
                    break;

                case 'n': /* alphanumeric password */
                    if(isPasswordTypeSet == 0) {
                        set = CHARSET_ALPHANUMERIC;
                        isPasswordTypeSet = 1;
                    } else {
                        showHelp();
                        return EXIT_FAILURE;
                    }
                    break;

                case 'a': /* ascii password */
                    if(isPasswordTypeSet == 0) {
                        set = CHARSET_ASCII;
                        isPasswordTypeSet = 1;
                    } else {
                        showHelp();
                        return EXIT_FAILURE;
                    }
                    break;

                case 'w': /* words password */
                    if (isPasswordTypeSet == 0) {
                        generateWordPassword = 1;
                        isPasswordTypeSet = 1;
                    } else {
                        showHelp();
                        return EXIT_FAILURE;
                    }
                    break;

                case 'p': /* multiple passwords */
                    if(isPasswordCountSet == 0) {
                        if(sscanf(optarg, "%u", &numberOfPasswords) > 0) {
                            isPasswordCountSet = 1;
                        } else {
                            showHelp();
                            return EXIT_FAILURE;
                        }
                    } else { 
                        showHelp();
                        return EXIT_FAILURE;
                    }
                    break;

                case 'z': /* skip self test - for test.rb */
                    skipSelfTest = 1;
                    break;

                default:
                    showHelp();
                    return EXIT_FAILURE;
            }
    }
    
    // user didn't choose a password type, just a switch?
    if(!isPasswordTypeSet) {
        showHelp();
        return EXIT_FAILURE;
    }

    if (!skipSelfTest && !runtimeTests()) {
        fprintf(stderr, "ERROR: Runtime self-tests failed. SOMETHING IS WRONG\n");
        return EXIT_FAILURE;
    }

    if (generateWordPassword) {
        for (unsigned int i = 0; i < numberOfPasswords; i++) {
            if (!showRandomWords()) {
                fprintf(stderr, "Error getting random data.\n");
                return EXIT_FAILURE;
            }
        }
    } else {
        char result[PASSWORD_LENGTH];
        
        for(unsigned int i = 0; i < numberOfPasswords; i++) {
            if(getPassword(set, strlen(set), result, PASSWORD_LENGTH)) {
                for(int j = 0; j < PASSWORD_LENGTH; j++) {
                    printf("%c", result[j]);
                }
                printf("\n");
            } else {
                fprintf(stderr, "Error getting random data.\n");
                return EXIT_FAILURE;
            }
            memset(result, 0, PASSWORD_LENGTH);
        }
    }

    return EXIT_SUCCESS;
}

void showHelp(void)
{
    puts("Usage: passgen <type> <optional arguments>");
    puts("WARNING: If automated, you MUST check that the exit status is 0.");
    puts("Where <type> is one of:");
    puts("  -x, --hex\t\t\t\t64-character hex string");
    puts("  -a, --ascii\t\t\t\t64-character ASCII string");
    puts("  -n, --alpha\t\t\t\t64-character alpha-numeric string");
    printf("  -w, --words\t\t\t\t%d random words from a list of %d\n", WORD_COUNT, WORDLIST_WORD_COUNT);
    puts("  -h, --help\t\t\t\tShow this help menu");

    puts("Where <optional arguments> can be:");
    puts("  -p, --password-count N\t\tSpecify number of passwords to generate");
}

int getPassword(const char *set, unsigned long setLength, char *password, unsigned long passwordLength)
{
    unsigned long bufLen = passwordLength; 
    unsigned long bufIdx = 0;
    unsigned char *rndBuf = (unsigned char*)malloc(bufLen);

    if (setLength < 1) {
        return 0;
    }
    unsigned char bitMask = getLeastCoveringMask(setLength - 1ul) & 0xFF;


    if(!getRandom(rndBuf, bufLen)) {
        memset(rndBuf, 0, bufLen);
        free(rndBuf);
        return 0;
    }

    unsigned long i = 0;
    while(i < passwordLength) {
        // Read more random bytes if necessary.
        if(bufIdx >= bufLen) {
            if(!getRandom(rndBuf, bufLen)) {
                memset(rndBuf, 0, bufLen);
                free(rndBuf);
                return 0;
            }
            
            bufIdx = 0;
        }

        unsigned char c = rndBuf[bufIdx++];
        c = c & bitMask;

        // Discard the random byte if it isn't in range.
        if(c < setLength) {
            password[i] = set[c];
            i++;
        }
    }

    memset(rndBuf, 0, bufLen);
    free(rndBuf);
    return 1;
}

int showRandomWords(void)
{
    unsigned long random = 0;
    unsigned int words_printed = 0;
    while (words_printed < WORD_COUNT) {
        if (!getRandomUnsignedLong(&random)) {
            return 0;
        }
        random = random & getLeastCoveringMask(WORDLIST_WORD_COUNT - 1);
        if (random < WORDLIST_WORD_COUNT) {
            printf("%s", words[random]);
            if (words_printed != WORD_COUNT - 1) {
                printf(".");
            }
            words_printed++;
        }
    }
    memset(&random, 0, sizeof(random));
    printf("\n");
    return 1;
}

unsigned long getLeastCoveringMask(unsigned long toRepresent)
{
    unsigned long mask = 0;
    while (mask < toRepresent) {
        mask = (mask << 1) | 1;
    }
    return mask;
}

/*
 * Fills 'buffer' with cryptographically secure random bytes.
 * buffer - gets filled with random bytes.
 * bufferlength - length of buffer
 */
int getRandom(void* buffer, unsigned long bufferlength)
{
    FILE* random;

    random = fopen("/dev/urandom", "rb");
    if(random == NULL) {
        return 0;
    }

    size_t read = fread(buffer, sizeof(unsigned char), bufferlength, random);
    if(read != bufferlength) {
        return 0;
    }

    if (fclose(random) != 0) {
        return 0;
    }

    return 1;
}

int getRandomUnsignedLong(unsigned long *random)
{
    return getRandom(random, sizeof(unsigned long));
}

int runtimeTests(void)
{
    /* Make sure the random number generator isn't *completely* broken. */
    unsigned char buffer[16];
    memset(buffer, 0, sizeof(buffer));
    if (!getRandom(buffer, sizeof(buffer))) {
        return 0;
    }
    int all_zero = 1;
    for (size_t i = 0; i < sizeof(buffer); i++) {
        if (buffer[i] != 0) {
            all_zero = 0;
            break;
        }
    }
    if (all_zero) {
        return 0;
    }

    /* Make sure the random long generator isn't *completely* broken. */
    unsigned long test = 0;
    if (!getRandomUnsignedLong(&test)) {
        return 0;
    }
    if (test == 0) {
        return 0;
    }

    /* Test getLeastCoveringMask around boundaries. */
    if (getLeastCoveringMask(0) != 0) { return 0; }
    if (getLeastCoveringMask(1) != 1) { return 0; }
    if (getLeastCoveringMask(2) != 3) { return 0; }
    if (getLeastCoveringMask(3) != 3) { return 0; }
    if (getLeastCoveringMask(4) != 7) { return 0; }
    if (getLeastCoveringMask(5) != 7) { return 0; }
    if (getLeastCoveringMask(6) != 7) { return 0; }
    if (getLeastCoveringMask(7) != 7) { return 0; }
    if (getLeastCoveringMask(8) != 15) { return 0; }

    /* Test getLeastCoveringMask around weird values. */
    if (getLeastCoveringMask(ULONG_MAX) != ULONG_MAX) { return 0; }
    if (getLeastCoveringMask(ULONG_MAX - 1) != ULONG_MAX) { return 0; }

    char buffer2[128];
    getPassword("AB", 2, buffer2, sizeof(buffer2));
    unsigned int a_count = 0, b_count = 0;
    for (size_t i = 0; i < sizeof(buffer2); i++) {
        if (buffer2[i] == 'A') { a_count++; }
        else if (buffer2[i] == 'B') { b_count++; }
        else { return 0; }
    }
    if (a_count == 0 || b_count == 0) {
        return 0;
    }

    return 1;
}
