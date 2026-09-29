-- Creates the AccountServer service role and database, then loads schema and seed data.
-- Run as a PostgreSQL superuser from this folder:
--   psql -U postgres -f 01_create_account_database.sql
-- The database name is quoted because it is case-sensitive in the servers' connection string.
-- Dev-default password, change it for anything but a local install.

create role pko_account login password 'Y87dc#$98';
create database "AccountServer" owner pko_account template template0 encoding 'UTF8';

\connect "AccountServer"
set role pko_account;
\ir account/schema.sql
\ir account/seed.sql
