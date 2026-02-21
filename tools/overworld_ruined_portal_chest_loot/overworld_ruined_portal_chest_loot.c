#include <stdio.h>
#include <stdint.h>
#include "parsing.h"

int main(int argc, char *argv[])
{
    SearchConfig config;
    if (!parse_args(argc, argv, &config)) {
        return 1;
    }

    printf("Search parameters:\n");
    printf("  min_egas:     %d\n", config.min_egas);
    printf("  max_distance: %d\n", config.max_distance);
    printf("  start_seed:   %llu\n", (unsigned long long)config.start_seed);
    printf("  end_seed:     %llu\n", (unsigned long long)config.end_seed);
    printf("  threads:      %d\n", config.threads);
    printf("  fast_mode:    %s\n", config.fast_mode ? "yes" : "no");
    printf("\n");

    for (uint64_t seed = config.start_seed; seed <= config.end_seed; seed++) {
        printf("Checking seed %llu\n", (unsigned long long)seed);
    }

    printf("\nDone. Checked %llu seeds.\n",
           (unsigned long long)(config.end_seed - config.start_seed + 1));

    return 0;
}
