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
#include "log.h"

#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define LOG_TMP "/tmp/elytra_payload.log"

static int g_fd = -1;

void log_open(void) {
  g_fd = open(LOG_TMP, O_WRONLY | O_CREAT | O_TRUNC, 0644);
}

void log_str(const char *s) {
  if (g_fd < 0 || !s)
    return;
  size_t n = strlen(s), off = 0;
  while (off < n) {
    ssize_t w = write(g_fd, s + off, n - off);
    if (w <= 0)
      return;
    off += (size_t)w;
  }
}

void log_line(const char *s) {
  log_str(s);
  if (g_fd >= 0)
    (void)write(g_fd, "\n", 1);
}

void log_hex(const char *label, uintptr_t v) {
  if (g_fd < 0)
    return;
  char buf[96];
  int n = snprintf(buf, sizeof(buf), "%s: 0x%lx\n", label, (unsigned long)v);
  if (n > 0)
    (void)write(g_fd, buf, (size_t)n);
}

void log_int(const char *label, int v) {
  if (g_fd < 0)
    return;
  char buf[64];
  int n = snprintf(buf, sizeof(buf), "%s%d\n", label, v);
  if (n > 0)
    (void)write(g_fd, buf, (size_t)n);
}

void log_fmt(const char *fmt, ...) {
  if (g_fd < 0)
    return;
  char buf[256];
  va_list ap;
  va_start(ap, fmt);
  int n = vsnprintf(buf, sizeof(buf), fmt, ap);
  va_end(ap);
  if (n > 0)
    (void)write(g_fd, buf, (size_t)n);
}
