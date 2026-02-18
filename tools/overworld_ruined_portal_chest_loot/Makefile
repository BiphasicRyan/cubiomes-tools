CC ?= gcc
CFLAGS ?= -O3 -Wall -Wextra -fwrapv
LDFLAGS ?= -lm -pthread

TARGET := overworld_ruined_portal_chest_loot
SRC := overworld_ruined_portal_chest_loot.c
SOURCES := $(SRC) \
           parsing.c \
           loot_printer.c

ROOT := ../../cubiomes

all: $(TARGET)

$(ROOT)/libcubiomes.a:
	$(MAKE) -C $(ROOT)

$(TARGET): $(SOURCES) $(ROOT)/libcubiomes.a
	$(CC) $(CFLAGS) -I$(ROOT) -o $@ $(SOURCES) $(ROOT)/libcubiomes.a $(LDFLAGS)

clean:
	rm -f $(TARGET) *.o
