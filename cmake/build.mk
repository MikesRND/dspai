# CMake build system with preset management
# Uses .buildconfig.mk to track current preset

.PHONY: build-help preset-debug preset-release build test clean clean-all current

build-help:
	@echo "Build Commands:"
	@echo "  preset-debug   - Set debug as default preset"
	@echo "  preset-release - Set release as default preset"
	@echo "  build          - Build current configuration"
	@echo "  test           - Run tests for current configuration"
	@echo "  clean          - Clean current configuration"
	@echo "  clean-all      - Remove all build directories"
	@echo "  current        - Show current preset"

# Include current preset if it exists
-include .buildconfig.mk

# Default preset if not set
PRESET ?= release

# Set debug as default preset
preset-debug:
	@echo "PRESET=debug" > .buildconfig.mk
	@echo "Default preset set to: debug"

# Set release as default preset
preset-release:
	@echo "PRESET=release" > .buildconfig.mk
	@echo "Default preset set to: release"

# Build the current configuration (configure first if cache missing)
build:
	@if [ ! -f build/$(PRESET)/CMakeCache.txt ]; then \
		./cdo cmake --preset $(PRESET); \
	fi
	./cdo cmake --build --preset $(PRESET)

# Run tests for current configuration (build first if needed)
test: build
	./cdo ctest --test-dir build/$(PRESET) --output-on-failure

# Clean current configuration
clean:
	rm -rf build/$(PRESET)

# Remove all build directories
clean-all:
	rm -rf build/

# Show current preset
current:
	@echo "Current preset: $(PRESET)"
