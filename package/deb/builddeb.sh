#!/bin/bash
# Basic Debian package creation script for AppVeyor

echo "Creating basic .deb package from build directory..."

# Get version from appveyor environment or use default
VERSION=${APPVEYOR_BUILD_VERSION:-3.0.43}
PACKAGE_NAME="v8unpack"
MAINTAINER="Sergey Rudakov <fishcaroot@gmail.com>"

# Create basic package structure
mkdir -p build_deb/DEBIAN
mkdir -p build_deb/usr/bin
mkdir -p build_deb/usr/share/doc/v8unpack

# Copy executable
cp v8unpack build_deb/usr/bin/

# Copy documentation
cp ../../LICENSE build_deb/usr/share/doc/v8unpack/
cp ../../README.md build_deb/usr/share/doc/v8unpack/

# Create control file
cat > build_deb/DEBIAN/control << EOF
Package: ${PACKAGE_NAME}
Version: ${VERSION}
Architecture: amd64
Maintainer: ${MAINTAINER}
Description: 1C:Enterprise v8 configuration unpacker
 Utility for unpacking 1C:Enterprise v8 configuration files (.cf, .epf, .erf).
 Supports extraction, compression, deflate and metadata parsing.
EOF

# Build the package
dpkg-deb --build build_deb "${PACKAGE_NAME}-${VERSION}.deb"

# Move to parent directory
mv "${PACKAGE_NAME}-${VERSION}.deb" ..

# Cleanup
rm -rf build_deb

echo "Debian package ${PACKAGE_NAME}-${VERSION}.deb created successfully."
