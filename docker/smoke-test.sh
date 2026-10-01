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

wait_for() {
    local seconds="$1"
    shift
    local deadline=$(( $(date +%s) + seconds ))
    until "$@" >/dev/null 2>&1; do
        [ "$(date +%s)" -lt "$deadline" ] || return 1
        sleep 2
    done
}

gate_port="${GATE_PUBLISHED_PORT:-1973}"
group_name="${GROUP_NAME:-GroupServerAndor2}"

"${compose[@]}" up --detach --build --wait --wait-timeout 600 account group gate || fail "postgres, account, group and gate become healthy"
pass "postgres, account, group and gate are healthy"

account_logs=$("${compose[@]}" logs --no-color account)
grep -Eq 'Connecting database \[postgres:5432 : AccountServer\]\.\.\. success!' <<<"$account_logs" \
    || fail "AccountServer reports a database connection"
"${compose[@]}" exec -T account grep -q 'connected to Auth database successfully' /var/log/tales/AccountServer/log/As2.log \
    || fail "AccountServer auth threads report database connections"
pass "AccountServer and its auth threads are connected to the database"

"${compose[@]}" exec -T account bash -c 'exec 3<>/dev/tcp/127.0.0.1/"$ACCOUNT_PORT"' \
    || fail "AccountServer accepts TCP connections"
pass "AccountServer accepts TCP connections"

wait_for 60 "${compose[@]}" exec -T account grep -qF "[$group_name] Add Successfully!" /var/log/tales/AccountServer/log/GroupServer.log \
    || fail "GroupServer logs in to AccountServer"
pass "GroupServer is logged in to AccountServer"

group_has_gate() {
    local logs
    logs=$("${compose[@]}" logs --no-color group)
    grep -Eq 'GateServer:[0-9.]+,[0-9]+login success' <<<"$logs"
}

wait_for 60 group_has_gate || fail "GateServer logs in to GroupServer"
pass "GateServer is logged in to GroupServer"

received=$(timeout 30 bash -c 'exec 3<>/dev/tcp/127.0.0.1/"$1"; head -c 2 <&3 | wc -c | tr -d " "' _ "$gate_port") || received=0
[ "$received" = 2 ] || fail "GateServer accepts a client on published port $gate_port and sends its first packet"
pass "GateServer accepts a client on published port $gate_port"

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
