#include "parsing.h"

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
#include <string.h>
#include <pthread.h>

// -------------------------------------------------------------------------
// Shared state
// -------------------------------------------------------------------------

static pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t loot_mutex  = PTHREAD_MUTEX_INITIALIZER;
static volatile uint64_t total_hits = 0;
static FILE *results_file = NULL;
static const char *results_path = "results.txt";

typedef struct {
    int px, pz;             // portal position
    int cx, cz;             // chest position
    int egas;               // EGA count
    char item_lines[27][256];  // pre-generated item strings
    int num_items;
} ChestHit;

typedef struct {
    SearchParams params;
    uint64_t seed_start;
    uint64_t seed_end;
    int thread_id;
} ThreadArg;

// Generate loot once, count EGAs, and store item strings for later printing.
// IMPORTANT: The cubiomes loot library uses a static singleton context, so all
// loot generation must be serialized with loot_mutex to prevent data races.
static int generate_chest_loot(int mc, uint64_t loot_seed, ChestHit *hit)
{
    pthread_mutex_lock(&loot_mutex);

    LootTableContext *ctx = NULL;
    if (!init_ruined_portal(&ctx, mc) || ctx == NULL) {
        pthread_mutex_unlock(&loot_mutex);
        return 0;
    }

    set_loot_seed(ctx, loot_seed);
    generate_loot(ctx);

    int ega_count = 0;
    hit->num_items = 0;

    for (int i = 0; i < ctx->generated_item_count && hit->num_items < 27; i++) {
        ItemStack it = ctx->generated_items[i];
        int global_id = get_global_item_id(ctx, it.item);
        
        if (global_id == ITEM_ENCHANTED_GOLDEN_APPLE)
            ega_count += it.count;

        const char *item_name = global_id2item_name(global_id, mc);
        if (item_name) {
            const char *display = item_name;
            if (strncmp(item_name, "minecraft:", 10) == 0)
                display = item_name + 10;
            if (global_id == ITEM_ENCHANTED_GOLDEN_APPLE)
                snprintf(hit->item_lines[hit->num_items], 256, "      %s x%d  <<<  EGA", display, it.count);
            else
                snprintf(hit->item_lines[hit->num_items], 256, "      %s x%d", display, it.count);
        } else {
            snprintf(hit->item_lines[hit->num_items], 256, "      item_id=%d count=%d", global_id, it.count);
        }
        hit->num_items++;
    }

    hit->egas = ega_count;

    pthread_mutex_unlock(&loot_mutex);
    return ega_count;
}

// -------------------------------------------------------------------------
// Per-seed check
// -------------------------------------------------------------------------

static int check_seed(const SearchParams *p, Generator *g, uint64_t seed)
{
    const int mc  = p->mc_version;
    const int st  = Ruined_Portal;
    const int64_t R = (int64_t)p->max_distance;

    applySeed(g, DIM_OVERWORLD, seed);

    StructureConfig sc;
    if (!getStructureConfig(st, mc, &sc))
        return 0;

    const int64_t regBlocks = (int64_t)sc.regionSize * 16;

    // Region bounds covering the circle of radius R around (0,0)
    int rx0 = (int)((-R) / regBlocks) - 1;
    int rx1 = (int)(( R) / regBlocks) + 1;
    int rz0 = rx0;
    int rz1 = rx1;

    // Collect chests where that individual chest has >= min_egas EGAs
    ChestHit hits[64];
    int nhits = 0;

    for (int rz = rz0; rz <= rz1; rz++) {
        for (int rx = rx0; rx <= rx1; rx++) {
            Pos pos;
            if (!getStructurePos(st, mc, seed, rx, rz, &pos))
                continue;

            // Euclidean distance check from (0,0)
            int64_t dx = (int64_t)pos.x;
            int64_t dz = (int64_t)pos.z;
            if (dx * dx + dz * dz > R * R)
                continue;

            // Biome viability (optional)
            if (!p->skip_biome_check) {
                if (!isViableStructurePos(st, g, pos.x, pos.z, 0))
                    continue;
            }

            // Get biome for variant computation
            int biomeId = getBiomeAt(g, 4, pos.x >> 2, 0, pos.z >> 2);

            StructureVariant sv = {0};
            getVariant(&sv, st, mc, seed, pos.x, pos.z, biomeId);

            StructureSaltConfig ssconf;
            if (!getStructureSaltConfig(st, mc, sv.biome, &ssconf))
                continue;

            Piece pieces[64];
            int nPieces = getStructurePieces(pieces, 64, st, ssconf, &sv,
                                             mc, seed, pos.x, pos.z);

            for (int i = 0; i < nPieces; i++) {
                for (int c = 0; c < pieces[i].chestCount; c++) {
                    if (nhits >= 64) continue;
                    
                    uint64_t lootSeed = pieces[i].lootSeeds[c];
                    Pos chestPos = pieces[i].chestPoses[c];
                    
                    hits[nhits].px = pos.x;
                    hits[nhits].pz = pos.z;
                    hits[nhits].cx = chestPos.x;
                    hits[nhits].cz = chestPos.z;
                    
                    int egas = generate_chest_loot(mc, lootSeed, &hits[nhits]);
                    
                    if (egas >= p->min_egas) {
                        nhits++;
                    }
                }
            }
        }
    }

    if (nhits == 0)
        return 0;

    // ---- Print result (thread-safe) ----
    pthread_mutex_lock(&print_mutex);
    total_hits++;
    printf("========================================\n");
    printf("SEED %" PRId64 " (unsigned %" PRIu64 ")  |  Qualifying chests: %d\n",
           (int64_t)seed, seed, nhits);
    printf("----------------------------------------\n");
    
    for (int i = 0; i < nhits; i++) {
        printf("  Portal at (%d, %d)  Chest at (%d, %d)  EGAs: %d\n",
               hits[i].px, hits[i].pz, hits[i].cx, hits[i].cz, hits[i].egas);
        printf("    Chest loot:\n");
        if (hits[i].num_items == 0) {
            printf("      (no items)\n");
        } else {
            for (int j = 0; j < hits[i].num_items; j++) {
                printf("%s\n", hits[i].item_lines[j]);
            }
        }
    }
    
    printf("========================================\n\n");
    fflush(stdout);
    
    if (results_file) {
        for (int i = 0; i < nhits; i++) {
            fprintf(results_file, "%" PRId64 " %d %d %d\n",
                    (int64_t)seed, hits[i].px, hits[i].pz, hits[i].egas);
        }
        fflush(results_file);
    }
    pthread_mutex_unlock(&print_mutex);

    return 1;
}

