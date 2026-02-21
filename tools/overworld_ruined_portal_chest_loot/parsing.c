#include "parsing.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_usage(const char *program_name)
{
    printf("Usage: %s <min_egas> <max_distance> [options]\n", program_name);
    printf("\n");
    printf("Required arguments:\n");
    printf("  <min_egas>       Minimum enchanted golden apples in a single chest\n");
    printf("  <max_distance>   Maximum block distance from (0,0) to search\n");
    printf("\n");
    printf("Optional arguments:\n");
    printf("  -s START         Start seed (default: 0)\n");
    printf("  -e END           End seed inclusive (default: 2^48 - 1)\n");
    printf("  -t THREADS       Number of threads (default: 1)\n");
    printf("  --fast           Skip biome viability check\n");
    printf("\n");
    printf("Examples:\n");
    printf("  %s 1 500 -s 0 -e 100000\n", program_name);
    printf("  %s 2 1000 -s 0 -e 100000 -t 4 --fast\n", program_name);
}

int parse_args(int argc, char *argv[], SearchConfig *config)
{
    // Defaults
    config->min_egas = 0;
    config->max_distance = 0;
    config->start_seed = 0;
    config->end_seed = (1ULL << 48) - 1;
    config->threads = 1;
    config->fast_mode = 0;
    config->mc_version = 0;

    if (argc < 3) {
        print_usage(argv[0]);
        return 0;
    }

    // Required positional arguments
    config->min_egas = atoi(argv[1]);
    config->max_distance = atoi(argv[2]);

    if (config->min_egas <= 0) {
        fprintf(stderr, "Error: min_egas must be a positive integer\n");
        return 0;
    }
    if (config->max_distance <= 0) {
        fprintf(stderr, "Error: max_distance must be a positive integer\n");
        return 0;
    }

    // Optional arguments
    for (int i = 3; i < argc; i++) {
        if (strcmp(argv[i], "-s") == 0 && i + 1 < argc) {
            config->start_seed = strtoull(argv[++i], NULL, 10);
        } else if (strcmp(argv[i], "-e") == 0 && i + 1 < argc) {
            config->end_seed = strtoull(argv[++i], NULL, 10);
        } else if (strcmp(argv[i], "-t") == 0 && i + 1 < argc) {
            config->threads = atoi(argv[++i]);
            if (config->threads <= 0) {
                fprintf(stderr, "Error: threads must be a positive integer\n");
                return 0;
            }
        } else if (strcmp(argv[i], "--fast") == 0) {
            config->fast_mode = 1;
        } else {
            fprintf(stderr, "Error: unknown argument '%s'\n", argv[i]);
            print_usage(argv[0]);
            return 0;
        }
    }

    if (config->start_seed > config->end_seed) {
        fprintf(stderr, "Error: start seed (%llu) must be <= end seed (%llu)\n",
                (unsigned long long)config->start_seed,
                (unsigned long long)config->end_seed);
        return 0;
    }

    return 1;
}
