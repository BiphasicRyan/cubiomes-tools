#ifndef PARSING_H
#define PARSING_H

#include <stdint.h>
#include <inttypes.h>

uint64_t parse_u64(const char *s);
int parse_i32_nonneg(const char *s, const char *what);

#endif
