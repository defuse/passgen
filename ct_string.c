/*
 * This is a time-invariant and cache-invariant string builder library.
 * Written by Taylor Hornby (@DefuseSec) using Samuel Neves' integer functions.
 *
 * License
 * --------
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
 *  
 * You should have received a copy of the CC0 Public Domain Dedication along with
 * this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
 *
 * Explanation
 * ------------
 *
 * We want to build a string by concatenating smaller strings, but we don't want
 * to leak the contents of the string or the exact lengths of the strings we
 * concatenate.
 *
 * To do this, every string we concatenate must have two associated lengths:
 *
 *  1. The maximum length. This is the maximum length the concatenated string
 *     could have *possibly* been. This value *is* leaked through side channels.
 *
 *  2. The actual length. This is the actual number of meaningful characters in
 *     the string. This value should *not* be leaked through side channels.
 *
 * After all the concatenations are done, the attacker might have learnt all of
 * the maximum lengths, and therefore the maximum possible length of the result
 * (the sum of each maximum length), but they should not learn anything about
 * the contents of the string or the actual length of the string. 
 *
 * For example, the attacker might know that the final string is at most 100
 * characters since it was made by concatenating four strings that could be at
 * most 20 characters and two strings that could be at most 10 characters, but
 * they have no idea that each concatenation was really only adding three
 * characters for a total actual length of 18.
 * 
 * WARNING: Be cautious of other side channels that might leak information about
 * your string. If you print the string to a terminal, its word-wrap might leak
 * some information about it!
 */

#include <stdlib.h>
#include <string.h>

#include "ct_string.h"
#include "ct32.h"
#include "memset_s.h"

/*
 * Initializes a ct_string context.
 */
void ct_string_init(ct_string *str)
{
    str->allocated_length = 0;
    str->actual_length = 0;
    str->string = NULL;
}

/*
 * Concatenates a string to 'str'.
 *
 * The 'to_append' pointer must point to at least 'max_length' bytes of memory,
 * of which the first 'actual_length' bytes are the meaningful part that will be
 * concatenated to 'str'.
 *
 * Returns 0 on failure, 1 on success.
 */
int ct_string_concat(ct_string *str, const unsigned char *to_append, uint32_t max_length, uint32_t actual_length)
{
    if (max_length == 0 || actual_length == 0 || actual_length > max_length) {
        return 0;
    }

    /* Every time this function is called, the allocated length of the string
     * increases by max_length, and the new memory is initialized to zero. This
     * is important, since we're going to bitwise-or stuff into it later. */

    if (str->string == NULL) {
        /* If this is the first time, allocate the buffer and zero it. */
        str->string = malloc(max_length);
        if (str->string == NULL) {
            return 0;
        }
        memset(str->string, 0, max_length);
        str->allocated_length = max_length;
    } else {
        /* Otherwise, increase the length by max_length and zero the new bytes. */
        str->string = realloc(str->string, str->allocated_length + max_length);
        if (str->string == NULL) {
            return 0;
        }
        memset(str->string + str->allocated_length, 0, max_length);
        str->allocated_length += max_length;
    }

    for (uint32_t i = 0; i + max_length - 1 < str->allocated_length; i++) {
        /* outer_mask is 0xFFFFFFFF only when we're at the right spot in the
         * string to start ORing in bytes from to_append. This will leave all
         * the other bytes in the string unchanged until we get to the right
         * index. */
        uint32_t outer_mask = ct_mask_u32(ct_eq_u32(i, str->actual_length));
        for (uint32_t j = 0; j < max_length; j++) {
            /* inner_mask is 0xFFFFFFFF until we've gone past the actual length
             * of to_append, so that its padding bytes don't clobber our
             * string's zero padding bytes. */
            uint32_t inner_mask = ct_mask_u32(ct_lt_u32(j, actual_length));
            str->string[i+j] |= (uint32_t)to_append[j] & outer_mask & inner_mask;
        }
    }
    str->actual_length += actual_length;

    return 1;
}

/*
 * Copies the entire string to a buffer you provide, replacing the
 * not-meaningful characters at the end with 'filler'.
 *
 * The 'buf' pointer must point to enough memory to hold the entire *allocated*
 * string (not just the meaningful prefix). You can get the allocated length by
 * calling ct_string_allocated_length().
 */
void ct_string_finalize(ct_string *str, unsigned char *buf, unsigned char filler)
{
    for (uint32_t i = 0; i < str->allocated_length; i++) {
        /* buf[i] = (j < actual_length) ? str->string[i] : filler */
        buf[i] = ct_select_u32(str->string[i], filler, ct_lt_u32(i, str->actual_length));
    }
}

/*
 * Returns the *allocated* length of the string.
 */
uint32_t ct_string_allocated_length(ct_string *str)
{
    return str->allocated_length;
}

/*
 * Overwrites sensitive data then frees internal memory.
 */
void ct_string_deinit(ct_string *str)
{
    if (str->string != NULL) {
        memset_s(str->string, 0, str->allocated_length);
        free(str->string);
    }
    memset_s(&(str->allocated_length), 0, sizeof(str->allocated_length));
    memset_s(&(str->actual_length), 0, sizeof(str->actual_length));
}
