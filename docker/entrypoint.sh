#!/bin/sh
set -eu

if [ "$#" -lt 1 ]; then
    echo "usage: entrypoint.sh <server> [server arguments...]" >&2
    exit 64
fi

server="$1"
shift

run_dir="/opt/tales/$server"
template_dir="/opt/tales/templates/$server"

if [ ! -x "$run_dir/$server" ]; then
    echo "entrypoint: unknown server '$server'" >&2
    exit 64
fi

if [ -d "$template_dir" ]; then
    for template in "$template_dir"/*.template; do
        [ -e "$template" ] || continue
        variables=$(grep -o '\${[A-Za-z_][A-Za-z0-9_]*}' "$template" | sort -u)
        missing=""
        for variable in $variables; do
            name=${variable#??}
            name=${name%?}
            if ! printenv "$name" >/dev/null; then
                missing="$missing $name"
            fi
        done
        if [ -n "$missing" ]; then
            echo "entrypoint: $(basename "$template") needs these environment variables:$missing" >&2
            exit 78
        fi
        target="$run_dir/$(basename "$template" .template)"
        envsubst "$(echo $variables)" < "$template" > "$target"
    done
fi

cd "$run_dir"
exec "./$server" "$@"
