# Debian Packages Source Code#!/bin/bash
# package/build_deb.sh

echo "[*] Setting permissions..."
chmod +x usr/bin/finndev
chmod +x DEBIAN/postinst

echo "[*] Building finndev deb package..."
dpkg-deb --build . ../finndev_1.0.0_all.deb

echo "[+] Package generated at parent directory: finndev_1.0.0_all.deb"
