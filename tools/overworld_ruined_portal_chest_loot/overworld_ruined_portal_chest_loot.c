#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "parsing.h"
#include "finders.h"
#include "generator.h"
#include "loot/loot_tables.h"
#include "loot/mc_loot.h"
#include "loot/items.h"

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

    LootTableContext *lootCtx = NULL;
    if (!init_ruined_portal(&lootCtx, mc)) {
        fprintf(stderr, "Error: could not init ruined portal loot table\n");
        return 1;
    }

    Generator g;
    setupGenerator(&g, mc, 0);

    for (uint64_t seed = config.start_seed; seed <= config.end_seed; seed++) {
        applySeed(&g, 0, seed);

        for (int regX = -maxRegion; regX <= maxRegion; regX++) {
            for (int regZ = -maxRegion; regZ <= maxRegion; regZ++) {
                Pos pos;
                if (!getStructurePos(Ruined_Portal, mc, seed, regX, regZ, &pos))
                    continue;

                double dist = sqrt((double)pos.x * pos.x + (double)pos.z * pos.z);
                if (dist > config.max_distance)
                    continue;

                if (!config.fast_mode) {
                    if (!isViableStructurePos(Ruined_Portal, &g, pos.x, pos.z, 0))
                        continue;
                }

                int biomeID = getBiomeAt(&g, 4, pos.x >> 2, 160 >> 2, pos.z >> 2);

                StructureVariant sv;
                if (!getVariant(&sv, Ruined_Portal, mc, seed, pos.x, pos.z, biomeID))
                    continue;

                StructureSaltConfig ssconf;
                if (!getStructureSaltConfig(Ruined_Portal, mc, sv.biome, &ssconf))
                    continue;

                Piece piece;
                if (getStructurePieces(&piece, 1, Ruined_Portal, ssconf, &sv, mc, seed, pos.x, pos.z) < 1)
                    continue;

                set_loot_seed(lootCtx, piece.lootSeeds[0]);
                generate_loot(lootCtx);

                int ega_count = 0;
                for (int i = 0; i < lootCtx->generated_item_count; i++) {
                    int gid = get_global_item_id(lootCtx, lootCtx->generated_items[i].item);
                    if (gid == ITEM_ENCHANTED_GOLDEN_APPLE)
                        ega_count += lootCtx->generated_items[i].count;
                }

                if (ega_count >= config.min_egas) {
                    printf("SEED %-12llu  Portal at (%d, %d)  dist=%.0f  EGAs: %d\n",
                           (unsigned long long)seed, pos.x, pos.z, dist, ega_count);
                    printf("  Chest loot:\n");
                    for (int i = 0; i < lootCtx->generated_item_count; i++) {
                        const char *name = get_item_name(lootCtx, lootCtx->generated_items[i].item);
                        int count = lootCtx->generated_items[i].count;
                        int gid = get_global_item_id(lootCtx, lootCtx->generated_items[i].item);
                        printf("    %s x%d%s\n", name, count,
                               gid == ITEM_ENCHANTED_GOLDEN_APPLE ? "  <<< EGA" : "");
                    }
                }
            }
        }
    }

    printf("\nDone. Checked %llu seeds.\n",
           (unsigned long long)(config.end_seed - config.start_seed + 1));

    return 0;
}
