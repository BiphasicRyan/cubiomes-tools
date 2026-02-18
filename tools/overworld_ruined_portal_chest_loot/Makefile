CC ?= gcc
CFLAGS ?= -O3 -Wall -Wextra -fwrapv
LDFLAGS ?= -lm -pthread

TARGET := overworld_ruined_portal_chest_loot
SRC := overworld_ruined_portal_chest_loot.c

ROOT := ../../cubiomes

all: $(TARGET)

$(ROOT)/libcubiomes.a:
	$(MAKE) -C $(ROOT)

$(TARGET): $(SRC) $(ROOT)/libcubiomes.a
	$(CC) $(CFLAGS) -I$(ROOT) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(TARGET)
