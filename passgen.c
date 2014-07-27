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
#include <stdint.h>

/* Constant time integer functions by Samuel Neves */
#include "libs/ct32.h"
/* Constant time string library. */
#include "libs/ct_string.h"
/* Automatically generated file containing the wordlist array. */
#include "libs/wordlist.h"
/* An implementation of memset() that the compiler won't optimize out. */
#include "libs/memset_s.h"

#define PASSWORD_LENGTH 64
#define WORD_COUNT 10

#define CHARSET_HEX "0123456789ABCDEF"
#define CHARSET_ALPHANUMERIC "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"
#define CHARSET_ASCII "!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"
#define CHARSET_DIGIT "0123456789"

int getRandom(void* buffer, unsigned long bufferlength);
int getRandomUnsignedLong(unsigned long *random);
void showHelp(void);
unsigned long getLeastCoveringMask(unsigned long toRepresent);
int getPassword(const char *set, unsigned long setLength, unsigned char *password, unsigned long passwordLength);
int showRandomWords(void);
int runtimeTests(void);
uint32_t lookup_word(unsigned char *buf, uint32_t index);

static struct option long_options[] = {
    {"help",              no_argument,       NULL, 'h' },
    {"hex",               no_argument,       NULL, 'x' },
    {"alpha",             no_argument,       NULL, 'n' },
    {"ascii",             no_argument,       NULL, 'a' },
    {"digit",             no_argument,       NULL, 'd' },
    {"words",             no_argument,       NULL, 'w' },
    {"password-count",    required_argument, NULL, 'p' },
    /* This skips the self test -- don't do it unless you're testing. */
    {"dont-use-this",     no_argument,       NULL, 'z' },
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
    int numberOfPasswords = 1;
    int generateWordPassword = 0;
    int skipSelfTest = 0;

