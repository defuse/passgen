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
 * 3. Added quick mode for linux/unix oses
 * 4. There is now proper command line parsing
 *
 * Compiling with G++:
 *  g++ passgen.cpp -o passgen
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#define PASSWORD_LENGTH 64
#define RANDOM_DATA_ERROR 3
#define NO_PASSWORD_COUNT 4

/*
 * Fills 'buffer' with cryptographically secure random bytes.
 * buffer - gets filled with random bytes.
 * bufferlength - length of buffer
 */
bool getRandom(unsigned char* buffer, unsigned int bufferlength, bool quickMode)
{
    FILE* random;

    if(quickMode)
    {
        random = fopen("/dev/urandom", "rb");
    }
    else
    {
        random = fopen("/dev/random", "rb");
    }

    if(random == NULL)
        return false;
    unsigned int read = fread(buffer, sizeof(unsigned char), bufferlength, random);
    if(read != bufferlength)
        return false;
    fclose(random);
    return true;
}

void showHelp()
{
    puts("Usage: passgen <type> <optional arguments>");
    puts("Where <type> is one of:");
    puts("  -h, --hex\t\t\t\t64-character hex string");
    puts("  -t, --ascii\t\t\t\t64-character ASCII string");
    puts("  -a, --alpha\t\t\t\t64-character alpha-numeric string");
    puts("  -h, --help\t\t\t\tShow this help menu");

    puts("Where <optional arguments> can be:");
    puts("  -q, --quick\t\t\t\tUse /dev/urandom instead of /dev/random");
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

bool getPassword(char *set, unsigned char setLength, char *password, unsigned int passwordLength, bool quickMode)
{
    unsigned int bufLen = passwordLength; 
    int bufIdx = 0;
    unsigned char *rndBuf = (unsigned char*)malloc(bufLen);

    unsigned char bitMask = getMinimalBitMask(setLength - 1);

    if(!getRandom(rndBuf, bufLen, quickMode))
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
            if(!getRandom(rndBuf, bufLen, quickMode))
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

int main(int argc, char* argv[])
{
    if(argc < 2)
    {
        showHelp();
        return EXIT_FAILURE;
    }
    static struct option long_options[] = {
        {"help",              no_argument,       NULL, 'h' },
        {"quick",             no_argument,       NULL, 'q' },
        {"hex",               no_argument,       NULL, 'n' },
        {"alpha",             no_argument,       NULL, 'a' },
        {"ascii",             no_argument,       NULL, 't' },
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
    bool quickMode = false;
    while((currentOptChar = getopt_long(argc, argv, "hqnatp:", long_options, &optIndex)) != -1)
    {
            switch(currentOptChar)
            {
                case 'h': // help 
                    showHelp();
                    return EXIT_SUCCESS;
                case 'q': // quick mode
                    if(quickMode != true)
                    {
                        quickMode = true;
                    }
                    else
                    {
                        showHelp();
                        return EXIT_FAILURE;
                    }
                    break; 
                case 'n': // hex password 
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
                case 'a': // alpha password
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
                case 't': // ascii password
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
        if(getPassword(set, setLength, result, PASSWORD_LENGTH, quickMode))
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

