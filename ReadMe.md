# Elytra V3

<p align="center">
  <img src="https://img.shields.io/badge/License-Schlonny%20Tech%20Shared%20Source-blue" alt="License">
  <img src="https://img.shields.io/badge/Platform-Linux-informational" alt="Platform">
  <img src="https://img.shields.io/badge/Language-C99%20%7C%20Luau-lightgrey" alt="Language">
</p>

---

## Overview

Elytra V3 is an out-of-process Luau execution bridge for x86_64 Linux targets.
It attaches a payload to a running Luau process, resolves the VM's internal
API by RVA, and exposes a FIFO-based command channel so an external client can
submit Luau source and have it run inside the host's lua_State.

The project ships two components:

- payload — a shared object loaded into the target process. It locates
  lua_State, hooks into the VM, and starts a background reader thread.
- executor — a small CLI client that talks to the payload over a named pipe
  (/tmp/elytra_pipe) and forwards Luau source to it.

## Features

- Direct luaD_call dispatch, no reliance on public Lua C API entry points.
- Runtime RVA resolution — offsets are stored as constants and rebased against
  the module base at init.
- FIFO command channel, so no network stack or extra dependencies are needed.
- Thread-safe reader runs alongside the host's own Lua activity.
- Modular headers (dcall.h, luastate.h, luau.h, pipe.h) so offsets and layouts
  are isolated from the calling code.

## Layout

- include/ — public headers (luau.h, luastate.h, dcall.h, pipe.h)
- src/dcall.c — luaD_call wrapper + string-arg convenience call
- src/luastate.c — lua_State accessors, table scanning, luau_run
- src/pipe.c — FIFO reader thread
- tools/executor.c — standalone pipe client
- README.md

## Requirements

- x86_64 Linux
- GCC or Clang with C99 support
- pthreads
- A Luau build matching the RVAs in include/luau.h

## Build

gcc -O2 -fPIC -shared src/*.c -o payload.so -lpthread
gcc -O2 tools/executor.c -o executor

## Usage

1. Inject payload.so into a running Luau process. On load the constructor
   creates /tmp/elytra_pipe and starts the reader thread.
2. From a shell, run the client:

./executor 'print("hello from elytra")'

Or start it interactively:

./executor
elytra> print(1 + 1)
elytra> game.Players.LocalPlayer.Name

Each line you type is written to the FIFO and executed inside the host's
lua_State on the payload side.

## Offsets

All hard-coded RVAs and structure offsets live in include/luau.h and
include/dcall.h. If you target a different Luau build, update:

- RVA_LUA_PUSHSTRING, RVA_LUA_GETFIELD, RVA_LUA_TYPE, RVA_LUA_D_CALL, and the
  other RVA_LUA_* constants.
- LUA_STATE_*_OFF for lua_State field positions.
- OFF_TVALUE_* and TVALUE_SIZE for the TValue layout.
- OFF_L_STATE_*, OFF_CI_*, OFF_CLOSURE_*, OFF_PROTO_* for the internal VM
  structures used by dcall.

The TValue layout comment in luau.h documents the two common arrangements;
the constants are already set for the build these were derived from.

## Protocol

The FIFO is line-delimited. Each newline terminates one Luau chunk. Empty
lines are ignored. The payload reads one byte at a time, accumulates into a
4 KiB buffer, and dispatches to luau_run when a newline arrives.

## License

Schlonny Tech Shared Source License. See the header of luau.h for the
canonical notice. All rights reserved.
