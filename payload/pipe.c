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
#include "pipe.h"
#include "log.h"
#include "luastate.h"

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

void *g_L = NULL;

static pthread_t g_thread;
static volatile int g_running = 0;

static void *pipe_reader(void *arg) {
  (void)arg;

  struct stat st;
  if (stat(ELYTRA_PIPE_PATH, &st) != 0) {
    if (mkfifo(ELYTRA_PIPE_PATH, 0666) != 0) {
      log_fmt("pipe: mkfifo failed: %s\n", strerror(errno));
      return NULL;
    }
    log_fmt("pipe: created %s\n", ELYTRA_PIPE_PATH);
  } else {
    log_fmt("pipe: reusing existing %s\n", ELYTRA_PIPE_PATH);
  }

  while (g_running) {
    int fd = open(ELYTRA_PIPE_PATH, O_RDONLY);
    if (fd < 0) {
      log_fmt("pipe: open read failed: %s\n", strerror(errno));
      usleep(500000);
      continue;
    }

    log_line("pipe: writer connected");

    char line[4096];
    size_t len = 0;
    char c;
    ssize_t r;

    while (g_running && (r = read(fd, &c, 1)) == 1) {
      if (c == '\n') {
        line[len] = '\0';
        if (len > 0) {
          log_fmt("pipe: exec> %s\n", line);
          if (g_L) {
            luau_run(g_L, line);
          } else {
            log_line("pipe: g_L is NULL, cannot run");
          }
        }
        len = 0;
      } else if (len + 1 < sizeof(line)) {
        line[len++] = c;
      } else {
        len = 0;
      }
    }

    close(fd);
    log_line("pipe: writer disconnected");
    usleep(100000);
  }

  return NULL;
}

void pipe_start(void) {
  if (g_running)
    return;
  g_running = 1;

  int err = pthread_create(&g_thread, NULL, pipe_reader, NULL);
  if (err != 0) {
    log_fmt("pipe: pthread_create failed: %d\n", err);
    g_running = 0;
    return;
  }
  pthread_detach(g_thread);
  log_line("pipe: reader thread started");
}

void pipe_stop(void) { g_running = 0; }
