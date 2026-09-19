/*
 * Elytra Executor
 * Owned and made by Schlonny
 * License: Schlonny Tech Shared Source License
 * Repository: https://github.com/SchlonnyTech/Elytra_V3_Opensource
 *
 * This software is proprietary. All rights reserved.
 *
 */

#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define PIPE_PATH "/tmp/elytra_pipe"

static volatile sig_atomic_t g_running = 1;

static void on_sigint(int sig) {
  (void)sig;
  g_running = 0;
}

static int open_pipe_writer(void) {
  int fd = open(PIPE_PATH, O_WRONLY | O_NONBLOCK);
  if (fd >= 0) {
    int fl = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, fl & ~O_NONBLOCK);
    return fd;
  }

  if (errno == ENXIO) {
    fprintf(stderr, "[executor] waiting for payload to open %s...\n",
            PIPE_PATH);
    fd = open(PIPE_PATH, O_WRONLY);
    return fd;
  }

  perror("open");
  return -1;
}

static char *read_line(void) {
  static char buf[8192];
  ssize_t n = read(STDIN_FILENO, buf, sizeof(buf) - 1);
  if (n <= 0)
    return NULL;
  buf[n] = '\0';
  size_t len = strlen(buf);
  while (len > 0 && (buf[len - 1] == '\n' || buf[len - 1] == '\r'))
    buf[--len] = '\0';
  return buf;
}

int main(int argc, char **argv) {
  signal(SIGINT, on_sigint);
  signal(SIGPIPE, SIG_IGN);

  printf("========================================\n");
  printf(" Elytra executor — pipe client\n");
  printf("========================================\n");
  printf("Pipe: %s\n", PIPE_PATH);

  if (argc > 1) {
    int fd = open_pipe_writer();
    if (fd < 0) {
      fprintf(stderr, "[executor] cannot open pipe\n");
      return 1;
    }
    dprintf(fd, "%s\n", argv[1]);
    close(fd);
    printf("[executor] sent: %s\n", argv[1]);
    return 0;
  }

  int fd = open_pipe_writer();
  if (fd < 0) {
    fprintf(stderr,
            "[executor] cannot open %s for writing.  "
            "Is the payload running and listening?\n",
            PIPE_PATH);
    return 1;
  }

  printf("[executor] connected.  Type Luau code.  Ctrl-D to exit.\n");

  while (g_running) {
    fputs("elytra> ", stdout);
    fflush(stdout);

    char *line = read_line();
    if (!line) {
      printf("\n[executor] EOF — exiting\n");
      break;
    }
    if (line[0] == '\0')
      continue;

    size_t len = strlen(line);
    ssize_t w = write(fd, line, len);
    if (w != (ssize_t)len) {
      perror("write");
      fprintf(stderr, "[executor] write failed, exiting\n");
      break;
    }
    if (write(fd, "\n", 1) != 1) {
      perror("write newline");
      break;
    }
    usleep(20000);
  }

  close(fd);
  return 0;
}
