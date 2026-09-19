/*
 * Elytra Module
 * Owned and made by Schlonny
 * License: Schlonny Tech Shared Source License
 * Repository: https://github.com/SchlonnyTech/Elytra_V3_Opensource
 *
 * This software is proprietary. All rights reserved.
 *
 */
#ifndef ELYTRA_LOG_H
#define ELYTRA_LOG_H

#include <stdint.h>

void log_open(void);
void log_str(const char *s);
void log_line(const char *s);
void log_hex(const char *label, uintptr_t v);
void log_int(const char *label, int v);
void log_fmt(const char *fmt, ...);

#endif
