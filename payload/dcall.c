/*
 * Elytra Module
 * Owned and made by Schlonny
 * License: Schlonny Tech Shared Source License
 * Repository: https://github.com/SchlonnyTech/Elytra_V3_Opensource
 *
 * This software is proprietary. All rights reserved.
 *
 */
#include "dcall.h"
#include "../luau.h"
#include "log.h"

extern uintptr_t g_base;

int dcall(void *L, void *func_slot, int nresults) {
  if (!g_base) {
    log_line("dcall: no base");
    return -1;
  }
  typedef void (*luaD_call_t)(void *, void *, int);
  luaD_call_t real = (luaD_call_t)(g_base + RVA_LUA_D_CALL);
  real(L, func_slot, nresults);
  return 0;
}

int dcall_with_strings(void *L, void *func_slot, const char **args, int nargs,
                       int nresults) {
  extern const char *(*g_orig_pushstring)(void *L, const char *s);
  if (!g_orig_pushstring)
    return -1;
  for (int i = 0; i < nargs; ++i)
    g_orig_pushstring(L, args[i]);
  return dcall(L, func_slot, nresults);
}
