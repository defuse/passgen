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
#include "wordlist.hpp"

#define PASSWORD_LENGTH 64
#define RANDOM_DATA_ERROR 3
#define NO_PASSWORD_COUNT 4
#define WORD_COUNT 10

/*
 * Fills 'buffer' with cryptographically secure random bytes.
 * buffer - gets filled with random bytes.
 * bufferlength - length of buffer
 */
bool getRandom(unsigned char* buffer, unsigned int bufferlength)
{
    FILE* random;

    random = fopen("/dev/urandom", "rb");
    if(random == NULL) {
        return false;
    }

    unsigned int read = fread(buffer, sizeof(unsigned char), bufferlength, random);
    if(read != bufferlength) {
        return false;
    }

    if (fclose(random) != 0) {
        return false;
    }

    return true;
}

bool getRandomUnsignedLong(unsigned long *random) {
    return getRandom((unsigned char*)random, sizeof(unsigned long));
}

void showHelp()
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
    puts("  -p, --password-count COUNT\t\tGenerate COUNT passwords");
}

inline unsigned long getMinimalBitMask(unsigned long toRepresent)
{
    unsigned long mask = 0;
    unsigned int bit = 0;
    while (mask < toRepresent)
    {
        mask = (mask << 1) | 1;
    }
    return mask;
}

bool getPassword(const char *set, unsigned char setLength, char *password, unsigned int passwordLength)
{
    unsigned int bufLen = passwordLength; 
    int bufIdx = 0;
    unsigned char *rndBuf = (unsigned char*)malloc(bufLen);

    unsigned char bitMask = getMinimalBitMask(setLength - 1) & 0xFF;

    if(!getRandom(rndBuf, bufLen))
    {
        memset(rndBuf, 0, bufLen);
        free(rndBuf);
        return false;
    }

    int i = 0;
    while(i < passwordLength)
    {
        // Read more random bytes if necessary.
        if(bufIdx >= bufLen)
        {
            if(!getRandom(rndBuf, bufLen))
            {
                memset(rndBuf, 0, bufLen);
                free(rndBuf);
                return false;
            }
            
            bufIdx = 0;
        }

        unsigned char c = rndBuf[bufIdx++];
        c = c & bitMask;

        // Discard the random byte if it isn't in range.
        if(c < setLength)
        {
            password[i] = set[c];
            i++;
        }
    }
    memset(rndBuf, 0, bufLen);
    free(rndBuf);
    return true;
}

