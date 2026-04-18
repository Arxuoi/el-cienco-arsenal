#!/bin/bash

echo "========================================="
echo "       EL CIENCO ARSENAL BUILD"
echo "           Year 2310 Build"
echo "========================================="

# Detect OS
OS=$(uname -s)
ARCH=$(uname -m)

echo "[*] Detected OS: $OS"
echo "[*] Detected Architecture: $ARCH"

# Set compiler based on environment
if command -v clang &> /dev/null; then
    CC="clang"
    echo "[+] Using clang compiler"
elif command -v gcc &> /dev/null; then
    CC="gcc"
    echo "[+] Using gcc compiler"
else
    echo "[!] No C compiler found. Install gcc or clang."
    exit 1
fi

# Set flags
CFLAGS="-Wall -O3 -pthread -D_GNU_SOURCE"
LDFLAGS="-lpthread"

# Platform specific
if [[ "$OS" == "Linux" ]]; then
    if [[ "$ARCH" == "aarch64" ]] || [[ "$ARCH" == "arm"* ]]; then
        echo "[*] ARM/Linux detected (Termux/Android)"
        CFLAGS="$CFLAGS -DTERMUX"
    fi
elif [[ "$OS" == "Darwin" ]]; then
    echo "[*] macOS detected"
    CFLAGS="$CFLAGS -D__APPLE__"
fi

# Create build directory
BUILD_DIR="build"
mkdir -p $BUILD_DIR

echo "[*] Compiling source files..."

# Source files
SOURCES=(
    "src/main.c"
    "src/core/udp_flood.c"
    "src/core/tcp_syn.c"
    "src/core/http_flood.c"
    "src/core/slowloris.c"
    "src/core/dns_amp.c"
    "src/core/icmp_flood.c"
    "src/core/rudyloris.c"
    "src/core/cloudflare_bypass.c"
    "src/utils/socket_utils.c"
    "src/utils/packet_builder.c"
    "src/utils/ip_spoof.c"
    "src/utils/thread_pool.c"
    "api/server.c"
    "api/routes.c"
    "api/handlers.c"
)

# Compile
$CC $CFLAGS -Iinclude ${SOURCES[@]} -o $BUILD_DIR/elcienco $LDFLAGS

if [ $? -eq 0 ]; then
    echo "[+] Build successful!"
    echo "[+] Binary: $BUILD_DIR/elcienco"
    chmod +x $BUILD_DIR/elcienco
    
    # Display binary info
    echo ""
    echo "Binary Information:"
    file $BUILD_DIR/elcienco
    ls -lh $BUILD_DIR/elcienco
else
    echo "[!] Build failed!"
    exit 1
fi

echo ""
echo "Usage:"
echo "  $BUILD_DIR/elcienco [target] [port] [threads] [method] [duration]"
echo "  $BUILD_DIR/elcienco --api [port]"
echo ""
echo "Methods:"
echo "  0 - UDP Flood"
echo "  1 - TCP SYN Flood"
echo "  2 - HTTP Flood"
echo "  3 - Slowloris"
echo "  4 - DNS Amplification"
echo "  5 - ICMP Flood"
echo "  6 - RudyLoris"
echo "  7 - Cloudflare Bypass"
