/* I stole this file from: https://gist.github.com/sneves/10845247 */

/*
Constant-time integer comparisons
 
Written in 2014 by Samuel Neves <sneves@dei.uc.pt>
 
To the extent possible under law, the author(s) have dedicated all copyright
and related and neighboring rights to this software to the public domain
worldwide. This software is distributed without any warranty.
 
You should have received a copy of the CC0 Public Domain Dedication along with
this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#include <stdint.h>

#include "ct32.h"

/* Unsigned comparisons */
/* Return 1 if condition is true, 0 otherwise */
int ct_isnonzero_u32(uint32_t x)
{
    return (x|-x)>>31;
}

int ct_iszero_u32(uint32_t x)
{
    return 1 ^ ct_isnonzero_u32(x);
}

int ct_neq_u32(uint32_t x, uint32_t y)
{
    return ((x-y)|(y-x))>>31;
}

int ct_eq_u32(uint32_t x, uint32_t y)
{
    return 1 ^ ct_neq_u32(x, y);
}

int ct_lt_u32(uint32_t x, uint32_t y)
{
    return (x^((x^y)|((x-y)^y)))>>31;
}

int ct_gt_u32(uint32_t x, uint32_t y)
{
    return ct_lt_u32(y, x);
}

int ct_le_u32(uint32_t x, uint32_t y)
{
    return 1 ^ ct_gt_u32(x, y);
}

int ct_ge_u32(uint32_t x, uint32_t y)
{
    return 1 ^ ct_lt_u32(x, y);
}

/* Signed comparisons */
/* Return 1 if condition is true, 0 otherwise */
int ct_isnonzero_s32(uint32_t x)
{
    return (x|-x)>>31;
}

int ct_iszero_s32(uint32_t x)
{
    return 1 ^ ct_isnonzero_s32(x);
}

int ct_neq_s32(uint32_t x, uint32_t y)
{
    return ((x-y)|(y-x))>>31;
}

int ct_eq_s32(uint32_t x, uint32_t y)
{
    return 1 ^ ct_neq_s32(x, y);
}

int ct_lt_s32(uint32_t x, uint32_t y)
{
    return (x^((x^(x-y))&(y^(x-y))))>>31;
}

int ct_gt_s32(uint32_t x, uint32_t y)
{
    return ct_lt_s32(y, x);
}

int ct_le_s32(uint32_t x, uint32_t y)
{
    return 1 ^ ct_gt_s32(x, y);
}

int ct_ge_s32(uint32_t x, uint32_t y)
{
    return 1 ^ ct_lt_s32(x, y);
}

/* Generate a mask: 0xFFFFFFFF if bit != 0, 0 otherwise */
uint32_t ct_mask_u32(uint32_t bit)
{
    return -ct_isnonzero_u32(bit);
}

/* Conditionally return x or y depending on whether bit is set */
/* Equivalent to: return bit ? x : y */
uint32_t ct_select_u32(uint32_t x, uint32_t y, uint32_t bit)
{
    uint32_t m = ct_mask_u32(bit);
    return (x&m) | (y&~m);
    /* return ((x^y)&m)^y; */
}
