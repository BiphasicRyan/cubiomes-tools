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
- `-v VERSION` — Minecraft version string, e.g., `1.21`, `1.20`, `1.16` (default: 1.21.11)
- `--fast` — Skip biome viability check (faster, may include portals that wouldn't actually generate)

**Examples:**
```bash
# Find seeds where a single chest within 500 blocks has 3+ EGAs
./tools/overworld_ruined_portal_chest_loot/overworld_ruined_portal_chest_loot 3 500

# Search seeds 0-100000 with 4 threads, fast mode
./tools/overworld_ruined_portal_chest_loot/overworld_ruined_portal_chest_loot 2 1000 -s 0 -e 100000 -t 4 --fast

# Check a specific seed for any EGA chests within 4000 blocks
./tools/overworld_ruined_portal_chest_loot/overworld_ruined_portal_chest_loot 1 4000 -s 36568 -e 36568 -t 1
```

**Sample Output:**
```
========================================
SEED 510544 (unsigned 510544)  |  Qualifying chests: 2
----------------------------------------
  Portal at (-432, 16)  Chest at (-432, 16)  EGAs: 1
    Chest loot:
      obsidian x1
      bell x1
      enchanted_golden_apple x1  <<<  EGA
      golden_apple x1
      golden_pickaxe x1
      lodestone x1
  Portal at (160, 240)  Chest at (160, 240)  EGAs: 2
    Chest loot:
      golden_leggings x1
      enchanted_golden_apple x1  <<<  EGA
      golden_axe x1
      fire_charge x1
      enchanted_golden_apple x1  <<<  EGA
      obsidian x1
      golden_sword x1
      gold_ingot x5
========================================
```

**Results File:**
Found seeds are appended to `results.txt` in the format:
```
SEED PORTAL_X PORTAL_Z EGA_COUNT
```

**Notes:**
- The tool uses a per-chest threshold: a chest is only reported if it individually contains ≥ `min_egas` EGAs
- Multi-threaded searches are thread-safe (loot generation is properly synchronized)
- Default Minecraft version is 1.21.11 (latest)
- Searching large seed ranges with large distances can be slow due to many portal candidates per seed

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