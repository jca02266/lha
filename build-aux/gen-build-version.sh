#!/bin/sh
# Print version string derived from Git or .tarball-version
# If called with --dist, generate .tarball-version with simplified version

set -e

top_srcdir=$(dirname $0)/..

VERSION_PREFIX="1.14i-ac"

# command line switch
if test "$1" = "--dist"; then
  OUTPUT_FOR_DIST=true
fi

# Use .tarball-version if present
if test -s $top_srcdir/.tarball-version; then
  cat $top_srcdir/.tarball-version
  exit 0
fi

# Use Git info to generate full version
if git rev-parse --is-inside-work-tree >/dev/null 2>&1; then
  IS_INSIDE_WORK_TREE=true
  set -- $(git log -1 --date=format:"%Y%m%d" --pretty=format:"%cd %h")
  DATE=$1 HASH=$2

  DIRTY=""
  if ! git diff --quiet || ! git diff --cached --quiet; then
    DIRTY="-dirty"
    echo "Warning: repository is dirty (has uncommitted changes)" >&2
  fi
fi

# --dist: output version for .tarball-version and exit
if test "$OUTPUT_FOR_DIST" = true; then
  if test x"$DIRTY" != x; then
    exit 1
  fi

  if test "$IS_INSIDE_WORK_TREE" = true; then
    echo "${VERSION_PREFIX}${DATE}"
    exit 0
  else
    echo "Error: not inside a Git repository" >&2
    exit 1
  fi
fi

if test "$IS_INSIDE_WORK_TREE" = true; then
  echo "${VERSION_PREFIX}${DATE}-${HASH}${DIRTY}"
  exit 0
fi

# Fallback if nothing is available
echo "${VERSION_PREFIX}unknown"
exit 1
