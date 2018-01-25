#!/bin/sh

argv0=$(echo "$0" | sed -e 's,\\,/,g')
BINDIR=$(dirname "$(readlink "$0" || echo "$argv0")")

if test -d "$BINDIR"/../.git ; then
if type git >/dev/null 2>&1 ; then
git describe --tags --match 'v[0-9]*' 2>/dev/null \
| sed -e 's/^v//' -e 's/-/-git-/'
else
sed 's/$/-git/' < VERSION
fi
else
cat VERSION
fi
