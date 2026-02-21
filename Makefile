CC ?= gcc
CFLAGS ?= -O3 -Wall -Wextra -fwrapv
LDFLAGS ?= -lm -pthread

# Directories
TOOLS_DIR := tools
CUBIOMES_DIR := cubiomes

# Targets
TOOLS := $(TOOLS_DIR)/overworld_ruined_portal_chest_loot/overworld_ruined_portal_chest_loot

.PHONY: all clean cubiomes tools

all: cubiomes tools

cubiomes: $(CUBIOMES_DIR)/libcubiomes.a

$(CUBIOMES_DIR)/libcubiomes.a:
	@if [ ! -d "$(CUBIOMES_DIR)" ]; then \
		echo "cubiomes submodule not found. Run:"; \
		echo "git submodule update --init --recursive"; \
		exit 1; \
	fi
	$(MAKE) -C $(CUBIOMES_DIR)

tools: $(TOOLS)

$(TOOLS_DIR)/overworld_ruined_portal_chest_loot/overworld_ruined_portal_chest_loot: $(TOOLS_DIR)/overworld_ruined_portal_chest_loot/overworld_ruined_portal_chest_loot.c $(TOOLS_DIR)/overworld_ruined_portal_chest_loot/parsing.c
	$(CC) $(CFLAGS) -I$(CUBIOMES_DIR) -o $@ $(TOOLS_DIR)/overworld_ruined_portal_chest_loot/overworld_ruined_portal_chest_loot.c $(TOOLS_DIR)/overworld_ruined_portal_chest_loot/parsing.c $(LDFLAGS)

clean:
	rm -f $(TOOLS)
	$(MAKE) -C $(CUBIOMES_DIR) clean 2>/dev/null || true

help:
	@echo "Available targets:"
	@echo "  all      - Build cubiomes library and all tools"
	@echo "  cubiomes - Build cubiomes library only"
	@echo "  tools    - Build tools only"
	@echo "  clean    - Remove built files"
	@echo "  help     - Show this help"
