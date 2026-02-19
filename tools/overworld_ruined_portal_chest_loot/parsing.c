#include "parsing.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Parses an unsigned 64-bit integer from a string 
// Supports decimal and hexadecimal
// Returns the parsed value or exits with error if invalid
uint64_t parse_u64(const char *s)
{
    errno = 0;
    char *end = NULL;
    unsigned long long v = strtoull(s, &end, 0); // supports decimal and 0x...
    if (errno != 0 || end == s || *end != '\0') {
        fprintf(stderr, "Invalid seed: '%s'\n", s);
        exit(2);
    }
    return (uint64_t)v;
}

// Parses a non-negative 32-bit integer from a string 
// Decimal only
// Returns the parsed value or exits with error if invalid or out of range
int parse_i32_nonneg(const char *s, const char *what)
{
    errno = 0;
    char *end = NULL;
    long v = strtol(s, &end, 10);
    if (errno != 0 || end == s || *end != '\0' || v < 0 || v > 2000000) {
        fprintf(stderr, "Invalid %s: '%s'\n", what, s);
        exit(2);
    }
    return (int)v;
}