bool showRandomWords()
{
    unsigned long random = 0;
    unsigned int words_printed = 0;
    while (words_printed < WORD_COUNT) {
        if (!getRandomUnsignedLong(&random)) {
            return false;
        }
        random = random & getMinimalBitMask(WORDLIST_WORD_COUNT);
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
    return true;
}

bool runtimeTests()
{
    /* Make sure the random number generator isn't *completely* broken. */
    unsigned char buffer[16];
    memset(buffer, 0, sizeof(buffer));
    if (!getRandom(buffer, sizeof(buffer))) {
        return false;
    }
    bool all_zero = true;
    for (int i = 0; i < sizeof(buffer); i++) {
        if (buffer[i] != 0) {
            all_zero = false;
            break;
        }
    }
    if (all_zero) {
        return false;
    }

    /* Make sure the random long generator isn't *completely* broken. */
    unsigned long test = 0;
    if (!getRandomUnsignedLong(&test)) {
        return false;
    }
    if (test == 0) {
        return false;
    }

    /* Test getMinimalBitMask around boundaries. */
    if (getMinimalBitMask(0) != 0) { return false; }
    if (getMinimalBitMask(1) != 1) { return false; }
    if (getMinimalBitMask(2) != 3) { return false; }
    if (getMinimalBitMask(3) != 3) { return false; }
    if (getMinimalBitMask(4) != 7) { return false; }
    if (getMinimalBitMask(5) != 7) { return false; }
    if (getMinimalBitMask(6) != 7) { return false; }
    if (getMinimalBitMask(7) != 7) { return false; }
    if (getMinimalBitMask(8) != 15) { return false; }

    /* Test getMinimalBitMask around weird values. */
    if (getMinimalBitMask(ULONG_MAX) != ULONG_MAX) { return false; }
    if (getMinimalBitMask(ULONG_MAX - 1) != ULONG_MAX) { return false; }

    char buffer2[128];
    getPassword("AB", 2, buffer2, sizeof(buffer2));
    unsigned int a_count = 0, b_count = 0;
    for (int i = 0; i < sizeof(buffer2); i++) {
        if (buffer2[i] == 'A') { a_count++; }
        else if (buffer2[i] == 'B') { b_count++; }
        else { return false; }
    }
    if (a_count == 0 || b_count == 0) {
        return false;
    }

    return true;
}

int main(int argc, char* argv[])
{
    if(argc < 2)
    {
        showHelp();
        return EXIT_FAILURE;
    }
    static struct option long_options[] = {
        {"help",              no_argument,       NULL, 'h' },
        {"hex",               no_argument,       NULL, 'x' },
        {"alpha",             no_argument,       NULL, 'n' },
        {"ascii",             no_argument,       NULL, 'a' },
        {"words",             no_argument,       NULL, 'w' },
        {"password-count",    required_argument, NULL, 'p' },
        {NULL, 0, NULL, 0 }
    };
    
    int optIndex =  0; // not currently being used
    int currentOptChar = 0;

    char set[255]; 
    unsigned char setLength = 0;
    unsigned int numberOfPasswords = 1;
    bool isPasswordTypeSet = false;
    bool isPasswordCountSet = false;
    bool generateWordPassword = false;
    while((currentOptChar = getopt_long(argc, argv, "hxnwap:", long_options, &optIndex)) != -1)
    {
            switch(currentOptChar)
            {
                case 'h': // help 
                    showHelp();
                    return EXIT_SUCCESS;
                case 'x': // hex password 
                    if(isPasswordTypeSet != true)
                    {
                        strcpy(set, "ABCDEF0123456789");
                        setLength = 16;
                        isPasswordTypeSet = true;
                    }
                    else
                    {
                        showHelp();
                        return EXIT_FAILURE;
                    }
                    break;
                case 'n': // alpha password
                    if(isPasswordTypeSet != true)
                    {
                        strcpy(set, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");
                        setLength = 62;
                        isPasswordTypeSet = true;
                    }
                    else
                    {
                        showHelp();
                        return EXIT_FAILURE;
                    }
                    break;
                case 'a': // ascii password
                    if(isPasswordTypeSet != true)
                    {
                        strcpy(set, "!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~");
                        setLength = 94;
                        isPasswordTypeSet = true;
                    }
                    else
                    {
                        showHelp();
                        return EXIT_FAILURE;
                    }
                    break;
                case 'w': // random words
                    if (isPasswordTypeSet != true) {
                        generateWordPassword = true;
                        isPasswordTypeSet = true;
                    } else {
                        showHelp();
                        return EXIT_FAILURE;
                    }
                    break;
                case 'p': // password-count 
                    if(isPasswordCountSet != true)
                    {
                        if(sscanf(optarg, "%u", &numberOfPasswords) > 0)
                        {
                            isPasswordCountSet = true;
                        }
                        else
                        {
                            showHelp();
                            return EXIT_FAILURE;
                        }
                    }
                    else 
                    { 
                        showHelp();
                        return EXIT_FAILURE;
                    }
                    break;
                default: // unknown opt
                    showHelp();
                    return EXIT_FAILURE;
            }
    }
    
    // user didn't choose a password type, just a switch?
    if(!isPasswordTypeSet)
    {
        showHelp();
        return EXIT_FAILURE;
    }

    if (!runtimeTests()) {
        fprintf(stderr, "ERROR: Runtime self-tests failed. SOMETHING IS WRONG\n");
        return EXIT_FAILURE;
    }

    if (generateWordPassword) {
        for (int i = 0; i < numberOfPasswords; i++) {
            if (!showRandomWords()) {
                fprintf(stderr, "Error getting random data.\n");
                return RANDOM_DATA_ERROR;
            }
        }
    } else {
        char result[PASSWORD_LENGTH];
        
        for(int i = 0; i < numberOfPasswords; i++)
        {
            if(getPassword(set, setLength, result, PASSWORD_LENGTH))
            {
                for(int j = 0; j < PASSWORD_LENGTH; j++)
                {
                    printf("%c", result[j]);
                }
                printf("\n");
            }
            else
            {
                fprintf(stderr, "Error getting random data.\n");
                return RANDOM_DATA_ERROR;
            }
            memset(result, 0, PASSWORD_LENGTH);
        }
    }

    return EXIT_SUCCESS;
}

