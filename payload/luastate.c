/*
 * Elytra Module
 * Owned and made by Schlonny
 * License: Schlonny Tech Shared Source License
 * Repository: https://github.com/SchlonnyTech/Elytra_V3_Opensource
 *
 * This software is proprietary. All rights reserved.
 *
 */
#include "luastate.h"
#include "../luau.h"
#include "dcall.h"
#include "log.h"
#include "mem.h"

#include <string.h>

uintptr_t g_base = 0;

uint64_t luau_get_top(void *L) {
  uint64_t top = 0;
  memcpy(&top, (void *)((uintptr_t)L + OFF_L_STATE_TOP), sizeof(top));
  return top;
}

void luau_set_top(void *L, uint64_t top) {
  memcpy((void *)((uintptr_t)L + OFF_L_STATE_TOP), &top, sizeof(top));
}

void *luau_get_l_G(void *L) {
  void *g = NULL;
  memcpy(&g, (void *)((uintptr_t)L + LUA_STATE_L_G_OFF), sizeof(g));
  return g;
}

// TValue layout for this build (from lua_pushstring disasm):
//   +0x00  value (8 bytes)
//   +0x08  extra (4 bytes, usually 0)
//   +0x0C  tt (4 bytes, LUA_T*)
void luau_push_table(void *L, uint64_t table_ptr) {
  uint64_t top = luau_get_top(L);
  uint64_t value = table_ptr;
  uint32_t extra = 0;
  uint32_t tt = LUA_TTABLE;
  memcpy((void *)top, &value, 8);
  memcpy((void *)(top + 0x08), &extra, 4);
  memcpy((void *)(top + 0x0C), &tt, 4);
  luau_set_top(L, top + TVALUE_SIZE);
}

int looks_like_table(uint64_t p) {
  if (p < 0x550000000000ULL || p >= 0x800000000000ULL)
    return 0;
  uint8_t hdr[2] = {0};
  if (safe_read(p, hdr, 2) != 0)
    return 0;
  if (hdr[0] != LUA_TTABLE)
    return 0;
  if (hdr[1] > 8)
    return 0;
  return 1;
}

void luau_dump_l_G(void *L) {
  void *l_G = luau_get_l_G(L);
  log_hex("l_G", (uintptr_t)l_G);
  if (!l_G)
    return;

  uint8_t buf[0x80];
  if (safe_read((uintptr_t)l_G, buf, sizeof(buf)) != 0) {
    log_line("cannot read l_G");
    return;
  }
  for (size_t off = 0; off + 8 <= sizeof(buf); off += 8) {
    uint64_t p = 0;
    memcpy(&p, buf + off, 8);
    uint8_t b0 = 0;
    if (p >= 0x550000000000ULL && p <= 0x800000000000ULL)
      safe_read((uintptr_t)p, &b0, 1);
    log_fmt("l_G+0x%02zx = 0x%016lx  byte0=0x%02x\n", off, (unsigned long)p,
            b0);
  }
}

// luau_run is now a thin wrapper around dcall.  It pushes gt, gets
// `print`, pushes the code string, and calls dcall.
void luau_run(void *L, const char *code) {
  if (!g_base) {
    log_line("luau_run: no base");
    return;
  }

  lua_getfield_t getfield = (lua_getfield_t)(g_base + RVA_LUA_GETFIELD);
  lua_pushstring_t push = (lua_pushstring_t)(g_base + RVA_LUA_PUSHSTRING);
  lua_type_t ltype = (lua_type_t)(g_base + RVA_LUA_TYPE);

  uint64_t orig_top = luau_get_top(L);

  // find gt
  uint64_t gt = 0;
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
    getfield(L, -1, "print");

    uint64_t now_top = luau_get_top(L);
    if (now_top != orig_top + 2 * TVALUE_SIZE) {
      luau_set_top(L, orig_top);
      continue;
    }
    uint32_t tt = 0;
    safe_read(now_top - TVALUE_SIZE + OFF_TVALUE_TT, &tt, 4);
    if (tt == LUA_TFUNCTION) {
      gt = cand;
      break;
    }
    luau_set_top(L, orig_top);
  }

  if (!gt) {
    log_line("luau_run: gt not found");
    return;
  }

  // find the compiler
  const char *compilers[] = {"loadstring", "load", "dofile", "require", NULL};
  const char *chosen = NULL;

  for (int i = 0; compilers[i]; ++i) {
    luau_set_top(L, orig_top);
    luau_push_table(L, gt);
    getfield(L, -1, compilers[i]);
    if (ltype(L, -1) == LUA_TFUNCTION) {
      chosen = compilers[i];
      break;
    }
  }

  if (!chosen) {
    log_line(
        "luau_run: no loadstring/load/dofile in gt — falling back to print");
    luau_set_top(L, orig_top);
    luau_push_table(L, gt);
    getfield(L, -1, "print");
    push(L, code);
    uint64_t top = luau_get_top(L);
    dcall(L, (void *)(top - 2 * TVALUE_SIZE), 0);
    luau_set_top(L, orig_top);
    return;
  }

  log_fmt("luau_run: using %s\n", chosen);

  // Stack: [gt, compiler]
  // Push the code string as the argument.
  push(L, code);

  // Call compiler(code) with 1 result.
  uint64_t top = luau_get_top(L);
  void *compiler_slot = (void *)(top - 2 * TVALUE_SIZE);
  dcall(L, compiler_slot, 1); // nresults = 1

  // Stack: [gt, closure_or_nil]
  uint64_t now_top = luau_get_top(L);
  uint32_t tt = 0;
  safe_read(now_top - TVALUE_SIZE + OFF_TVALUE_TT, &tt, 4);
  log_fmt("loadstring returned tt=%u\n", tt);

  if (tt != LUA_TFUNCTION) {
    log_line("luau_run: compiler did not return a function");
    luau_set_top(L, orig_top);
    return;
  }

  // Call the resulting closure with 0 args.
  void *closure_slot = (void *)(now_top - TVALUE_SIZE);
  dcall(L, closure_slot, 0);

  luau_set_top(L, orig_top);
}
