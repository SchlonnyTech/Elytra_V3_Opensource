/*
 * Elytra Module
 * Owned and made by Schlonny
 * License: Schlonny Tech Shared Source License
 * Repository: https://github.com/SchlonnyTech/Elytra_V3_Opensource
 *
 * This software is proprietary. All rights reserved.
 *
 */
#define _GNU_SOURCE
#include "hook.h"
#include "../luau.h"
#include "dcall.h"
#include "log.h"
#include "luastate.h"
#include "mem.h"

#include <dlfcn.h>
#include <stdio.h>
#include <string.h>

const char *(*g_orig_pushstring)(void *L, const char *s) = NULL;

static int g_done = 0;

static const char *FUNCHOOK_PATHS[] = { // too lazy to automate since johnluduvo is an asshole
    "/home/schlonny/Documents/development/Exploit-Software/Elytra-V3/module/"
    "src/Injector/lnx/vendor/build/libfunchook.so.2",
    "libfunchook.so.2", NULL};

static uint64_t find_gt(void *L, uint64_t orig_top) {
  lua_getfield_t getfield = (lua_getfield_t)(g_base + RVA_LUA_GETFIELD);

  size_t try_offs[] = {0x58,  0x90,  0x158, 0x1d8, 0x28,  0x38,
                       0x110, 0x258, 0x60,  0x88,  0x1a0, 0x300};
  for (size_t i = 0; i < sizeof(try_offs) / sizeof(try_offs[0]); ++i) {
    uint64_t cand = 0;
    if (safe_read((uintptr_t)L + try_offs[i], &cand, 8) != 0)
      continue;
    if (!looks_like_table(cand))
      continue;

    luau_set_top(L, orig_top);
    luau_push_table(L, cand);
    getfield(L, -1, "print"); // for now just prints the code u put in, blame johnluduvo for me not fixing it

    uint64_t now_top = luau_get_top(L);
    if (now_top != orig_top + 2 * TVALUE_SIZE) {
      luau_set_top(L, orig_top);
      continue;
    }

    uint32_t tt = 0;
    safe_read(now_top - TVALUE_SIZE + OFF_TVALUE_TT, &tt, 4);
    if (tt == LUA_TFUNCTION) {
      log_fmt("gt found at L+0x%02zx = 0x%lx\n", try_offs[i],
              (unsigned long)cand);
      luau_set_top(L, orig_top);
      return cand;
    }
    luau_set_top(L, orig_top);
  }
  return 0;
}

static const char *hook_pushstring(void *L, const char *s) {
  if (!g_done && s && strcmp(s, "CashLabel") == 0) { // yea i played that scraper game to test
    g_done = 1;
    log_line("--- pushstring hook fired on CashLabel ---");
    log_hex("L", (uintptr_t)L);

    if (!g_base)
      goto out;

    uint64_t orig_top = luau_get_top(L);
    uint64_t gt = find_gt(L, orig_top);

    if (!gt) {
      log_line("gt not found");
      goto out;
    }

    /* Stack after find_gt is back to orig_top.  Rebuild [gt, print]. */
    luau_set_top(L, orig_top);
    luau_push_table(L, gt);

    lua_getfield_t getfield = (lua_getfield_t)(g_base + RVA_LUA_GETFIELD);
    getfield(L, -1, "print");

    // Now stack: [gt, print]

    if (g_orig_pushstring)
      g_orig_pushstring(L, "print('Hello from Elytra v3')"); // just prints it for now

    // Stack: [gt, print, code]
    uint64_t top = luau_get_top(L);
    void *func_slot = (void *)(top - 2 * TVALUE_SIZE);

    log_hex("func_slot", (uintptr_t)func_slot);

    int rc = dcall(L, func_slot, 0);
    log_int("dcall rc", rc);

    luau_set_top(L, orig_top);
  }

out:
  if (g_orig_pushstring)
    return g_orig_pushstring(L, s);
  return s;
}

typedef void *(*funchook_create_t)(void);
typedef int (*funchook_prepare_t)(void *fh, void **target, void *hook);
typedef int (*funchook_install_t)(void *fh, int flags);

int hook_install(uintptr_t base) {
  void *fh = NULL;
  for (int i = 0; FUNCHOOK_PATHS[i]; ++i) {
    fh = dlopen(FUNCHOOK_PATHS[i], RTLD_NOW | RTLD_GLOBAL);
    if (fh) {
      log_str("loaded: ");
      log_line(FUNCHOOK_PATHS[i]);
      break;
    }
  }
  if (!fh) {
    log_line("dlopen failed");
    return -1;
  }

  funchook_create_t fc = (funchook_create_t)dlsym(fh, "funchook_create");
  funchook_prepare_t fp = (funchook_prepare_t)dlsym(fh, "funchook_prepare");
  funchook_install_t fi = (funchook_install_t)dlsym(fh, "funchook_install");
  if (!fc || !fp || !fi) {
    log_line("dlsym failed");
    return -1;
  }

  void *hook = fc();
  if (!hook) {
    log_line("funchook_create failed");
    return -1;
  }

  void *target = (void *)(base + RVA_LUA_PUSHSTRING);
  if (fp(hook, &target, (void *)hook_pushstring) != 0) {
    log_line("funchook_prepare pushstring failed");
    return -1;
  }
  g_orig_pushstring = (const char *(*)(void *, const char *))target;
  if (fi(hook, 0) != 0) {
    log_line("funchook_install pushstring failed");
    return -1;
  }

  log_line("hook installed on lua_pushstring");
  return 0;
}
