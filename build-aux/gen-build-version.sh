#!/bin/sh
# Print version string derived from Git or .tarball-version
# If called with --dist, output a simplified version string (written to file if -o is used)

set -e

top_srcdir=$(dirname $0)/..

VERSION_PREFIX="1.14i-ac"
OUTPUT_FOR_DIST=false
OUTPUT_FILE=

while [ $# -gt 0 ]; do
  case "$1" in
    --dist)
      OUTPUT_FOR_DIST=true
      shift
      ;;
    -o)
      OUTPUT_FILE="$2"
      shift 2
      ;;
    *)
      echo "Usage: $0 [--dist] [-o <output-file>]" >&2
      exit 1
      ;;
  esac
done

# Ensure we are inside a clean Git work tree for --dist
check_git_clean_for_dist() {
  if test x"$DIRTY" != x; then
    echo "Error: working tree is dirty; commit or stash changes before running with --dist" >&2
    exit 1
  fi

  if test "$IS_INSIDE_WORK_TREE" != true; then
    echo "Error: not inside a Git repository" >&2
    exit 1
  fi
}

output_version() {
  if test x"$OUTPUT_FILE" = x && test -f $top_srcdir/.tarball-version; then
    version=$(cat $top_srcdir/.tarball-version)
  elif test "$OUTPUT_FOR_DIST" = true; then
    check_git_clean_for_dist
    version="${VERSION_PREFIX}${DATE}"
  else
    version="${VERSION_PREFIX}${DATE}-${HASH}${DIRTY}"
  fi

  if test x"$OUTPUT_FILE" != x; then
    echo $version > $OUTPUT_FILE
  else
    echo $version
  fi
}

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

output_version
