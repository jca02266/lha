#!/bin/sh

# Exit on error
set -e

# Check for dependencies
if ! command -v git &>/dev/null; then
    echo "git is required but not found. Exiting."
    exit 1
fi

# Run autoreconf to generate configure script
autoreconf --force --install --symlink

# Run configure
mkdir build 2>/dev/null || true
cd build && ../configure "$@"
