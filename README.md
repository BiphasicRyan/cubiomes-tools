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
   ./tools/overworld_ruined_portal_chest_loot/overworld_ruined_portal_chest_loot 123456789 256
   ```

## Tools

### overworld_ruined_portal_chest_loot
Finds ruined portals near spawn and analyzes their chest contents.

**Usage:**
```bash
./tools/overworld_ruined_portal_chest_loot/overworld_ruined_portal_chest_loot <seed> <radius_chunks>
```

**Parameters:**
- `<seed>`: Minecraft world seed (decimal or hexadecimal with 0x prefix)
- `<radius_chunks>`: Search radius in chunks from spawn point

**Example:**
```bash
./tools/overworld_ruined_portal_chest_loot/overworld_ruined_portal_chest_loot 123456789 256
```

**Sample Output:**
```
mc=1.21.11 seed=123456789 spawn=(72,-40) radius=256 chunks
ruined_portal x=-560 z=-272 pieces=1
  chest[0] x=-560 z=-272 table=ruined_portal lootSeed=17115617723463524495
      flint x3
      golden_shovel x1
      obsidian x1
      golden_helmet x1
      golden_pickaxe x1
      golden_axe x1
      golden_apple x1
      gold_nugget x5
Found 1 ruined portals and 1 chest(s)
```

### ega_seed_finder
Searches for Minecraft seeds containing ruined portal chests with multiple enchanted golden apples (EGA). This tool scans consecutive seeds and logs any seed where at least one chest within 320 chunks of spawn contains 2 or more enchanted golden apples.

**Usage:**
```bash
./tools/overworld_ruined_portal_chest_loot/ega_seed_finder [start_seed]
```

**Parameters:**
- `[start_seed]`: Optional starting seed (defaults to 0 if not provided)

**Example:**
```bash
# Start searching from seed 0
./tools/overworld_ruined_portal_chest_loot/ega_seed_finder

# Resume searching from seed 1000000
./tools/overworld_ruined_portal_chest_loot/ega_seed_finder 1000000
```

**Sample Output:**
```
Starting search from seed 0
Searching 320 chunk radius from spawn for ruined portals
Logging seeds with chests containing ≥2 enchanted golden apples to ega_jackpot_seeds.txt
Press Ctrl+C to stop

Checked 1000 seeds, found 0 jackpots (current: 1000)
Found jackpot at seed 4006 (checked 4007)
Checked 2000 seeds, found 1 jackpots (current: 2000)
```

**Output File Format:** (ega_jackpot_seeds.txt)
```
seed=4006 ega_chests=1 (-2224,48)x2
seed=4069 ega_chests=1 (1024,-1920)x2
```
Each line shows: seed, number of qualifying chests, and (x,z)xcount for each chest location.

**Note:** Finding 2+ enchanted golden apples in a single chest is extremely rare (roughly 1 in 80,000 per chest). Expect to check millions of seeds to find results.

## Development

### Building Individual Components

```bash
# Build only the cubiomes library
make cubiomes

# Build only the tools (requires cubiomes library to be built first)
make tools

# Clean all built files
make clean
```

### Updating Dependencies

#### Update cubiomes submodule
To update to the latest version of the cubiomes dependency:

```bash
# Navigate to the cubiomes submodule directory
cd cubiomes

# Pull the latest changes
git pull origin main

# Navigate back to the root directory
cd ..

# Commit the submodule update
git add cubiomes
git commit -m "Update cubiomes submodule to latest version"
git push
```

#### Update submodule to specific commit/branch
```bash
# Navigate to the cubiomes submodule directory
cd cubiomes

# Checkout a specific branch or commit
git checkout <branch-name-or-commit-hash>

# Navigate back to the root directory
cd ..

# Commit the submodule update
git add cubiomes
git commit -m "Update cubiomes submodule to <branch-name-or-commit-hash>"
git push
```

### Adding New Tools

1. Create a new directory under `tools/`:
   ```bash
   mkdir tools/my_tool
   ```

2. Add your source files and a Makefile (see `tools/overworld_ruined_portal_chest_loot/Makefile` for reference)

3. Update the root `Makefile` to include your new tool in the `TOOLS` variable

4. Build and test:
   ```bash
   make tools
   ```

## Repository Structure

```
cubiomes-tools/
├── README.md                 # This file
├── Makefile                  # Root build file
├── .gitignore               # Git ignore patterns
├── .gitmodules              # Git submodule configuration
├── cubiomes/                # cubiomes library (git submodule)
│   ├── libcubiomes.a       # Compiled library
│   ├── *.h                 # Header files
│   └── ...                 # Other cubiomes files
└── tools/                   # Tool implementations
    └── overworld_ruined_portal_chest_loot/
        ├── overworld_ruined_portal_chest_loot.c   # Source code
        ├── ega_seed_finder.c                      # EGA seed finder source
        ├── loot_printer.c                         # Loot display utilities
        ├── parsing.c                              # Argument parsing utilities
        ├── Makefile        # Tool-specific build file
        ├── overworld_ruined_portal_chest_loot     # Compiled binary
        └── ega_seed_finder                          # EGA finder compiled binary
```

### Submodule Issues

**Submodule directory is empty**
```bash
git submodule update --init --recursive
```