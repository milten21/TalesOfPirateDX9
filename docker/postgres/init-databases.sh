#!/bin/sh
(
set -eu

: "${ACCOUNT_DB_PASSWORD:?set ACCOUNT_DB_PASSWORD}"
: "${GAME_DB_PASSWORD:?set GAME_DB_PASSWORD}"

cd /opt/tales/postgresql
psql -v ON_ERROR_STOP=1 --username "$POSTGRES_USER" --dbname "$POSTGRES_DB" --no-psqlrc -f 01_create_account_database.sql
psql -v ON_ERROR_STOP=1 --username "$POSTGRES_USER" --dbname "$POSTGRES_DB" --no-psqlrc -f 02_create_game_database.sql

psql -v ON_ERROR_STOP=1 --username "$POSTGRES_USER" --dbname "$POSTGRES_DB" --no-psqlrc \
    -v account_password="$ACCOUNT_DB_PASSWORD" -v game_password="$GAME_DB_PASSWORD" <<'SQL'
alter role pko_account password :'account_password';
alter role pko_game password :'game_password';
SQL
)
