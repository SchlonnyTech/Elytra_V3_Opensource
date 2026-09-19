/*
 * Elytra Module
 * Owned and made by Schlonny
 * License: Schlonny Tech Shared Source License
 * Repository: https://github.com/SchlonnyTech/Elytra_V3_Opensource
 *
 * This software is proprietary. All rights reserved.
 *
 */
#ifndef ELYTRA_LUASTATE_H
#define ELYTRA_LUASTATE_H

#include <stdint.h>

extern uintptr_t g_base;

uint64_t luau_get_top(void *L);
void luau_set_top(void *L, uint64_t top);
void *luau_get_l_G(void *L);
void luau_push_table(void *L, uint64_t table_ptr);
int looks_like_table(uint64_t p);
void luau_dump_l_G(void *L);
void luau_run(void *L, const char *code);

#endif
