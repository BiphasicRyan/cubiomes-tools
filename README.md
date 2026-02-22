# Cubiomes Tools

A collection of tools built on top of the [cubiomes](https://github.com/xpple/cubiomes) library for Minecraft world generation analysis.

### Installation
1. **Clone the repository with submodules:**
   ```bash
   git clone --recursive https://github.com/BiphasicRyan/cubiomes-tools.git
   cd cubiomes-tools
   ```

2. **Build everything:**
   ```bash
   make
   ```

3. **Run the tools:**
   ```bash
   ./tools/overworld_ruined_portal_chest_loot/overworld_ruined_portal_chest_loot 1 500 -s 0 -e 100000
   ```

## Tools

### overworld_ruined_portal_chest_loot

Searches for Minecraft seeds with ruined portal chests containing enchanted golden apples (EGAs). Reports seeds where individual chests meet the EGA threshold.

**Usage:**
```bash
./tools/overworld_ruined_portal_chest_loot/overworld_ruined_portal_chest_loot <min_egas> <max_distance> [options]
```

**Required Arguments:**
- `<min_egas>` — Minimum enchanted golden apples in a single chest (chest must have ≥ this many EGAs to be reported)
- `<max_distance>` — Maximum block distance from (0,0) to search for portals

**Optional Arguments:**
- `-s START` — Start seed (default: 0)
- `-e END` — End seed inclusive (default: 2^48 - 1)
- `-t THREADS` — Number of threads (default: 1)
- `--fast` — Skip biome viability check (faster, may include portals that wouldn't actually generate)

**Examples:**
```bash
# Find seeds where a single chest within 500 blocks has 1+ EGAs
./tools/overworld_ruined_portal_chest_loot/overworld_ruined_portal_chest_loot 1 500 -s 0 -e 1000

# Search seeds 0-100000 with 4 threads, fast mode
./tools/overworld_ruined_portal_chest_loot/overworld_ruined_portal_chest_loot 2 1000 -s 0 -e 100000 -t 4 --fast

# Check a specific seed for any EGA chests within 1000 blocks
./tools/overworld_ruined_portal_chest_loot/overworld_ruined_portal_chest_loot 1 1000 -s 271933139 -e 271933139
```

**Sample Output:**
```
SEED 271933139     Portal at (-352, 304)  dist=465  EGAs: 4
    minecraft:enchanted_golden_apple x1  <<< EGA
    minecraft:enchanted_golden_apple x1  <<< EGA
    minecraft:flint x2
    minecraft:flint_and_steel x1
    minecraft:enchanted_golden_apple x1  <<< EGA
    minecraft:enchanted_golden_apple x1  <<< EGA
    minecraft:golden_apple x1
    minecraft:fire_charge x1

Done. Checked 1 seeds, found 1 hits. Results appended to results.txt
```

**Results File:**
Found seeds are appended to `results.txt` in the format:
```
<seed> <portal_x> <portal_z> <ega_count>
```

**Notes:**
- The tool uses a per-chest threshold: a chest is only reported if it individually contains ≥ `min_egas` EGAs
- Multi-threaded searches are thread-safe; each thread gets its own `Generator` and `LootTableContext` copy
- Hardcoded to Minecraft 1.21 (MC_1_21)
- Searching large seed ranges with large distances can be slow due to many portal candidates per seed

## Development

### Building

```bash
# Build everything (cubiomes library + tools)
make

# Build only the cubiomes library
make cubiomes

# Build only the tools (requires cubiomes library)
make tools

# Clean all built files
make clean
```

### Adding New Tools

1. Create a new directory under `tools/`:
   ```bash
   mkdir tools/my_tool
   ```

2. Add your source files (`.c` and `.h`)

3. Add a build target to the root `Makefile` and append it to the `TOOLS` variable

4. Build and test:
   ```bash
   make tools
   ```

### Updating the cubiomes Submodule

```bash
cd cubiomes
git pull origin main
cd ..
git add cubiomes
git commit -m "Update cubiomes submodule"
```

## Repository Structure

```
cubiomes-tools/
├── README.md
├── Makefile
├── .gitignore
├── .gitmodules
├── cubiomes/                  # cubiomes library (git submodule)
│   ├── libcubiomes.a          # Compiled library
│   ├── *.h                    # Header files (finders.h, generator.h, etc.)
│   ├── loot/                  # Loot generation headers and tables
│   └── ...
└── tools/
    └── overworld_ruined_portal_chest_loot/
        ├── overworld_ruined_portal_chest_loot.c   # Main tool (search + threading)
        ├── parsing.c                              # CLI argument parsing
        └── parsing.h                              # SearchConfig struct + prototypes
```

### Submodule Issues

**Submodule directory is empty:**
```bash
git submodule update --init --recursive
```