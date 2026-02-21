#include "parsing.h"

#include "generator.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

void print_usage(const char *prog_name)
{
    fprintf(stderr,
        "Usage: %s <min_egas> <max_distance> [options]\n"
        "\n"
        "Search for Minecraft seeds with ruined portals near origin (0,0)\n"
        "whose chests contain enchanted golden apples (EGAs).\n"
        "\n"
        "Required arguments:\n"
        "  min_egas       Minimum total EGAs across portal chests within range\n"
        "  max_distance   Maximum block distance from (0,0)\n"
        "\n"
        "Optional arguments:\n"
        "  -s START       Start seed (default: 0)\n"
        "  -e END         End seed inclusive (default: 2^48 - 1)\n"
        "  -t THREADS     Number of threads (default: 1)\n"
        "  -v VERSION     Minecraft version string e.g. 1.21 (default: 1.21)\n"
        "  --fast         Skip biome viability check (faster, may include\n"
        "                 portals that wouldn't actually generate)\n"
        "\n"
        "Examples:\n"
        "  %s 1 500\n"
        "  %s 1 1000 -t 8 -s 0 -e 1000000\n"
        "  %s 2 2000 -t 16 -v 1.20\n",
        prog_name, prog_name, prog_name, prog_name);
}

static uint64_t parse_u64(const char *s, const char *what)
{
    errno = 0;
    char *end = NULL;
    unsigned long long v = strtoull(s, &end, 0);
    if (errno != 0 || end == s || *end != '\0') {
        fprintf(stderr, "Invalid %s: '%s'\n", what, s);
        exit(2);
    }
    return (uint64_t)v;
}

static int parse_int_positive(const char *s, const char *what)
{
    errno = 0;
    char *end = NULL;
    long v = strtol(s, &end, 10);
    if (errno != 0 || end == s || *end != '\0' || v <= 0) {
        fprintf(stderr, "Invalid %s: '%s' (must be a positive integer)\n", what, s);
        exit(2);
    }
    return (int)v;
}

int parse_args(int argc, char **argv, SearchParams *params)
{
    // defaults
    params->start_seed = 0;
    params->end_seed = ((uint64_t)1 << 48) - 1;
    params->num_threads = 1;
    params->mc_version = MC_1_21;
    params->skip_biome_check = 0;

    if (argc < 3) {
        print_usage(argv[0]);
        return 0;
    }

    params->min_egas = parse_int_positive(argv[1], "min_egas");
    params->max_distance = parse_int_positive(argv[2], "max_distance");

    for (int i = 3; i < argc; i++) {
        if (strcmp(argv[i], "-s") == 0 && i + 1 < argc) {
            params->start_seed = parse_u64(argv[++i], "start_seed");
        } else if (strcmp(argv[i], "-e") == 0 && i + 1 < argc) {
            params->end_seed = parse_u64(argv[++i], "end_seed");
        } else if (strcmp(argv[i], "-t") == 0 && i + 1 < argc) {
            params->num_threads = parse_int_positive(argv[++i], "threads");
        } else if (strcmp(argv[i], "-v") == 0 && i + 1 < argc) {
            i++;
            int mc = str2mc(argv[i]);
            if (mc == MC_UNDEF) {
                fprintf(stderr, "Unknown Minecraft version: '%s'\n", argv[i]);
                return 0;
            }
            params->mc_version = mc;
        } else if (strcmp(argv[i], "--fast") == 0) {
            params->skip_biome_check = 1;
        } else {
            fprintf(stderr, "Unknown option: '%s'\n", argv[i]);
            print_usage(argv[0]);
            return 0;
        }
    }

    if (params->start_seed > params->end_seed) {
        fprintf(stderr, "start_seed (%" PRIu64 ") > end_seed (%" PRIu64 ")\n",
                params->start_seed, params->end_seed);
        return 0;
    }

    return 1;
}
