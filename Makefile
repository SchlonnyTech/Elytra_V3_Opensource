# Elytra v3 — made by .schlonny

CC      := gcc
CFLAGS_COMMON := -O2 -Wall -Wextra -std=c11
PAY_CFLAGS    := $(CFLAGS_COMMON) -fPIC -Isrc -I.

INJ_SRC := injector.c

PAY_SRCS := payload/main.c payload/hook.c payload/luastate.c \
            payload/log.c  payload/mem.c  payload/dcall.c  payload/pipe.c
PAY_OUT  := payload.so
PAY_HDRS := payload/hook.h payload/luastate.h payload/log.h payload/mem.h luau.h

.PHONY: all injector dumper payload run dump clean

all: injector dumper payload

injector: $(INJ_SRC)
	$(CC) $(CFLAGS_COMMON) -o injector $(INJ_SRC) -lX11 -ldl

payload: $(PAY_OUT)

$(PAY_OUT): $(PAY_SRCS) $(PAY_HDRS)
	$(CC) $(PAY_CFLAGS) -shared -o $@ $(PAY_SRCS) -lpthread

run: injector payload
	sudo ./injector

clean:
	rm -f injector $(PAY_OUT) offsets.h


EXE_SRC := executor.c
EXE_OUT := executor

all: injector payload executor

executor: $(EXE_SRC)
	$(CC) $(CFLAGS_COMMON) -o $(EXE_OUT) $(EXE_SRC)
