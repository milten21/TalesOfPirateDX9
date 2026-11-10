#!/bin/sh
set -eu

for name in DB_HOST DB_PORT POSTGRES_PASSWORD ACCOUNT_DB_NAME ACCOUNT_DB_USER ACCOUNT_DB_PASSWORD GAME_DB_NAME GAME_DB_USER GAME_DB_PASSWORD; do
    if ! printenv "$name" >/dev/null; then
        echo "db-init: set $name" >&2
        exit 78
    fi
done

export PGHOST="$DB_HOST" PGPORT="$DB_PORT" PGUSER=postgres PGPASSWORD="$POSTGRES_PASSWORD" PGDATABASE=postgres PGOPTIONS="-c client_min_messages=warning"

sql() {
    psql --no-psqlrc --quiet --tuples-only --no-align -v ON_ERROR_STOP=1 "$@"
}

init_database() {
    database="$1" role="$2" password="$3" sql_dir="/opt/tales/postgresql/$4"

    sql -v role="$role" -v password="$password" <<'SQL'
select format('create role %I login', :'role') where not exists (select from pg_roles where rolname = :'role')
\gexec
alter role :"role" login password :'password';
SQL

    exists=$(sql -v database="$database" <<'SQL'
select 1 from pg_database where datname = :'database';
SQL
)
    if [ -n "$exists" ]; then
        echo "db-init: database $database exists, set the password of role $role"
        return
    fi

    staging="${database}_init"
    sql -v database="$staging" -v role="$role" <<'SQL'
drop database if exists :"database";
create database :"database" owner :"role" template template0 encoding 'UTF8';
SQL
    sql --dbname "$staging" -v role="$role" <<SQL
set role :"role";
\i $sql_dir/schema.sql
\i $sql_dir/seed.sql
SQL
    sql -v staging="$staging" -v database="$database" <<'SQL'
alter database :"staging" rename to :"database";
SQL
    echo "db-init: created database $database with role $role"
}

init_database "$ACCOUNT_DB_NAME" "$ACCOUNT_DB_USER" "$ACCOUNT_DB_PASSWORD" account
init_database "$GAME_DB_NAME" "$GAME_DB_USER" "$GAME_DB_PASSWORD" game
