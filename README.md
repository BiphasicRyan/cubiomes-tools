# Cubiomes Tools

A collection of tools built on top of the [cubiomes](https://github.com/Cubitect/cubiomes) library for Minecraft world generation analysis.

## Tools

### spawn_anchor
Finds ruined portals near spawn and analyzes their chest contents.

**Usage:**
```bash
./spawn_anchor <seed> <radius_chunks>
```

**Example:**
```bash
./spawn_anchor 123456789 256
```

## Building

This repository uses cubiomes as a git submodule. To build:

```bash
# Initialize the cubiomes submodule
git submodule update --init --recursive

# Build the cubiomes library
make -C cubiomes

# Build the tools
make
```

## Dependencies

- GCC (or compatible C compiler)
- Make
- pthread library
- math library

## License

This project follows the same license as the cubiomes library.
