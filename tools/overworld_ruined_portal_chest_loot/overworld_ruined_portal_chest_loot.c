#include "generator.h"
#include "finders.h"
#include "util.h"

#include "loot/loot_tables.h"
#include "loot/loot_table_context.h"
#include "loot/items.h"

#include "parsing.h"
#include "loot_printer.h"

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// Calculate distance squared from spawn point
// Used to check if ruined portal is within search radius
static inline int64_t sqr64(int64_t a) { return a * a; }


int main(int argc, char **argv)
{
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <seed> <radius_chunks>\n", argv[0]);
        fprintf(stderr, "Example: %s 123456789 256\n", argv[0]);
        return 2;
    }

    const int mc = MC_1_21;  // Minecraft version 1.21
    const uint64_t seed = parse_u64(argv[1]);  // World seed
    const int radius_chunks = parse_i32_nonneg(argv[2], "radius_chunks");

    // Convert chunk radius to block radius for distance checking
    const int64_t R = (int64_t)radius_chunks * 16;

    // Initialize world generator with seed
    Generator g;
    setupGenerator(&g, mc, 0);
    applySeed(&g, DIM_OVERWORLD, seed);

    // Find approximate world spawn point
    Pos spawn = estimateSpawn(&g, NULL);

    printf("mc=%s seed=%" PRIu64 " spawn=(%d,%d) radius=%d chunks\n",
           mc2str(mc), seed, spawn.x, spawn.z, radius_chunks);

    // Set up structure search for ruined portals
    const int st = Ruined_Portal;

    StructureConfig sc;
    if (!getStructureConfig(st, mc, &sc)) {
        fprintf(stderr, "getStructureConfig failed for %s in %s\n",
                struct2str(st), mc2str(mc));
        return 3;
    }

    // Find all regions that could contain ruined portals
    const int regChunks = sc.regionSize;
    const int64_t regBlocks = (int64_t)regChunks * 16;

    // Set search boundaries around spawn point
    int rx0 = (int)((spawn.x - R) / regBlocks) - 2;
    int rx1 = (int)((spawn.x + R) / regBlocks) + 2;
    int rz0 = (int)((spawn.z - R) / regBlocks) - 2;
    int rz1 = (int)((spawn.z + R) / regBlocks) + 2;

    int portals = 0;
    int chests = 0;

    // Search through all regions that might contain portals
    for (int rz = rz0; rz <= rz1; rz++)
    for (int rx = rx0; rx <= rx1; rx++)
    {
        Pos p;
        if (!getStructurePos(st, mc, seed, rx, rz, &p))
            continue;

        // Check if portal is too far from spawn
        int64_t dx = (int64_t)p.x - spawn.x;
        int64_t dz = (int64_t)p.z - spawn.z;
        if (sqr64(dx) + sqr64(dz) > sqr64(R))
            continue;

        if (!isViableStructurePos(st, &g, p.x, p.z, 0))
            continue;

        portals++;

        int biomeId = getBiomeAt(&g, 4, p.x >> 2, 0, p.z >> 2);

        // Calculate how this portal looks (rotation, materials)
        StructureVariant sv = (StructureVariant){0};
        getVariant(&sv, st, mc, seed, p.x, p.z, biomeId);

        // Get loot generation settings for this portal
        StructureSaltConfig ssconf;
        if (!getStructureSaltConfig(st, mc, sv.biome, &ssconf)) {
            printf("%s x=%d z=%d (no loot support)\n", struct2str(st), p.x, p.z);
            continue;
        }

        // Build the actual portal structure and search each piece for chests
        Piece pieces[64];
        int nPieces = getStructurePieces(pieces, 64, st, ssconf, &sv, mc, seed, p.x, p.z);

        printf("%s x=%d z=%d pieces=%d\n", struct2str(st), p.x, p.z, nPieces);

        for (int i = 0; i < nPieces; i++) {
            for (int c = 0; c < pieces[i].chestCount; c++) {
                Pos chest = pieces[i].chestPoses[c];
                uint64_t lootSeed = pieces[i].lootSeeds[c];
                const char *lootTable = pieces[i].lootTables[c];
                chests++;

                printf("  chest[%d] x=%d z=%d table=%s lootSeed=%" PRIu64 "\n",
                       c, chest.x, chest.z, lootTable, lootSeed);

                print_loot(lootTable, mc, lootSeed);
            }
        }
    }

    fprintf(stderr, "Found %d ruined portals and %d chest(s)\n", portals, chests);
    return 0;
}