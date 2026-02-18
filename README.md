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
   ./tools/spawn_anchor/spawn_anchor 123456789 256
   ```

## Tools

### spawn_anchor
Finds ruined portals near spawn and analyzes their chest contents.

**Usage:**
```bash
./tools/spawn_anchor/spawn_anchor <seed> <radius_chunks>
```

**Parameters:**
- `<seed>`: Minecraft world seed (decimal or hexadecimal with 0x prefix)
- `<radius_chunks>`: Search radius in chunks from spawn point

**Example:**
```bash
./tools/spawn_anchor/spawn_anchor 123456789 256
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

2. Add your source files and a Makefile (see `tools/spawn_anchor/Makefile` for reference)

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
    └── spawn_anchor/
        ├── spawn_anchor.c   # Source code
        ├── Makefile        # Tool-specific build file
        └── spawn_anchor     # Compiled binary
```

### Submodule Issues

**Submodule directory is empty**
```bash
git submodule update --init --recursive
```