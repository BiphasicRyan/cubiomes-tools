#include "loot_display.h"

#include "loot/loot_tables.h"
#include "loot/loot_table_context.h"
#include "loot/items.h"

#include <stdio.h>
#include <string.h>

void print_loot(const char *lootTable, int mc, uint64_t lootSeed)
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
