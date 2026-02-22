#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "parsing.h"
#include "finders.h"
#include "generator.h"

int main(int argc, char *argv[])
{
    SearchConfig config;
    if (!parse_args(argc, argv, &config)) {
        return 1;
    }

    int mc = MC_1_21;

    printf("Search parameters:\n");
    printf("  min_egas:     %d\n", config.min_egas);
    printf("  max_distance: %d\n", config.max_distance);
    printf("  start_seed:   %llu\n", (unsigned long long)config.start_seed);
    printf("  end_seed:     %llu\n", (unsigned long long)config.end_seed);
    printf("  threads:      %d\n", config.threads);
    printf("  fast_mode:    %s\n", config.fast_mode ? "yes" : "no");
    printf("\n");

    StructureConfig sconf;
    if (!getStructureConfig(Ruined_Portal, mc, &sconf)) {
        fprintf(stderr, "Error: could not get Ruined_Portal config for this MC version\n");
        return 1;
    }

    int regionBlocks = sconf.regionSize * 16;
    int maxRegion = (config.max_distance / regionBlocks) + 1;

    printf("Ruined_Portal config: regionSize=%d chunkRange=%d\n", sconf.regionSize, sconf.chunkRange);
    printf("Region scan range: -%d to +%d (%d regions per axis)\n\n",
           maxRegion, maxRegion, 2 * maxRegion + 1);

    Generator g;
    setupGenerator(&g, mc, 0);

    for (uint64_t seed = config.start_seed; seed <= config.end_seed; seed++) {
        applySeed(&g, 0, seed);

        int found = 0;
        for (int regX = -maxRegion; regX <= maxRegion; regX++) {
            for (int regZ = -maxRegion; regZ <= maxRegion; regZ++) {
                Pos pos;
                if (!getStructurePos(Ruined_Portal, mc, seed, regX, regZ, &pos))
                    continue;

                double dist = sqrt((double)pos.x * pos.x + (double)pos.z * pos.z);
                if (dist > config.max_distance)
                    continue;

                if (!found) {
                    printf("Seed %llu:\n", (unsigned long long)seed);
                    found = 1;
                }
                printf("  Portal candidate at (%d, %d)  dist=%.0f\n", pos.x, pos.z, dist);
            }
        }
    }

    printf("\nDone. Checked %llu seeds.\n",
           (unsigned long long)(config.end_seed - config.start_seed + 1));

    return 0;
}
