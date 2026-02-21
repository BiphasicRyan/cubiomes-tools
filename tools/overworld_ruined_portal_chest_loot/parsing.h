#ifndef PARSING_H
#define PARSING_H

#include <stdint.h>

typedef struct {
    int min_egas;           // minimum enchanted golden apples required
    int max_distance;       // maximum block distance from (0,0)
    uint64_t start_seed;    // start of seed range
    uint64_t end_seed;      // end of seed range (inclusive)
    int num_threads;        // number of threads to use
    int mc_version;         // minecraft version constant
    int skip_biome_check;   // skip biome viability check for speed
} SearchParams;

int parse_args(int argc, char **argv, SearchParams *params);
void print_usage(const char *prog_name);

#endif // PARSING_H
