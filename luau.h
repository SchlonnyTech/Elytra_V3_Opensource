/*
 * Luau.h
 * Owned and made by Schlonny
 * License: Schlonny Tech Shared Source License
 * Repository: https://github.com/SchlonnyTech/Elytra_V3_Opensource
 *
 * This software is proprietary. All rights reserved.
 *
 */

#ifndef ELYTRA_LUAU_H
#define ELYTRA_LUAU_H

#include <stddef.h>
#include <stdint.h>

#define LUA_TNIL 0           // no value
#define LUA_TBOOLEAN 1       // true/false
#define LUA_TLIGHTUSERDATA 2 // raw pointer, not GC'd
#define LUA_TNUMBER 3        // double
#define LUA_TVECTOR 4        // Luau-only: 3-float vector
#define LUA_TSTRING 5        // interned string (GCObject)
#define LUA_TTABLE 6         // hash/array table (GCObject)
#define LUA_TFUNCTION 7      // LClosure or CClosure (GCObject)
#define LUA_TUSERDATA 8      // full userdata with metatable (GCObject)
#define LUA_TTHREAD 9        // coroutine / lua_State (GCObject)

#define LUAI_MAXSTACK 8000                        // max stack slots per frame
#define LUA_REGISTRYINDEX (-LUAI_MAXSTACK - 1000) // = -9000  -> registry table
#define LUA_ENVIRONINDEX (-LUAI_MAXSTACK - 2000)  // = -10000 -> current env
#define LUA_GLOBALSINDEX (-LUAI_MAXSTACK - 2001)  // = -10001 -> _G table

#define RVA_LUA_PUSHSTRING 0x517420UL // lua_pushstring(L, s)
#define RVA_LUA_GETFIELD 0x5178f0UL   // lua_getfield(L, idx, k)
#define RVA_LUA_TYPE 0x5164a0UL       // lua_type(L, idx)
#define RVA_LUA_D_PCALL 0x523b40UL    // luaD_pcall: protected call (setjmp)
#define RVA_LUA_D_THROW 0x51c170UL    // luaD_throw: longjmp out with error
#define RVA_LUA_D_CALL 0x51c500UL     // luaD_call: unprotected call dispatch
#define RVA_LUA_D_PRECALL 0x5395c0UL  // luaD_precall: frame setup for a call
#define RVA_LUA_V_EXECUTE 0x52f1c0UL  // luaV_execute: bytecode interpreter loop

// funcs
typedef const char *(*lua_pushstring_t)(void *L, const char *s);
typedef int (*lua_getfield_t)(void *L, int idx, const char *k);
typedef int (*lua_type_t)(void *L, int idx);
typedef int (*lua_d_pcall_t)(void *L, void *func, long a3, int a4);
typedef int (*lua_pcall_t)(void *L, int nargs, int nresults, int msgh);

/* ------------------------------------------------------------------ */
/* lua_State layout                                                    */
/* ------------------------------------------------------------------ */

/* Confirmed by hooking lua_pushstring and dumping L:
 *
 *   +0x00 : 8-byte GC header (tt, marked, memcat, pad, ...)
 *   +0x08 : StkId top      — pointer to the next free stack slot
 *   +0x10 : StkId stack    — base of the value stack
 *   +0x18 : global_State*  — pointer into the process's main heap
 *   +0x28 : CallInfo*      — pointer into Luau heap
 *   +0x30 : StkId stack_last
 *   +0x38 : CallInfo* ci
 *   +0x40 : CallInfo* base_ci
 *   +0x48 : 0x0000000800000XXX — flags / counters
 *
 * The stack grows upward by 16 bytes per TValue.
 */

#define LUA_STATE_TOP_OFF 0x08   // StkId L->top
#define LUA_STATE_STACK_OFF 0x10 // StkId L->stack
#define LUA_STATE_L_G_OFF 0x18   // global_State *L->l_G

/* ------------------------------------------------------------------ */
/* TValue layout                                                       */
/* ------------------------------------------------------------------ */

/* Luau's TValue is 16 bytes on x86_64.
 *
 * Layout A (Lua 5.1/5.3):
 *   +0x00 : 8-byte value (GCObject * or double)
 *   +0x08 : uint32_t tt
 *   +0x0C : uint32_t pad
 *
 * Layout B (Luau 0.6xx = Luduvo):
 *   +0x00 : uint32_t tt
 *   +0x04 : uint32_t flags
 *   +0x08 : 8-byte value
 */

#define OFF_TVALUE_VALUE 0x00   // value field offset (A/B differ)
#define OFF_TVALUE_TT 0x0C      // type tag offset used by this build
#define TVALUE_SIZE 16          // sizeof(TValue)
#define TVALUE_TT_OFF_A 0x08    // tt offset, Layout A
#define TVALUE_TT_OFF_B 0x00    // tt offset, Layout B
#define TVALUE_VALUE_OFF_A 0x00 // value offset, Layout A
#define TVALUE_VALUE_OFF_B 0x08 // value offset, Layout B
#define LUA_TVALUE_TT_OFF 0xC   // alias used by callers

#define L_G_STATE_MAINTHREAD 0x00 // lua_State *mainthread
#define L_G_STATE_STRT 0x28       // Table *strt (string metatable) — tentative
#define L_G_STATE_TMNAME 0x60     // TString *tmname[TM_N] (metamethod names)
#define L_G_STATE_GT 0x38         // Table *gt (globals) — tentative

#endif /* ELYTRA_LUAU_H */