    /* Variables used while parsing. */
    int optionCharacter = 0;
    int isPasswordTypeSet = 0;
    int isPasswordCountSet = 0;
    while((optionCharacter = getopt_long(argc, argv, "hzxndwap:", long_options, NULL)) != -1) {
            switch(optionCharacter)
            {
                case 'h': /* help */
                    showHelp();
                    return EXIT_SUCCESS;

                case 'x': /* hex password */
                    if (isPasswordTypeSet) {
                        showHelp();
                        return EXIT_FAILURE;
                    }
                    set = CHARSET_HEX;
                    isPasswordTypeSet = 1;
                    break;

                case 'n': /* alphanumeric password */
                    if (isPasswordTypeSet) {
                        showHelp();
                        return EXIT_FAILURE;
                    }
                    set = CHARSET_ALPHANUMERIC;
                    isPasswordTypeSet = 1;
                    break;

                case 'a': /* ascii password */
                    if (isPasswordTypeSet) {
                        showHelp();
                        return EXIT_FAILURE;
                    }
                    set = CHARSET_ASCII;
                    isPasswordTypeSet = 1;
                    break;

                case 'd': /* ascii password */
                    if (isPasswordTypeSet) {
                        showHelp();
                        return EXIT_FAILURE;
                    }
                    set = CHARSET_DIGIT;
                    isPasswordTypeSet = 1;
                    break;

                case 'w': /* words password */
                    if (isPasswordTypeSet) {
                        showHelp();
                        return EXIT_FAILURE;
                    }
                    generateWordPassword = 1;
                    isPasswordTypeSet = 1;
                    break;

                case 'p': /* multiple passwords */
                    if (isPasswordCountSet) {
                        showHelp();
                        return EXIT_FAILURE;
                    }
                    if(sscanf(optarg, "%d", &numberOfPasswords) == 1) {
                        if (numberOfPasswords <= 0) {
                            showHelp();
                            return EXIT_FAILURE;
                        }
                        isPasswordCountSet = 1;
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
    
    /* Choosing a password type is mandatory. */
    if(!isPasswordTypeSet) {
        showHelp();
        return EXIT_FAILURE;
    }

    /* We run unit tests on EVERY execution just to make sure nothing is
     * horribly wrong. There's an option to skip it, which is used for testing
     * purposes (see test.rb). */
    if (!skipSelfTest && !runtimeTests()) {
        fprintf(stderr, "ERROR: Runtime self-tests failed. SOMETHING IS WRONG\n");
        return EXIT_FAILURE;
    }

    if (generateWordPassword) {
        for (int i = 0; i < numberOfPasswords; i++) {
            if (!showRandomWords()) {
                fprintf(stderr, "Error getting random data.\n");
                return EXIT_FAILURE;
            }
        }
    } else {
        unsigned char result[PASSWORD_LENGTH];
        
        for(int i = 0; i < numberOfPasswords; i++) {
            if(getPassword(set, strlen(set), result, PASSWORD_LENGTH)) {
                fwrite(result, sizeof(unsigned char), PASSWORD_LENGTH, stdout);
                printf("\n");
            } else {
                memset_s(result, 0, PASSWORD_LENGTH);
                fprintf(stderr, "Error getting random data or allocating memory.\n");
                return EXIT_FAILURE;
            }
            memset_s(result, 0, PASSWORD_LENGTH);
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

int getPassword(const char *set, unsigned long setLength, unsigned char *password, unsigned long passwordLength)
{
    unsigned long bufLen = passwordLength; 
    unsigned long bufIdx = 0;
    unsigned char *rndBuf = (unsigned char*)malloc(bufLen);

    if (rndBuf == NULL) {
        return 0;
    }

    if (setLength < 1 || setLength > 256) {
        return 0;
    }
    unsigned char bitMask = getLeastCoveringMask(setLength - 1ul) & 0xFF;

    if(!getRandom(rndBuf, bufLen)) {
        memset_s(rndBuf, 0, bufLen);
        free(rndBuf);
        return 0;
    }

    unsigned long i = 0;
    while(i < passwordLength) {
        // Read more random bytes if necessary.
        if(bufIdx >= bufLen) {
            if(!getRandom(rndBuf, bufLen)) {
                memset_s(rndBuf, 0, bufLen);
                free(rndBuf);
                return 0;
            }
            
            bufIdx = 0;
        }

        unsigned char c = rndBuf[bufIdx++];
        c = c & bitMask;

        // Discard the random byte if it isn't in range.
        if(c < setLength) {
            password[i] = invariant_time_lookup((const unsigned char*)set, setLength, c);
            i++;
        }
    }

    memset_s(rndBuf, 0, bufLen);
    free(rndBuf);
    return 1;
}

int showRandomWords(void)
{
    unsigned int words_added = 0;
    unsigned long random = 0;
    unsigned char word[WORDLIST_MAX_LENGTH];
    uint32_t word_length = 0;
    ct_string str;
    ct_string_init(&str);

    while (words_added < WORD_COUNT) {
        if (!getRandomUnsignedLong(&random)) {
            return 0;
        }
        random = random & getLeastCoveringMask(WORDLIST_WORD_COUNT - 1);

        if (random < WORDLIST_WORD_COUNT) {
            word_length = lookup_word(word, random);

            /* Concatenate the '.' between words if it isn't the first word. */
            if (words_added > 0) {
                if (!ct_string_concat(&str, (const unsigned char *)".", 1, 1)) {
                    return 0;
                }
            }

            /* Concatenate the word itself. */
            if (!ct_string_concat(&str, word, WORDLIST_MAX_LENGTH, word_length)) {
                return 0;
            }

            words_added++;
        }

    }

    uint32_t total_length = ct_string_allocated_length(&str);

    unsigned char *final = malloc(total_length);
    if (final == NULL) {
        return 0;
    }

    ct_string_finalize(&str, final, '.');
    fwrite(final, sizeof(unsigned char), total_length, stdout);
    printf("\n");

    memset_s(&random, 0, sizeof(random));
    memset_s(word, 0, WORDLIST_MAX_LENGTH);
    memset_s(final, 0, total_length);

    ct_string_deinit(&str);
    free(final);

    return 1;
}

uint32_t lookup_word(unsigned char *buf, uint32_t index)
{
    uint32_t length = 0;
    for (uint32_t i = 0; i < WORDLIST_MAX_LENGTH; i++) {
        buf[i] = 0;
    }

    for (uint32_t i = 0; i < WORDLIST_WORD_COUNT; i++) {
        uint32_t mask = ct_mask_u32(ct_eq_u32(i, index));
        for (uint32_t j = 0; j < WORDLIST_MAX_LENGTH; j++) {
            buf[j] |= words[i][j+1] & mask;
        }
        length |= words[i][0] & mask;
    }

    return length;
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
    memset_s(buffer, 0, sizeof(buffer));
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
    if (getLeastCoveringMask(255) != 255) { return 0; }
    if (getLeastCoveringMask(ULONG_MAX) != ULONG_MAX) { return 0; }
    if (getLeastCoveringMask(ULONG_MAX - 1) != ULONG_MAX) { return 0; }

    /* Test that the first and last character in the set can be selected. */
    unsigned char buffer2[128];
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

    /* Make sure memset_s zeroes the memory. */
    buffer2[0] = -1;
    buffer2[3] = -1;
    buffer2[127] = -1;
    memset_s(buffer2, 0, sizeof(buffer2));
    if (buffer2[0] != 0 || buffer2[3] != 0 || buffer2[127] != 0) {
        return 0;
    }

    /* Test the functions we use from Samuel Neves' code. */
    // FIXME: these tests don't reflect what we use anymore
    if (ct_eq_u32(0, 0) != 1) { return 0; }
    if (ct_eq_u32(5, 5) != 1) { return 0; }
    if (ct_eq_u32(0x80000000u, 0x80000000u) != 1) { return 0; }
    if (ct_eq_u32(0, 1) != 0) { return 0; }
    if (ct_eq_u32(1, 0) != 0) { return 0; }
    if (ct_eq_u32(2, 0) != 0) { return 0; }
    if (ct_eq_u32(3, 0) != 0) { return 0; }
    if (ct_eq_u32(1, 2) != 0) { return 0; }
    if (ct_eq_u32(0x80000000u, 0) != 0) { return 0; }
    if (ct_mask_u32(ct_eq_u32(1, 1)) != UINT32_MAX) { return 0; }
    if (ct_mask_u32(ct_eq_u32(1, 0)) != 0) { return 0; }

    /* Test the constant-time array lookup code. */
    const char *set = CHARSET_ASCII;
    for (size_t i = 0; i < strlen(set); i++) {
        if (set[i] != invariant_time_lookup((const unsigned char*)set, strlen(set), i)) {
            return 0;
        }
    }

    /* Test constant time string library. */
    ct_string str;
    ct_string_init(&str);
    ct_string_concat(&str, (const unsigned char *)"ABCDEF", 6, 3);
    ct_string_concat(&str, (const unsigned char *)"GHIJKL", 6, 3);
    ct_string_concat(&str, (const unsigned char *)"12345", 5, 5);
    if (ct_string_allocated_length(&str) != 17) { return 0; }
    unsigned char str_result[17];
    ct_string_finalize(&str, str_result, 'Z');
    if (memcmp(str_result, "ABCGHI12345ZZZZZZ", 17) != 0) {
        return 0;
    }
    ct_string_deinit(&str);

    return 1;
}
