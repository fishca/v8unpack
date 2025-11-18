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

# Copy documentation (find files relative to the script location)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$SCRIPT_DIR/../.."

# Copy documentation if files exist
if [ -f "$PROJECT_ROOT/LICENSE" ]; then
    cp "$PROJECT_ROOT/LICENSE" build_deb/usr/share/doc/v8unpack/
fi

if [ -f "$PROJECT_ROOT/README.md" ]; then
    cp "$PROJECT_ROOT/README.md" build_deb/usr/share/doc/v8unpack/
fi

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

# Build the package in current directory
dpkg-deb --build build_deb "${PACKAGE_NAME}-${VERSION}.deb"

# Move to parent directory if file was created
if [ -f "${PACKAGE_NAME}-${VERSION}.deb" ]; then
    mv "${PACKAGE_NAME}-${VERSION}.deb" ..
    echo "Debian package ${PACKAGE_NAME}-${VERSION}.deb created successfully."
else
    echo "Error: Debian package was not created."
    exit 1
fi

# Cleanup
rm -rf build_deb

echo "Packaging completed."
