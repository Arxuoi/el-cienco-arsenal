# ============================================
# EL CIENCO ARSENAL MAKEFILE
# Year 2310 Build System
# ============================================

# Compiler and flags
CC ?= gcc
CFLAGS = -Wall -O3 -pthread -D_GNU_SOURCE -Iinclude
LDFLAGS = -lpthread

# Target binary
TARGET = elcienco

# Directories
SRC_DIR = src
CORE_DIR = src/core
UTILS_DIR = src/utils
API_DIR = api
INCLUDE_DIR = include
BUILD_DIR = build

# Source files
CORE_SOURCES = $(wildcard $(CORE_DIR)/*.c)
UTILS_SOURCES = $(wildcard $(UTILS_DIR)/*.c)
API_SOURCES = $(filter-out $(API_DIR)/routes.c, $(wildcard $(API_DIR)/*.c))
# Filter out problematic files
UTILS_SOURCES := $(filter-out $(UTILS_DIR)/thread_pool.c, $(UTILS_SOURCES))

ALL_SOURCES = $(SRC_DIR)/main.c $(CORE_SOURCES) $(UTILS_SOURCES) $(API_SOURCES)

# Object files
OBJ_FILES = $(ALL_SOURCES:.c=.o)

# ============================================
# TARGETS
# ============================================

.PHONY: all clean termux install help

all: $(TARGET)

$(TARGET): $(OBJ_FILES)
	@echo "[LD] Linking $@"
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "[+] Build successful: $(TARGET)"
	@ls -lh $(TARGET)

%.o: %.c
	@echo "[CC] Compiling $<"
	$(CC) $(CFLAGS) -c -o $@ $<

# ============================================
# TERMUX SPECIFIC
# ============================================
termux:
	@echo "[*] Building for Termux/Android"
	@echo "[*] Using compiler: $(CC)"
	$(MAKE) clean
	$(MAKE) CC=clang all

# ============================================
# INSTALLATION
# ============================================
install:
	@if [ -d "/data/data/com.termux" ]; then \
		echo "[*] Termux detected, installing to $$PREFIX/bin"; \
		cp $(TARGET) $$PREFIX/bin/; \
		chmod +x $$PREFIX/bin/$(TARGET); \
		echo "[+] Installed to $$PREFIX/bin/$(TARGET)"; \
	elif [ -w "/usr/local/bin" ]; then \
		echo "[*] Installing to /usr/local/bin"; \
		cp $(TARGET) /usr/local/bin/; \
		chmod +x /usr/local/bin/$(TARGET); \
		echo "[+] Installed to /usr/local/bin/$(TARGET)"; \
	else \
		echo "[*] Installing to /usr/local/bin (sudo)"; \
		sudo cp $(TARGET) /usr/local/bin/; \
		sudo chmod +x /usr/local/bin/$(TARGET); \
		echo "[+] Installed to /usr/local/bin/$(TARGET)"; \
	fi

# ============================================
# CLEANUP
# ============================================
clean:
	@echo "[*] Cleaning build files..."
	rm -f $(TARGET)
	rm -f $(OBJ_FILES)
	@echo "[+] Clean complete"

distclean: clean
	rm -rf $(BUILD_DIR)

# ============================================
# HELP
# ============================================
help:
	@echo "=========================================="
	@echo "    EL CIENCO ARSENAL BUILD SYSTEM"
	@echo "=========================================="
	@echo ""
	@echo "Targets:"
	@echo "  make          - Build elcienco binary"
	@echo "  make termux   - Build for Termux (uses clang)"
	@echo "  make install  - Install to system PATH"
	@echo "  make clean    - Remove build artifacts"
	@echo "  make help     - Show this help"
	@echo ""
	@echo "Usage after build:"
	@echo "  ./elcienco --api [port]     - Start API server"
	@echo "  ./elcienco --help           - Show usage"
	@echo "  ./elcienco TARGET PORT THREADS METHOD DURATION"
	@echo ""
	@echo "Methods:"
	@echo "  0 = UDP Flood"
	@echo "  1 = TCP SYN Flood"
	@echo "  2 = HTTP Flood"
	@echo "  3 = Slowloris"
	@echo "  4 = DNS Amplification"
	@echo "  5 = ICMP Flood"
	@echo "  6 = RudyLoris"
	@echo "  7 = Cloudflare Bypass"
	@echo ""

# ============================================
# DEVELOPMENT TARGETS
# ============================================
dev: CFLAGS += -g -DDEBUG
dev: clean all
	@echo "[*] Development build complete"

check:
	@echo "[*] Checking source files..."
	@for f in $(ALL_SOURCES); do \
		if [ ! -f "$$f" ]; then \
			echo "[!] Missing: $$f"; \
		fi; \
	done
	@echo "[*] Source check complete"

# ============================================
# QUICK TEST
# ============================================
test: $(TARGET)
	@echo "[*] Running basic test..."
	./$(TARGET) --version
	./$(TARGET) --help | head -5

# ============================================
# PACKAGE
# ============================================
package: clean
	@echo "[*] Creating release package..."
	@mkdir -p $(BUILD_DIR)/el-cienco-arsenal
	@cp -r src include api web scripts Makefile README.md $(BUILD_DIR)/el-cienco-arsenal/
	@cd $(BUILD_DIR) && tar -czf el-cienco-arsenal.tar.gz el-cienco-arsenal/
	@echo "[+] Package created: $(BUILD_DIR)/el-cienco-arsenal.tar.gz"
