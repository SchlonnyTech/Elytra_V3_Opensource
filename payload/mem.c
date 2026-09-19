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
#include "mem.h"

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int safe_read(uintptr_t addr, void *buf, size_t len) {
  int fd = open("/proc/self/mem", O_RDONLY);
  if (fd < 0)
    return -1;
  ssize_t n = pread(fd, buf, len, (off_t)addr);
  close(fd);
  return (n == (ssize_t)len) ? 0 : -1;
}

uintptr_t find_self_base(void) {
  char exe[512];
  ssize_t n = readlink("/proc/self/exe", exe, sizeof(exe) - 1);
  if (n <= 0)
    return 0;
  exe[n] = '\0';

  int fd = open("/proc/self/maps", O_RDONLY);
  if (fd < 0)
    return 0;
  char buf[16384];
  ssize_t r = read(fd, buf, sizeof(buf) - 1);
  close(fd);
  if (r <= 0)
    return 0;
  buf[r] = '\0';

  char *line = buf;
  while (line && *line) {
    char *nl = line;
    while (*nl && *nl != '\n')
      ++nl;
    char saved = *nl;
    *nl = '\0';

    if (strstr(line, exe) && strstr(line, "r--p")) {
      uintptr_t v = 0;
      for (char *p = line; *p && *p != '-'; ++p) {
        int d;
        if (*p >= '0' && *p <= '9')
          d = *p - '0';
        else if (*p >= 'a' && *p <= 'f')
          d = *p - 'a' + 10;
        else
          break;
        v = (v << 4) | (uintptr_t)d;
      }
      return v;
    }
    if (saved == '\0')
      break;
    line = nl + 1;
  }
  return 0;
}
