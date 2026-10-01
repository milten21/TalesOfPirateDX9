#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")/.."
export MSYS_NO_PATHCONV=1

project="${SMOKE_PROJECT:-tales-smoke}"
env_file="${SMOKE_ENV_FILE:-.env.example}"
compose=(docker compose --project-name "$project" --env-file "$env_file")

cleanup() {
    "${compose[@]}" down --volumes --remove-orphans >/dev/null 2>&1 || true
}

fail() {
    echo "not ok - $*" >&2
    "${compose[@]}" logs --no-color --tail 200 >&2 || true
    exit 1
}

pass() {
    echo "ok - $*"
}

trap cleanup EXIT
cleanup

"${compose[@]}" up --detach --build --wait --wait-timeout 600 account || fail "postgres and account become healthy"
pass "postgres and account are healthy"

account_logs=$("${compose[@]}" logs --no-color account)
grep -Eq 'Connecting database \[postgres:5432 : AccountServer\]\.\.\. success!' <<<"$account_logs" \
    || fail "AccountServer reports a database connection"
"${compose[@]}" exec -T account grep -q 'connected to Auth database successfully' /var/log/tales/AccountServer/log/As2.log \
    || fail "AccountServer auth threads report database connections"
pass "AccountServer and its auth threads are connected to the database"

"${compose[@]}" exec -T account bash -c 'exec 3<>/dev/tcp/127.0.0.1/"$ACCOUNT_PORT"' \
    || fail "AccountServer accepts TCP connections"
pass "AccountServer accepts TCP connections"

containers=$("${compose[@]}" ps --quiet)
started=$(date +%s)
"${compose[@]}" stop >/dev/null || fail "docker compose stop succeeds"
stopped=$(date +%s)
for container in $containers; do
    read -r name exit_code < <(docker inspect --format '{{index .Config.Labels "com.docker.compose.service"}} {{.State.ExitCode}}' "$container")
    [ "$exit_code" = 0 ] || fail "$name exits with code 0 on stop (got $exit_code)"
    pass "$name stopped cleanly with exit code 0"
done
pass "the stack stopped in $((stopped - started))s"

echo "smoke test passed"
