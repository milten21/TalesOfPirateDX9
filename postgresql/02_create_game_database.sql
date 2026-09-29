-- Creates the GameDB service role and database, then loads schema and seed data.
-- Run as a PostgreSQL superuser from this folder:
--   psql -U postgres -f 02_create_game_database.sql
-- The database name is quoted because it is case-sensitive in the servers' connection string.
-- Dev-default password, change it for anything but a local install.

create role pko_game login password 'Y87dc#$98';
create database "GameDB" owner pko_game template template0 encoding 'UTF8';

\connect "GameDB"
set role pko_game;
\ir game/schema.sql
\ir game/seed.sql
