#include "generator.h"
#include "finders.h"
#include "util.h"

#include "loot/loot_tables.h"
#include "loot/loot_table_context.h"
#include "loot/items.h"

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static inline int64_t sqr64(int64_t a) { return a * a; }

// Counts enchanted golden apples in generated loot
// Returns total count of ITEM_ENCHANTED_GOLDEN_APPLE in the chest
int count_enchanted_golden_apples(LootTableContext *ctx)
{
    int count = 0;
    for (int i = 0; i < ctx->generated_item_count; i++) {
        ItemStack it = ctx->generated_items[i];
        int globalItemId = get_global_item_id(ctx, it.item);
        if (globalItemId == ITEM_ENCHANTED_GOLDEN_APPLE) {
            count += it.count;
        }
    }
    return count;
}

// Checks a single seed for ruined portals with ≥2 enchanted golden apples total
// across all chests within radius. Returns total EGA count (0 if none found).
int check_seed(uint64_t seed, int mc, int radius_chunks, FILE *outfile)
{
    const int64_t R = (int64_t)radius_chunks * 16;

    Generator g;
    setupGenerator(&g, mc, 0);
    applySeed(&g, DIM_OVERWORLD, seed);

    Pos spawn = estimateSpawn(&g, NULL);

    const int st = Ruined_Portal;

    StructureConfig sc;
    if (!getStructureConfig(st, mc, &sc)) {
        return 0;
    }

    const int regChunks = sc.regionSize;
    const int64_t regBlocks = (int64_t)regChunks * 16;

    int rx0 = (int)((spawn.x - R) / regBlocks) - 2;
    int rx1 = (int)((spawn.x + R) / regBlocks) + 2;
    int rz0 = (int)((spawn.z - R) / regBlocks) - 2;
    int rz1 = (int)((spawn.z + R) / regBlocks) + 2;

    int total_ega = 0;
    int total_chests = 0;
    
    // Track up to 64 chests with EGA
    typedef struct { int x; int z; int count; } EgaChest;
    EgaChest ega_chests[64];
    int ega_chest_count = 0;

    for (int rz = rz0; rz <= rz1; rz++)
    for (int rx = rx0; rx <= rx1; rx++)
    {
        Pos p;
        if (!getStructurePos(st, mc, seed, rx, rz, &p))
            continue;

        int64_t dx = (int64_t)p.x - spawn.x;
        int64_t dz = (int64_t)p.z - spawn.z;
        if (sqr64(dx) + sqr64(dz) > sqr64(R))
            continue;

        if (!isViableStructurePos(st, &g, p.x, p.z, 0))
            continue;

        int biomeId = getBiomeAt(&g, 4, p.x >> 2, 0, p.z >> 2);

        StructureVariant sv = (StructureVariant){0};
        getVariant(&sv, st, mc, seed, p.x, p.z, biomeId);

        StructureSaltConfig ssconf;
        if (!getStructureSaltConfig(st, mc, sv.biome, &ssconf)) {
            continue;
        }

        Piece pieces[64];
        int nPieces = getStructurePieces(pieces, 64, st, ssconf, &sv, mc, seed, p.x, p.z);

        for (int i = 0; i < nPieces; i++) {
            for (int c = 0; c < pieces[i].chestCount; c++) {
                Pos chest = pieces[i].chestPoses[c];
                uint64_t lootSeed = pieces[i].lootSeeds[c];
                const char *lootTable = pieces[i].lootTables[c];

                LootTableContext *ctx = NULL;
                if (!init_loot_table_name(&ctx, lootTable, mc) || ctx == NULL) {
                    continue;
                }

                set_loot_seed(ctx, lootSeed);
                generate_loot(ctx);

                total_chests++;
                int ega_count = count_enchanted_golden_apples(ctx);
                total_ega += ega_count;
                
                // Track this chest if it has EGA
                if (ega_count > 0 && ega_chest_count < 64) {
                    ega_chests[ega_chest_count].x = chest.x;
                    ega_chests[ega_chest_count].z = chest.z;
                    ega_chests[ega_chest_count].count = ega_count;
                    ega_chest_count++;
                }
            }
        }
    }

    if (total_ega >= 2) {
        fprintf(outfile, "seed=%" PRIu64 " total_ega=%d total_chests=%d ega_chests=%d",
                seed, total_ega, total_chests, ega_chest_count);
        for (int i = 0; i < ega_chest_count; i++) {
            fprintf(outfile, " (%d,%d)x%d", ega_chests[i].x, ega_chests[i].z, ega_chests[i].count);
        }
        fprintf(outfile, "\n");
        fflush(outfile);
        return total_ega;
    }

    return 0;
}

int main(int argc, char **argv)
{
    const int mc = MC_1_21;
    const int radius_chunks = 320;
    const char *outfile_name = "ega_jackpot_seeds.txt";

    uint64_t start_seed = 0;
    if (argc >= 2) {
        char *end = NULL;
        start_seed = strtoull(argv[1], &end, 0);
        if (end == argv[1] || *end != '\0') {
            fprintf(stderr, "Invalid start seed: %s\n", argv[1]);
            return 2;
        }
    }

    FILE *outfile = fopen(outfile_name, "a");
    if (!outfile) {
        fprintf(stderr, "Failed to open %s for writing\n", outfile_name);
        return 3;
    }

    fprintf(stderr, "Starting search from seed %" PRIu64 "\n", start_seed);
    fprintf(stderr, "Searching %d chunk radius from spawn for ruined portals\n", radius_chunks);
    fprintf(stderr, "Logging seeds with ≥2 enchanted golden apples to %s\n", outfile_name);
    fprintf(stderr, "Press Ctrl+C to stop\n\n");

    uint64_t seed = start_seed;
    uint64_t checked = 0;
    uint64_t found = 0;

    while (1) {
        if (check_seed(seed, mc, radius_chunks, outfile)) {
            found++;
            fprintf(stderr, "Found jackpot at seed %" PRIu64 " (checked %" PRIu64 ")\n", seed, checked + 1);
        }

        checked++;
        seed++;

        // Progress report every 1000 seeds
        if (checked % 1000 == 0) {
            fprintf(stderr, "Checked %" PRIu64 " seeds, found %" PRIu64 " jackpots (current: %" PRIu64 ")\n",
                    checked, found, seed);
        }
    }

    fclose(outfile);
    return 0;
}
