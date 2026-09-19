/*
 * Elytra Module
 * Owned and made by Schlonny
 * License: Schlonny Tech Shared Source License
 * Repository: https://github.com/SchlonnyTech/Elytra_V3_Opensource
 *
 * This software is proprietary. All rights reserved.
 *
 */
#ifndef ELYTRA_PIPE_H
#define ELYTRA_PIPE_H

#include <stdint.h>

#define ELYTRA_PIPE_PATH "/tmp/elytra_pipe"

// reads lines from ELYTRA_PIPE_PATH
// and runs each via luau_run on the lua_State stored in g_L.
void pipe_start(void);

void pipe_stop(void);

// The lua_State the pipe should execute against.  Set this once
// during hook init; the reader waits until it is nonzero.
extern void *g_L;

#endif
