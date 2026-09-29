-- GameDB seed rows: the 199 guild slots (ids 0 to 198) GroupServer requires at startup.
insert into guild (guild_id, guild_name)
select id, 'Pirate Guild ' || id from generate_series(0, 198) as id;
