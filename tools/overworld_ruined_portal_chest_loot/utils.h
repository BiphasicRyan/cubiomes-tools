#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <stdlib.h>

static inline int64_t sqr64(int64_t a) { return a * a; }

uint64_t parse_u64(const char *s);
int parse_i32_nonneg(const char *s, const char *what);

#endif
