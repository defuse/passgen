/* Command Line Password Generator for Windows and UNIX-like systems.
 * Copyright (C) 2011  Taylor Hornby
 * https://defuse.ca/ 
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
 * Modifications by Stephen Chavez (dicesoft.net)
 * 1. Better piping support for linux/unix oses
 * 2. Added password-count option
 * 3. Added quick mode for linux/unix oses (Now removed)
 * 4. There is now proper command line parsing
 *
 * Compiling with G++:
 *  g++ passgen.cpp -o passgen
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

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
    puts("Where <type> is one of:");
    puts("  -x, --hex\t\t\t\t64-character hex string");
    puts("  -a, --ascii\t\t\t\t64-character ASCII string");
    puts("  -n, --alpha\t\t\t\t64-character alpha-numeric string");
    printf("  -w, --words\t\t\t\t%d random words from a list of %d\n", WORD_COUNT, WORDLIST_WORD_COUNT);
    puts("  -h, --help\t\t\t\tShow this help menu");

    puts("Where <optional arguments> can be:");
    puts("  -p, --password-count COUNT\t\tGenerate COUNT passwords");
}

inline unsigned char getMinimalBitMask(unsigned char toRepresent)
{
    if(toRepresent <= 0x01)
        return 0x01;
    else if(toRepresent <= 0x03)
        return 0x03;
    else if(toRepresent <= 0x07)
        return 0x07;
    else if(toRepresent <= 0x0F)
        return 0x0F;
    else if(toRepresent <= 0x1F)
        return 0x1F;
    else if(toRepresent <= 0x3F)
        return 0x3F;
    else if(toRepresent <= 0x7F)
        return 0x7F;
    else
        return 0xFF;
}

inline unsigned long getMinimalBitMaskForInteger(unsigned long toRepresent)
{
    unsigned long mask = 0;
    unsigned int bit = 0;
    while (mask <= toRepresent)
    {
        mask = (mask << 1) | 1;
    }
    return mask;
}

bool getPassword(char *set, unsigned char setLength, char *password, unsigned int passwordLength)
{
    unsigned int bufLen = passwordLength; 
    int bufIdx = 0;
    unsigned char *rndBuf = (unsigned char*)malloc(bufLen);

    unsigned char bitMask = getMinimalBitMask(setLength - 1);

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

void showRandomWords()
{
    unsigned long random = 0;
    unsigned int words_printed = 0;
    while (words_printed < WORD_COUNT) {
        if (!getRandomUnsignedLong(&random)) {
            printf("ERROR: RANDOMNESS FAILURE. DO NOT USE.\n");
            exit(1);
        }
        random = random & getMinimalBitMaskForInteger(WORDLIST_WORD_COUNT);
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
                    showRandomWords();
                    return EXIT_SUCCESS;
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
    return EXIT_SUCCESS;
}

