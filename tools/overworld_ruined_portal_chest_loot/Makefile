# Simple wrapper to build from the tool directory
# This delegates to the root Makefile

ROOT_DIR := ../..

.PHONY: all build clean

all: build

build:
	$(MAKE) -C $(ROOT_DIR) tools

clean:
	$(MAKE) -C $(ROOT_DIR) clean
