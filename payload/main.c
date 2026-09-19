/*
 * Elytra Module
 * Owned and made by Schlonny
 * License: Schlonny Tech Shared Source License
 * Repository: https://github.com/SchlonnyTech/Elytra_V3_Opensource
 *
 * This software is proprietary. All rights reserved.
 *
 */
#include "hook.h"
#include "log.h"
#include "luastate.h"
#include "mem.h"
#include "pipe.h"

__attribute__((constructor)) static void elytra_payload(void) {
  log_open();
  log_line("Elytra v3 payload");

  uintptr_t base = find_self_base();
  g_base = base;
  log_hex("base", base);
  if (!base)
    return;

  hook_install(base);

  // starts pipe and it waits for g_L to be set.
  pipe_start();
}
