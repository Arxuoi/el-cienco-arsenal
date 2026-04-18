#!/data/data/com.termux/files/usr/bin/bash

echo "[ El Cienco Arsenal - Termux Installation ]"

# Update packages
pkg update -y && pkg upgrade -y

# Install dependencies
pkg install git clang make cmake ninja build-essential -y
pkg install ndk-multilib -y
pkg install root-repo -y
pkg install tsu -y

# Clone repository
cd ~
git clone https://github.com/Arxuoi/el-cienco-arsenal
cd el-cienco-arsenal

# Build
make termux

# Install
make install

echo "[+] Installation complete!"
echo "Usage: elcienco [target] [port] [threads] [method] [duration]"
echo "API Mode: elcienco --api"
