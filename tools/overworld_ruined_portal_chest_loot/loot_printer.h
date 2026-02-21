#ifndef LOOT_PRINTER_H
#define LOOT_PRINTER_H

#include <stdint.h>

// Count enchanted golden apples in a ruined portal chest for a given loot seed.
int count_egas_in_chest(int mc, uint64_t loot_seed);

// Print the full loot contents of a chest, marking EGAs with "<<<".
void print_chest_loot(int mc, uint64_t loot_seed);

#endif // LOOT_PRINTER_H
