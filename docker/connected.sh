#!/bin/sh
set -eu

port=$(printf ':%04X' "$1")
awk -v port="$port" '$4 == "01" && substr($3, length($3) - 4) == port { found = 1 } END { exit !found }' /proc/net/tcp