// -------------------------------------------------------------------------
// Worker thread
// -------------------------------------------------------------------------

static void *worker(void *arg)
{
    ThreadArg *ta = (ThreadArg *)arg;
    const SearchParams *p = &ta->params;

    Generator g;
    setupGenerator(&g, p->mc_version, 0);

    uint64_t count = 0;
    uint64_t range = ta->seed_end - ta->seed_start + 1;
    uint64_t next_report = range / 1000;  // ~1% progress steps
    if (next_report == 0) next_report = 1;

    for (uint64_t seed = ta->seed_start; seed <= ta->seed_end; seed++) {
        check_seed(p, &g, seed);
        count++;

        if (count % next_report == 0) {
            double pct = 100.0 * (double)count / (double)range;
            pthread_mutex_lock(&print_mutex);
            fprintf(stderr, "[thread %d] %.1f%% (seed %" PRIu64 "/%" PRIu64 ")\n",
                    ta->thread_id, pct, count, range);
            pthread_mutex_unlock(&print_mutex);
        }

        // Prevent infinite loop if seed_end == UINT64_MAX
        if (seed == ta->seed_end) break;
    }

    pthread_mutex_lock(&print_mutex);
    fprintf(stderr, "[thread %d] done. Checked %" PRIu64 " seeds.\n",
            ta->thread_id, count);
    pthread_mutex_unlock(&print_mutex);

    return NULL;
}

// -------------------------------------------------------------------------
// main
// -------------------------------------------------------------------------

int main(int argc, char **argv)
{
    SearchParams params;
    if (!parse_args(argc, argv, &params))
        return 2;

    results_file = fopen(results_path, "a");
    if (!results_file) {
        fprintf(stderr, "Warning: could not open '%s' for writing\n", results_path);
    } else {
        fprintf(stderr, "Appending hits to '%s'\n", results_path);
    }

    fprintf(stderr, "Searching for seeds with >= %d EGA(s) in ruined portal chests\n",
            params.min_egas);
    fprintf(stderr, "  within %d blocks of (0,0)\n", params.max_distance);
    fprintf(stderr, "  seed range: %" PRIu64 " .. %" PRIu64 "\n",
            params.start_seed, params.end_seed);
    fprintf(stderr, "  MC version: %s  threads: %d  biome check: %s\n",
            mc2str(params.mc_version), params.num_threads,
            params.skip_biome_check ? "OFF (fast)" : "ON");
    fprintf(stderr, "\n");

    int nthreads = params.num_threads;
    uint64_t total_seeds = params.end_seed - params.start_seed + 1;

    pthread_t *threads = calloc(nthreads, sizeof(pthread_t));
    ThreadArg *args    = calloc(nthreads, sizeof(ThreadArg));

    uint64_t chunk = total_seeds / nthreads;
    uint64_t remainder = total_seeds % nthreads;

    uint64_t cursor = params.start_seed;
    for (int i = 0; i < nthreads; i++) {
        args[i].params    = params;
        args[i].thread_id = i;
        args[i].seed_start = cursor;
        uint64_t this_chunk = chunk + (i < (int)remainder ? 1 : 0);
        args[i].seed_end = cursor + this_chunk - 1;
        cursor += this_chunk;

        pthread_create(&threads[i], NULL, worker, &args[i]);
    }

    for (int i = 0; i < nthreads; i++) {
        pthread_join(threads[i], NULL);
    }

    fprintf(stderr, "\nFinished. Total hits: %" PRIu64 "\n", total_hits);

    if (results_file)
        fclose(results_file);
    free(threads);
    free(args);
    return 0;
}
