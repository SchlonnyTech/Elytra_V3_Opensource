/*
 * Elytra Module
 * Owned and made by Schlonny
 * License: Schlonny Tech Shared Source License
 * Repository: https://github.com/SchlonnyTech/Elytra_V3_Opensource
 *
 * This software is proprietary. All rights reserved.
 *
 */
#ifndef ELYTRA_MEM_H
#define ELYTRA_MEM_H

#include <stddef.h>
#include <stdint.h>

int safe_read(uintptr_t addr, void *buf, size_t len);
uintptr_t find_self_base(void);

#endif
