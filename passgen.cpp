/* Command Line Password Generator for Windows and UNIX-like systems.
 * Copyright (C) 2011  FireXware <firexware@gmail.com>
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
 * Compiling with Visual C++:
 *  cl.exe passgen.cpp advapi32.lib
 * Compiling with G++:
 *  g++ passgen.cpp -o passgen
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#include <wincrypt.h>
#endif

#define PASSWORD_LENGTH 64

/*
 * Fills 'buffer' with cryptographically secure random bytes.
 * buffer - gets filled with random bytes.
 * bufferlength - length of buffer
 */
bool getRandom(unsigned char* buffer, unsigned int bufferlength)
{
#ifdef _WIN32
    HCRYPTPROV hCryptCtx = NULL;
    CryptAcquireContext(&hCryptCtx, NULL, MS_DEF_PROV, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT);
    if(hCryptCtx == NULL)
        return false;
    CryptGenRandom(hCryptCtx, bufferlength, buffer);
    CryptReleaseContext(hCryptCtx, 0);
#else
    FILE* random = fopen("/dev/random", "rb");
    if(random == NULL)
        return false;
    unsigned int read = fread(buffer, sizeof(unsigned char), bufferlength, random);
    if(read != bufferlength)
        return false;
    fclose(random);
#endif
    return true;
}

void showHelp()
{
    puts("Usage: passgen <type>");
    puts("Where <type> is one of:");
    puts("--hex 256 bit hex string");
    puts("--ascii 64 character ascii printable string");
    puts("--alpha 64 character alpha-numeric string");
    puts("--help show this page");
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

bool getPassword(char *set, unsigned char setLength, char *password, unsigned int passwordLength)
{
    unsigned int bufLen = passwordLength; 
    int bufIdx = 0;
    unsigned char *rndBuf = (unsigned char*)malloc(bufLen);

    unsigned char bitMask = getMinimalBitMask(setLength - 1);

    if(!getRandom(rndBuf, bufLen))
        return false;

    int i = 0;
    while(i < passwordLength)
    {
        // Read more random bytes if necessary.
        if(bufIdx >= bufLen)
        {
            if(!getRandom(rndBuf, bufLen))
                return false;
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
    memset(rndBuf, 0xFF, bufLen);
    free(rndBuf);
    return true;
}

int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        showHelp();
        return 1;
    }

    char set[255]; 
    unsigned char setlength = 0;
    if(strncmp(argv[1],"--help", 6) == 0)
    {
        showHelp();
        return 0;
    }
    else if(strncmp(argv[1], "--ascii", 6) == 0)
    {
        strcpy(set, "!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~");
        setlength = 94;
    }
    else if(strncmp(argv[1], "--hex", 5) == 0)
    {
        strcpy(set, "ABCDEF0123456789");
        setlength = 16;
    }
    else if(strncmp(argv[1], "--alpha", 7) == 0)
    {
        strcpy(set, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");
        setlength = 62;
    }
    else
    {
        showHelp();
        return 1;
    }

    char result[PASSWORD_LENGTH];
    puts("Getting random data...");
    if(getPassword(set, setlength, result, PASSWORD_LENGTH))
    {
        for(int i = 0; i < PASSWORD_LENGTH; i++)
        {
            printf("%c", result[i]);
        }
        printf("\n");
    }
    else
    {
        fprintf(stderr, "Error getting random data.\n");
        return 2;
    }
    memset(result, 0, PASSWORD_LENGTH);
    return 0;
}

