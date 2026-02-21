#ifndef PARSING_H
#define PARSING_H

#include <stdint.h>

typedef struct {
    int min_egas;
    int max_distance;
    uint64_t start_seed;
    uint64_t end_seed;
    int threads;
    int fast_mode;
    int mc_version;
} SearchConfig;

int parse_args(int argc, char *argv[], SearchConfig *config);
void print_usage(const char *program_name);

#endif
