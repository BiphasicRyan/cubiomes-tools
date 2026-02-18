// seed + radius_chunks -> find ruined portals near spawn -> print chest + loot
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
#include <errno.h>
#include <string.h>

static inline int64_t sqr64(int64_t a) { return a * a; }

static uint64_t parse_u64(const char *s)
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

static int parse_i32_nonneg(const char *s, const char *what)
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

static void print_loot(const char *lootTable, int mc, uint64_t lootSeed)
{
    LootTableContext *ctx = NULL;
    if (!init_loot_table_name(&ctx, lootTable, mc) || ctx == NULL) {
        printf("      (loot table unsupported: %s)\n", lootTable);
        return;
    }

    set_loot_seed(ctx, lootSeed);
    generate_loot(ctx);

    if (ctx->generated_item_count <= 0) {
        printf("      (no items)\n");
        return;
    }

    for (int i = 0; i < ctx->generated_item_count; i++) {
        ItemStack it = ctx->generated_items[i];
        
        // Convert local item ID to global item ID first
        int globalItemId = get_global_item_id(ctx, it.item);
        const char *itemName = global_id2item_name(globalItemId, mc);
        
        if (itemName) {
            const char *displayName = itemName;
            if (strncmp(itemName, "minecraft:", 10) == 0) {
                displayName = itemName + 10;
            }
            printf("      %s x%d\n", displayName, it.count);
        } else {
            printf("      item id=%d (local=%d) count=%d\n", globalItemId, it.item, it.count);
        }
    }
}

int main(int argc, char **argv)
{
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <seed> <radius_chunks>\n", argv[0]);
        fprintf(stderr, "Example: %s 123456789 256\n", argv[0]);
        return 2;
    }

    const int mc = MC_1_21; // fixed to 1.21.x ruleset for this tool
    const uint64_t seed = parse_u64(argv[1]);
    const int radius_chunks = parse_i32_nonneg(argv[2], "radius_chunks");

    // Convert chunk radius to block radius for distance filter.
    const int64_t R = (int64_t)radius_chunks * 16;

    Generator g;
    setupGenerator(&g, mc, 0);
    applySeed(&g, DIM_OVERWORLD, seed);

    // Spawn anchor (fast)
    Pos spawn = estimateSpawn(&g, NULL);

    printf("mc=%s seed=%" PRIu64 " spawn=(%d,%d) radius=%d chunks\n",
           mc2str(mc), seed, spawn.x, spawn.z, radius_chunks);

    const int st = Ruined_Portal;

    StructureConfig sc;
    if (!getStructureConfig(st, mc, &sc)) {
        fprintf(stderr, "getStructureConfig failed for %s in %s\n",
                struct2str(st), mc2str(mc));
        return 3;
    }

    // Region-based enumeration for this structure type
    const int regChunks = sc.regionSize;
    const int64_t regBlocks = (int64_t)regChunks * 16;

    // Conservative region bounds around spawn-centered radius
    int rx0 = (int)((spawn.x - R) / regBlocks) - 2;
    int rx1 = (int)((spawn.x + R) / regBlocks) + 2;
    int rz0 = (int)((spawn.z - R) / regBlocks) - 2;
    int rz1 = (int)((spawn.z + R) / regBlocks) + 2;

    int portals = 0;
    int chests = 0;

    for (int rz = rz0; rz <= rz1; rz++)
    for (int rx = rx0; rx <= rx1; rx++)
    {
        Pos p;
        if (!getStructurePos(st, mc, seed, rx, rz, &p))
            continue;

        // Distance filter in blocks
        int64_t dx = (int64_t)p.x - spawn.x;
        int64_t dz = (int64_t)p.z - spawn.z;
        if (sqr64(dx) + sqr64(dz) > sqr64(R))
            continue;

        // Biome viability check. For ruined portals, flags=0.
        if (!isViableStructurePos(st, &g, p.x, p.z, 0))
            continue;

        portals++;

        // Biome at portal pos (scale=4 uses biome coords)
        int biomeId = getBiomeAt(&g, 4, p.x >> 2, 0, p.z >> 2);

        // Compute variant (rotation/mirror/type) for this structure instance
        StructureVariant sv = (StructureVariant){0};
        getVariant(&sv, st, mc, seed, p.x, p.z, biomeId);

        // Needed for pieces/loot support
        StructureSaltConfig ssconf;
        if (!getStructureSaltConfig(st, mc, sv.biome, &ssconf)) {
            printf("%s x=%d z=%d (no loot support)\n", struct2str(st), p.x, p.z);
            continue;
        }

        // Build structure pieces and extract chest info
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