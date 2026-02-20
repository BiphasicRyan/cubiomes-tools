CC ?= gcc
CFLAGS ?= -O3 -Wall -Wextra -fwrapv
LDFLAGS ?= -lm -pthread

TARGET := overworld_ruined_portal_chest_loot
EGA_TARGET := ega_seed_finder
SRC := overworld_ruined_portal_chest_loot.c
EGA_SRC := ega_seed_finder.c
SOURCES := $(SRC) \
           parsing.c \
           loot_printer.c
EGA_SOURCES := $(EGA_SRC) \
               parsing.c \
               loot_printer.c

ROOT := ../../cubiomes

all: $(TARGET) $(EGA_TARGET)

$(ROOT)/libcubiomes.a:
	$(MAKE) -C $(ROOT)

$(TARGET): $(SOURCES) $(ROOT)/libcubiomes.a
	$(CC) $(CFLAGS) -I$(ROOT) -o $@ $(SOURCES) $(ROOT)/libcubiomes.a $(LDFLAGS)

$(EGA_TARGET): $(EGA_SOURCES) $(ROOT)/libcubiomes.a
	$(CC) $(CFLAGS) -I$(ROOT) -o $@ $(EGA_SOURCES) $(ROOT)/libcubiomes.a $(LDFLAGS)

clean:
	rm -f $(TARGET) $(EGA_TARGET) *.o
