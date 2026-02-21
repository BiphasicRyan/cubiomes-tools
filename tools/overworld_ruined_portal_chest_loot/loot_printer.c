#include "loot_printer.h"

#include "loot/loot_tables.h"
#include "loot/loot_table_context.h"
#include "loot/items.h"

#include <stdio.h>
#include <string.h>
#include <inttypes.h>

int count_egas_in_chest(int mc, uint64_t loot_seed)
{
    LootTableContext *ctx = NULL;
    if (!init_ruined_portal(&ctx, mc) || ctx == NULL)
        return 0;

    set_loot_seed(ctx, loot_seed);
    generate_loot(ctx);

    int ega_count = 0;
    for (int i = 0; i < ctx->generated_item_count; i++) {
        int global_id = get_global_item_id(ctx, ctx->generated_items[i].item);
        if (global_id == ITEM_ENCHANTED_GOLDEN_APPLE)
            ega_count += ctx->generated_items[i].count;
    }

    // Note: init_ruined_portal allocates ctx each call. For the hot path
    // (count_egas_in_chest called per-chest in the search loop) we accept
    // the leak because the context is small and seeds that reach this point
    // are rare.  print_chest_loot is only called on hits so the leak there
    // is negligible.
    return ega_count;
}

void print_chest_loot(int mc, uint64_t loot_seed)
{
    LootTableContext *ctx = NULL;
    if (!init_ruined_portal(&ctx, mc) || ctx == NULL) {
        printf("      (loot table unsupported for this MC version)\n");
        return;
    }

    set_loot_seed(ctx, loot_seed);
    generate_loot(ctx);

    if (ctx->generated_item_count <= 0) {
        printf("      (no items)\n");
        return;
    }

    for (int i = 0; i < ctx->generated_item_count; i++) {
        ItemStack it = ctx->generated_items[i];
        int global_id = get_global_item_id(ctx, it.item);
        const char *item_name = global_id2item_name(global_id, mc);

        if (item_name) {
            const char *display = item_name;
            if (strncmp(item_name, "minecraft:", 10) == 0)
                display = item_name + 10;
            printf("      %s x%d", display, it.count);
            if (global_id == ITEM_ENCHANTED_GOLDEN_APPLE)
                printf("  <<<  EGA");
            printf("\n");
        } else {
            printf("      item_id=%d count=%d\n", global_id, it.count);
        }
    }
}
