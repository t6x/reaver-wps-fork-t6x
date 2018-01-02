#!/bin/sh

BINDIR=$(dirname $(readlink -f "$0"))

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
