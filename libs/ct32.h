/*
Constant-time integer comparisons
 
Written in 2014 by Samuel Neves <sneves@dei.uc.pt>
 
To the extent possible under law, the author(s) have dedicated all copyright
and related and neighboring rights to this software to the public domain
worldwide. This software is distributed without any warranty.
 
You should have received a copy of the CC0 Public Domain Dedication along with
this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef CT32_H
#define CT32_H

#include <stdint.h>

int ct_isnonzero_u32(uint32_t x);
int ct_iszero_u32(uint32_t x);
int ct_neq_u32(uint32_t x, uint32_t y);
int ct_eq_u32(uint32_t x, uint32_t y);
int ct_lt_u32(uint32_t x, uint32_t y);
int ct_gt_u32(uint32_t x, uint32_t y);
int ct_le_u32(uint32_t x, uint32_t y);
int ct_ge_u32(uint32_t x, uint32_t y);

int ct_isnonzero_s32(uint32_t x);
int ct_iszero_s32(uint32_t x);
int ct_neq_s32(uint32_t x, uint32_t y);
int ct_eq_s32(uint32_t x, uint32_t y);
int ct_lt_s32(uint32_t x, uint32_t y);
int ct_gt_s32(uint32_t x, uint32_t y);
int ct_le_s32(uint32_t x, uint32_t y);
int ct_ge_s32(uint32_t x, uint32_t y);

uint32_t ct_mask_u32(uint32_t bit);
uint32_t ct_select_u32(uint32_t x, uint32_t y, uint32_t bit);

unsigned char invariant_time_lookup(const unsigned char *array, uint32_t length, uint32_t index);

#endif
