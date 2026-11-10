#!/bin/sh
set -eu

port=$(printf ':%04X' "$1")
awk -v port="$port" '$4 == "0A" && substr($2, length($2) - 4) == port { found = 1 } END { exit !found }' /proc/net/tcp
