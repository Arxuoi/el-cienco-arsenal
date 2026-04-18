#!/bin/bash

echo "========================================="
echo "     EL CIENCO DEPLOYMENT SCRIPT"
echo "========================================="

DEPLOY_TYPE=${1:-"local"}
TARGET_HOST=${2:-""}
TARGET_USER=${3:-"root"}
TARGET_PATH=${4:-"/data/data/com.termux/files/usr/bin"}

deploy_local() {
    echo "[*] Deploying locally..."
    
    # Check if Termux
    if [ -d "/data/data/com.termux" ]; then
        echo "[+] Termux environment detected"
        cp build/elcienco $PREFIX/bin/
        chmod +x $PREFIX/bin/elcienco
        echo "[+] Installed to $PREFIX/bin/elcienco"
    else
        # Standard Linux
        sudo cp build/elcienco /usr/local/bin/
        sudo chmod +x /usr/local/bin/elcienco
        echo "[+] Installed to /usr/local/bin/elcienco"
    fi
}

deploy_remote() {
    if [ -z "$TARGET_HOST" ]; then
        echo "[!] Target host required for remote deployment"
        echo "Usage: ./deploy.sh remote <host> [user] [path]"
        exit 1
    fi
    
    echo "[*] Deploying to $TARGET_USER@$TARGET_HOST:$TARGET_PATH"
    
    # Check SSH connection
    ssh -o ConnectTimeout=5 $TARGET_USER@$TARGET_HOST "echo OK" 2>/dev/null
    if [ $? -ne 0 ]; then
        echo "[!] Cannot connect to $TARGET_HOST"
        exit 1
    fi
    
    # Copy binary
    scp build/elcienco $TARGET_USER@$TARGET_HOST:$TARGET_PATH/
    ssh $TARGET_USER@$TARGET_HOST "chmod +x $TARGET_PATH/elcienco"
    
    echo "[+] Deployed successfully to $TARGET_HOST"
}

deploy_android() {
    echo "[*] Preparing Android/ADB deployment..."
    
    if ! command -v adb &> /dev/null; then
        echo "[!] ADB not found. Install Android SDK Platform Tools."
        exit 1
    fi
    
    # Check device connection
    adb devices | grep -q "device$"
    if [ $? -ne 0 ]; then
        echo "[!] No Android device connected"
        exit 1
    fi
    
    echo "[+] Device connected"
    
    # Push binary
    adb push build/elcienco /data/local/tmp/
    adb shell "chmod 755 /data/local/tmp/elcienco"
    
    echo "[+] Binary pushed to /data/local/tmp/elcienco"
    echo "[*] Note: May require root for RAW socket operations"
}

case $DEPLOY_TYPE in
    "local")
        deploy_local
        ;;
    "remote")
        deploy_remote
        ;;
    "android")
        deploy_android
        ;;
    *)
        echo "Usage: $0 {local|remote|android} [host] [user] [path]"
        exit 1
        ;;
esac

echo ""
echo "[+] Deployment complete!"
echo ""
echo "Test installation:"
echo "  elcienco --help"
