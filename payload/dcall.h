/*
 * Elytra Module
 * Owned and made by Schlonny
 * License: Schlonny Tech Shared Source License
 * Repository: https://github.com/SchlonnyTech/Elytra_V3_Opensource
 *
 * This software is proprietary. All rights reserved.
 *
 */
#ifndef ELYTRA_DCALL_H
#define ELYTRA_DCALL_H

#include <stdint.h>

#define OFF_L_STATE_TOP 0x08       // L->top
#define OFF_L_STATE_STACK 0x10     // L->stack
#define OFF_L_STATE_CI 0x20        // L->ci
#define OFF_L_STATE_BASE 0x28      // L->base
#define OFF_L_STATE_STACKLAST 0x30 // L->stack_last
#define OFF_L_STATE_CI_BASE 0x38   // L->ci_base (first CallInfo)
#define OFF_L_STATE_NCI 0x40       // L->nci (allocated CallInfo count)
#define OFF_L_STATE_NCCALLS 0x4C   // L->nCcalls (C call depth)

#define OFF_CI_BASE 0x00     // CallInfo->base (StkId)
#define OFF_CI_FUNC 0x08     // CallInfo->func (StkId)
#define OFF_CI_TOP 0x10      // CallInfo->top (StkId)
#define OFF_CI_SAVEDPC 0x18  // CallInfo->savedpc (const Instruction*)
#define OFF_CI_NRESULTS 0x20 // CallInfo->nresults (int)
#define OFF_CI_SIZE 0x50     // sizeof(CallInfo)

#define OFF_CLOSURE_PROTO 0x10 // Closure->l.p / c.f (Proto* or lua_CFunction)
#define OFF_CLOSURE_IS_C_OFF 0x03 // Closure->c.tt / isC flag byte offset
#define OFF_CLOSURE_IS_C_BIT 0x04 // isC mask bit within that byte
#define OFF_C_CLOSURE_F 0x10      // CClosure->f (lua_CFunction)

#define OFF_PROTO_MAXSTACK 0x60 // Proto->maxstacksize (uint8)

#define OFF_TVALUE_VALUE 0x00 // TValue->value (GCObject*/void*/number)
#define OFF_TVALUE_TT 0x0C    // TValue->tt (int type tag)
#define TVALUE_STRIDE 16      // sizeof(TValue)

// Run `func_tvalue_slot` (a TValue on the stack) with `nargs` args
// Same as
//     luaD_call(L, func, nresults);
// Returns 0 on success, negative on error.
int dcall(void *L, void *func_tvalue_slot, int nresults);

// push `func`, push `nargs` strings from `args[]`, call.
int dcall_with_strings(void *L, void *func_tvalue_slot, const char **args,
                       int nargs, int nresults);

#endif
