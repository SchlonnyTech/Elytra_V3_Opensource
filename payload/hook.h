/*
 * Elytra Module
 * Owned and made by Schlonny
 * License: Schlonny Tech Shared Source License
 * Repository: https://github.com/SchlonnyTech/Elytra_V3_Opensource
 *
 * This software is proprietary. All rights reserved.
 *
 */
#ifndef ELYTRA_HOOK_H
#define ELYTRA_HOOK_H

#include <stdint.h>

int hook_install(uintptr_t base);
extern const char *(*g_orig_pushstring)(void *L, const char *s);

#endif
