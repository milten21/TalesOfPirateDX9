#ifndef GAMESQL_H_
#define GAMESQL_H_

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <string>

namespace game_sql
{

struct Select
{
	std::string table;
	std::string columns;
	std::string filter;
};

inline std::string format(const char* fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	int n = _vscprintf(fmt, ap);
	va_end(ap);
	if (n < 0) return std::string();
	std::string s((size_t)n, '\0');
	va_start(ap, fmt);
	_vsnprintf(&s[0], (size_t)n + 1, fmt, ap);
	va_end(ap);
	return s;
}

template<size_t N>
inline void copy_to(char (&dst)[N], const std::string& s)
{
	strncpy_s(dst, N, s.c_str(), _TRUNCATE);
}

inline Select make_select(const char* table, const std::string& columns, const std::string& filter)
{
	Select s;
	s.table = table;
	s.columns = columns;
	s.filter = filter;
	return s;
}

namespace columns
{
	const char master[] = "cha_id1, cha_id2, finish, relation";
	const char character[] =
		"atorID, atorNome, motto, icon, version, pk_ctrl, endeMem, ato_id, guild_id, guild_stat, guild_permission, job, degree, exp, "
		"hp, sp, ap, tp, bomd, str, dex, agi, con, sta, luk, sail_lv, sail_exp, sail_left_exp, live_lv, live_exp, map, main_map, map_x, map_y, radius, "
		"angle, olhe, skillbag, shortcut, mission, misrecord, mistrigger, miscount, birth, login_cha, live_tp, bank, "
		"delflag, operdate, skill_state, kitbag, kitbag_tmp, kb_locked, credit, store_item";
	const char character_load[] =
		"ato_id, guild_stat, guild_id, hp, sp, exp, radius, angle, atorNome, motto, icon, version, pk_ctrl, degree, job, bomd, ap, tp, str, dex, agi, con, sta, luk, "
		"sail_lv, sail_exp, sail_left_exp, live_lv, live_exp, live_tp, main_map, map_x, map_y, birth, olhe, skillbag, shortcut, mission, misrecord, mistrigger, miscount, "
		"login_cha, kitbag, kitbag_tmp, map_mask, skill_state, bank, kb_locked, credit, store_item, extend, guild_permission, chatColour, IMP";
	const char lottery[] = "section";
	const char ticket[] = "itemno";
	const char winticket[] = "issue";
	const char amphitheater_setting[] = "season";
	const char amphitheater_team[] = "id";
	const char personinfo[] = "atorID";
	const char resource[] = "id, atorID, type_id, content";
	const char map_mask[] = "id, atorID, content1, content2, content3, content4, content5";
	const char account[] = "ato_id, ato_nome, jmes, ator_ids, last_ip, disc_reason, last_leave";
	const char boat[] =
		"boat_id, boat_berth, boat_name, boat_boatid, boat_header, boat_body, boat_engine, boat_cannon, boat_equipment, "
		"boat_bag, boat_diecount, boat_isdead, boat_ownerid, boat_createtime, boat_isdeleted, cur_endure, mx_endure, cur_supply, mx_supply, skill_state, "
		"map, map_x, map_y, angle, degree, exp";
	const char boat_load[] =
		"boat_name, boat_boatid, boat_berth, boat_header, boat_body, boat_engine, boat_cannon, boat_equipment, boat_diecount, boat_isdead, "
		"boat_ownerid, boat_isdeleted, cur_endure, mx_endure, cur_supply, mx_supply, skill_state, map, map_x, map_y, angle, degree, exp";
	const char guild[] = "guild_id, guild_name, motto, passwd, leader_id, exp, member_total, try_total, disband_date";
	const char guild_name[] = "guild_name";
	const char guild_name_motto[] = "guild_name, motto";
	const char guild_bank[] = "bank";
	const char guild_leader[] = "leader_id";
	const char guild_gold[] = "gold";
	const char guild_bank_log[] = "banklog";
	const char guild_challenge_id[] = "challlevel";
	const char guild_challenge_full[] = "guild_id, guild_name, challid, challmoney, leader_id, challstart";
	const char guild_challenge_pair[] = "guild_id, guild_name, challid, challmoney";
	const char guild_challenge_call[] = "guild_id, guild_name, challid, challmoney, challstart";
	const char guild_challenge_end[] = "challstart, guild_name, challid, challmoney, challlevel";
	const char guild_challenge_info[] = "guild_id, challid, challmoney";
	const char try_player[] = "g.guild_id, g.guild_name,g.motto, c.atorNome, g.member_total,g.exp, g.level";
}

inline std::string probe(const char* table, const char* cols)
{
	return format("select %s from %s where 1=2", cols, table);
}


inline Select character_name_exists(const char* name)
{
	return make_select("character", "atorNome", format("atorNome='%s'", name));
}

inline Select character_name_by_id(int id)
{
	return make_select("character", "atorNome", format("atorID = %d", id));
}

inline Select master_of(int cha_id)
{
	return make_select("master", "cha_id2", format("cha_id1=%d", cha_id));
}

inline Select master_relation_count(int prentice_id, int master_id)
{
	return make_select("master", "count(*)", format("(cha_id1=%d) and (cha_id2=%d)", prentice_id, master_id));
}

inline std::string exp_rank(const char* table, int count)
{
	return format("select atorNome,job,degree from %s where delflag =0 "
		"ORDER BY CASE WHEN (exp < 0) THEN (exp+4294967296) ELSE exp END DESC LIMIT %d", table, count);
}

inline Select character_load(int atorID)
{
	return make_select("character", columns::character_load, format("atorID=%d", atorID));
}

struct CharacterSave
{
	bool with_pos;
	int atorID;
	int hp, sp;
	const char* exp;
	const char* map;
	const char* main_map;
	int map_x, map_y, radius, angle, pk_ctrl, degree;
	const char* job;
	int bomd, ap, tp, str, dex, agi, con, sta, luk;
	const char* olhe;
	const char* skillbag;
	const char* shortcut;
	const char* mission;
	const char* misrecord;
	const char* mistrigger;
	const char* miscount;
	const char* birth;
	const char* login_cha;
	int sail_lv, sail_exp, sail_left_exp, live_lv, live_exp, live_tp, kb_locked, credit, store_item;
	const char* skill_state;
	const char* extend;
	int imp;
};

inline std::string save_character(const char* table, const CharacterSave& c)
{
	std::string s = format("update %s set hp=%d, sp=%d, exp=%s, ", table, c.hp, c.sp, c.exp);
	if (c.with_pos)
		s += format("map='%s', main_map='%s', map_x=%d, map_y=%d, ", c.map, c.main_map, c.map_x, c.map_y);
	s += format("radius=%d, ", c.radius);
	if (c.with_pos)
		s += format("angle=%d, ", c.angle);
	s += format("pk_ctrl=%d, degree=%d, job='%s', bomd=%d, ap=%d, tp=%d, str=%d, dex=%d, agi=%d, con=%d, sta=%d, luk=%d, "
		"olhe='%s', skillbag='%s', shortcut='%s', mission='%s', misrecord='%s', mistrigger='%s', miscount='%s', birth='%s', login_cha='%s', "
		"sail_lv=%d, sail_exp=%d, sail_left_exp=%d, live_lv=%d, live_exp=%d, live_tp=%d, kb_locked=%d, credit=%d, store_item=%d, "
		"skill_state='%s', extend ='%s', IMP = '%d' where atorID=%d",
		c.pk_ctrl, c.degree, c.job, c.bomd, c.ap, c.tp, c.str, c.dex, c.agi, c.con, c.sta, c.luk,
		c.olhe, c.skillbag, c.shortcut, c.mission, c.misrecord, c.mistrigger, c.miscount, c.birth, c.login_cha,
		c.sail_lv, c.sail_exp, c.sail_left_exp, c.live_lv, c.live_exp, c.live_tp, c.kb_locked, c.credit, c.store_item,
		c.skill_state, c.extend, c.imp, c.atorID);
	return s;
}

inline std::string save_position(const char* table, const char* map, const char* main_map, int x, int y, int angle, int atorID)
{
	return format("update %s set map='%s', main_map='%s', map_x=%d, map_y=%d, angle=%d where atorID=%d",
		table, map, main_map, x, y, angle, atorID);
}

inline std::string save_money(const char* table, int gold, int atorID)
{
	return format("update %s set bomd=%d where atorID=%d", table, gold, atorID);
}

inline std::string save_kitbag_id(const char* table, int resource_id, int atorID)
{
	return format("update %s set kitbag='%d' where atorID=%d", table, resource_id, atorID);
}

inline std::string save_kitbag_tmp_id(const char* table, int resource_id, int atorID)
{
	return format("update %s set kitbag_tmp=%d where atorID=%d", table, resource_id, atorID);
}

inline std::string save_kitbag_lock(const char* table, int locked, int atorID)
{
	return format("update %s set kb_locked=%d where atorID=%d", table, locked, atorID);
}

inline std::string save_store_item(const char* table, int item, int atorID)
{
	return format("update %s set store_item=%d where atorID=%d", table, item, atorID);
}

inline std::string add_money(const char* table, int money, int atorID)
{
	return format("update %s set bomd=bomd+%d where atorID=%d", table, money, atorID);
}

inline std::string add_credit(const char* table, int credit, int atorID)
{
	return format("update %s set credit=credit+%d where atorID=%d", table, credit, atorID);
}

inline std::string character_address(const char* table, int atorID)
{
	return format("select endeMem from %s where atorID=%d", table, atorID);
}

inline std::string save_map_mask_id(const char* table, int resource_id, int atorID)
{
	return format("update %s set map_mask='%d' where atorID=%d", table, resource_id, atorID);
}

inline std::string save_bank_ids(const char* table, const char* ids, int atorID)
{
	return format("update %s set bank='%s' where atorID=%d", table, ids, atorID);
}

inline std::string save_table_version(const char* table, int version, int atorID)
{
	return format("update %s set version=%d where atorID=%d", table, version, atorID);
}

inline std::string save_missions(const char* table, const char* mission, const char* record, const char* trigger, int atorID)
{
	return format("update %s set mission='%s', misrecord='%s', mistrigger='%s' where atorID=%d",
		table, mission, record, trigger, atorID);
}

inline std::string set_guild_permission(const char* table, unsigned long permission, int atorID, int guild_id)
{
	return format("update %s set guild_permission='%d' where atorID=%d and guild_id = %d", table, permission, atorID, guild_id);
}

inline std::string reset_character_address(const char* table, int atorID)
{
	return format("update %s set endeMem='%d' where atorID=%d", table, -1, atorID);
}

inline std::string set_character_guild(int guild_id, int permission, int atorID)
{
	return format("update character set guild_id =%d,guild_stat =0,guild_permission =%d where atorID =%d", guild_id, permission, atorID);
}


inline Select account_gm_level(int ato_id)
{
	return make_select("account", "jmes, ato_nome, IMP", format("ato_id=%d", ato_id));
}

inline std::string save_imp(int imp, int atorID)
{
	return format("Update %s set IMP = %d where atorID = %d;", "character", imp, atorID);
}

inline std::string save_gm_level(const char* table, int level, int ato_id)
{
	return format("Update %s set jmes = %d where ato_id = %d;", table, level, ato_id);
}


inline Select lottery_current_issue(int state)
{
	return make_select("LotterySetting", "issue", format("state  =%d", state));
}

inline Select lottery_win_item_no(int state, int issue)
{
	return make_select("LotterySetting", "itemno", format("state  =%d and issue = %d", state, issue));
}

inline std::string lottery_add_issue(const char* table, int section, int issue, int state)
{
	return format("insert into %s (section, issue, state, createdate, updatetime) values (%d, %d, %d, now(), now())",
		table, section, issue, state);
}

inline std::string lottery_set_state(const char* table, int state, int issue)
{
	return format("update %s set state = %d, updatetime = now() where issue = %d", table, state, issue);
}

inline std::string lottery_set_win_item_no(const char* table, const char* itemno, int issue)
{
	return format("update %s set itemno = '%s', updatetime = now() where issue = %d", table, itemno, issue);
}

inline std::string ticket_add(const char* table, int atorID, int issue, char n1, char n2, char n3, char n4, char n5, char n6, int real)
{
	return format("insert into %s (atorID, issue, itemno, real, buydate) values (%d, '%d', '%c%c%c%c%c%c', %d, now())",
		table, atorID, issue, n1, n2, n3, n4, n5, n6, real);
}

inline Select ticket_exists(int issue, const char* itemno)
{
	return make_select("Ticket", "count(*)", format(" issue = %d and itemno  ='%s'", issue, itemno));
}

inline std::string ticket_least_bought(int issue, int max)
{
	return format("SELECT * FROM (SELECT itemno, COUNT(*) AS num FROM Ticket WHERE (issue = %d) AND real = 0 GROUP BY itemno) AS A "
		"WHERE num <= %d ORDER BY num LIMIT 10", issue, max);
}

inline std::string win_ticket_add(const char* table, int issue, const char* itemno, int grade)
{
	return format("insert into %s (issue, itemno, grade, createdate) values (%d, '%s', %d, now())", table, issue, itemno, grade);
}

inline std::string win_ticket_exchange(const char* table, int issue, const char* itemno)
{
	return format("update %s set num = num + 1 where issue = %d and itemno = '%s'", table, issue, itemno);
}


inline Select amphitheater_current_season(int state)
{
	return make_select("AmphitheaterSetting", "season, \"round\"", format("state  =%d", state));
}

inline std::string amphitheater_add_season(const char* table, int section, int season, int round, int state)
{
	return format("insert into %s (section, season, \"round\", state, createdate, updatetime,winner) values (%d, %d, %d, %d, now(), now(),%s)",
		table, section, season, round, state, "null");
}

inline std::string amphitheater_close_season(const char* table, int state, const char* winner, int season)
{
	return format("update %s set state = %d, updatetime = now(),winner = (%s)::text where season = %d", table, state, winner, season);
}

inline std::string amphitheater_set_round(const char* table, int round, int season)
{
	return format("update %s set \"round\" = %d, updatetime = now() where season = %d", table, round, season);
}

inline Select amphitheater_team_count(int not_used_state)
{
	return make_select("AmphitheaterTeam", "count(*)", format("state > %d", not_used_state));
}

inline Select amphitheater_free_team(int not_used_state)
{
	return make_select("AmphitheaterTeam", "id", format("state = %d ORDER BY id LIMIT 1", not_used_state));
}

inline std::string amphitheater_sign_up(const char* table, int captain, const char* member, int state, int team)
{
	return format("update %s set captain = %d, member = '%s', state = %d, updatetime = now() where id = %d",
		table, captain, member, state, team);
}

inline std::string amphitheater_cancel(const char* table, int state, int team)
{
	return format("update %s set captain = null, member = null, matchno = 0, state = %d, updatetime = now() where id = %d",
		table, state, team);
}

inline Select amphitheater_valid_team(int team, int captain, int member1, int member2)
{
	return make_select("AmphitheaterTeam", "count(*)",
		format("id = %d and captain = %d and (member = '%d,%d' or member = '%d,%d')", team, captain, member1, member2, member2, member1));
}

inline Select amphitheater_registered(int actor)
{
	return make_select("AmphitheaterTeam", "count(*)",
		format("captain = %d or member like '%d,%%' or member like '%%,%d'", actor, actor, actor));
}

inline Select amphitheater_map_teams(int map)
{
	return make_select("AmphitheaterTeam", "count(*)", format("map = %d ", map));
}

inline std::string amphitheater_set_map_flag(const char* table, int flag, int team, int map)
{
	return format("update %s set mapflag = %d where id = %d and map = %d", table, flag, team, map);
}

inline Select amphitheater_map_flag(int team)
{
	return make_select("AmphitheaterTeam", "mapflag", format("id = %d ", team));
}

inline Select amphitheater_state_count(int state)
{
	return make_select("AmphitheaterTeam", "count(*)", format("state = %d", state));
}

inline std::string amphitheater_promote_top(const char* table, int promotion_state, int count, int relive_state)
{
	return format("update %s set state = %d ,relivenum = 0 where id in (select id from %s where state = %d order by relivenum desc, id limit %d)",
		table, promotion_state, table, relive_state, count);
}

inline std::string amphitheater_eliminate(const char* table, int out_state, int relive_state, int use_state)
{
	return format("update %s set state = %d ,relivenum = 0 where state = %d or state = %d", table, out_state, relive_state, use_state);
}

inline std::string amphitheater_set_state(const char* table, int state, int team)
{
	return format("update %s set state = %d where id = %d", table, state, team);
}

inline std::string amphitheater_map_captains(const char* table, int map)
{
	return format("select captain from %s where map = %d", table, map);
}

inline std::string amphitheater_clear_map(const char* table, int map)
{
	return format("update %s set map = null where map = %d  ", table, map);
}

inline std::string amphitheater_promoted_teams(int promotion_state)
{
	return format("select B.atorNome,A.id,A.winnum from AmphitheaterTeam A, character B where B.atorID = A.captain and a.state = %d  order by A.winnum DESC",
		promotion_state);
}

inline std::string amphitheater_relive_teams(int relive_state)
{
	return format("select B.atorNome, A.relivenum ,A.id from AmphitheaterTeam A,  character B where B.atorID = A.captain and A.state = %d  order by A.relivenum DESC",
		relive_state);
}

inline Select amphitheater_relive_number(int team)
{
	return make_select("AmphitheaterTeam", "relivenum", format("id = %d ", team));
}

inline std::string amphitheater_set_relive_number(const char* table, int number, int team)
{
	return format("update %s set relivenum = %d where id = %d ", table, number, team);
}

inline std::string amphitheater_use_to_relive(const char* table, int relive_state, int use_state)
{
	return format("update %s set state = %d where state = %d ", table, relive_state, use_state);
}

inline std::string amphitheater_set_captain_map(const char* table, int map, int captain)
{
	return format("update %s set map = %d where captain = %d ", table, map, captain);
}

inline std::string amphitheater_add_win(const char* table, int team)
{
	return format("update %s set winnum = winnum+1 where id = %d ", table, team);
}

inline Select amphitheater_top_winner()
{
	return make_select("AmphitheaterTeam", "id", "winnum in (select  max(winnum) from AmphitheaterTeam)");
}

inline std::string amphitheater_set_matchno(const char* table, int team)
{
	return format("update %s set matchno = 1 where id = %d ", table, team);
}

inline std::string amphitheater_promotion_to_use(const char* table, int use_state, int promotion_state)
{
	return format("update %s set state = %d where state= %d ", table, use_state, promotion_state);
}

inline std::string amphitheater_clear_map_flags(const char* table, int team1, int team2)
{
	return format("update %s set mapflag = null where id= %d or id = %d", table, team1, team2);
}

inline Select amphitheater_team_state(int team)
{
	return make_select("AmphitheaterTeam", "state", format("id = %d ", team));
}

inline Select person_birthday(int atorID)
{
	return make_select("personinfo", "birthday", format("atorID = %d ", atorID));
}


inline std::string resource_create(const char* table, int atorID, int type_id)
{
	return format("insert into %s (atorID, type_id) values(%d, %d) ", table, atorID, type_id);
}

inline Select resource_by_id(int id)
{
	return make_select("resource", "atorID, type_id, content", format("id=%d", id));
}

inline std::string resource_content(const char* table, int id)
{
	return format("select content from %s where id=%d", table, id);
}

inline std::string resource_save_content(const char* table, const char* content, int id)
{
	return format("update %s set content='%s' where id=%d", table, content, id);
}

inline std::string map_mask_create(const char* table, int atorID)
{
	return format("insert into %s (atorID) values(%d) ", table, atorID);
}

inline Select map_mask_by_id(const char* column, int id)
{
	return make_select("map_mask", format("atorID, %s", column), format("id=%d", id));
}

inline std::string map_mask_save(const char* table, const char* column, const char* content, int id)
{
	return format("update %s set %s='%s' where id=%d", table, column, content, id);
}


struct BoatCreate
{
	const char* name;
	int berth, boat, header, body, engine, cannon, equipment;
	const char* bag;
	int owner;
	const char* created;
};

inline std::string boat_create(const char* table, const BoatCreate& b)
{
	return format("insert into %s (boat_name, boat_berth, boat_boatid, boat_header, boat_body, boat_engine, boat_cannon, boat_equipment, "
		"boat_bag, boat_diecount, boat_isdead, boat_ownerid, boat_createtime, boat_isdeleted) "
		"values( '%s', %d, %d, %d, %d, %d, %d, %d, '%s', %d, '%d', %d, '%s', '%d' ) ",
		table, b.name, b.berth, b.boat, b.header, b.body, b.engine, b.cannon, b.equipment, b.bag, 0, 0, b.owner, b.created, 0);
}

inline Select boat_by_id(int id)
{
	return make_select("boat", columns::boat_load, format("boat_id=%d", id));
}

inline Select boat_cabin(int id)
{
	return make_select("boat", "boat_bag", format("boat_id=%d", id));
}

inline std::string boat_set_owner(const char* table, int owner, int deleted, int id)
{
	return format("update %s set boat_ownerid=%d, boat_isdeleted='%d' where boat_id=%d", table, owner, deleted, id);
}

inline std::string boat_set_deleted(const char* table, int deleted, int id)
{
	return format("update %s set boat_isdeleted='%d' where boat_id=%d", table, deleted, id);
}

inline std::string boat_set_death(const char* table, int die_count, int dead, int owner, int deleted, int id)
{
	return format("update %s set boat_diecount=%d, boat_isdead='%d' , boat_ownerid=%d, boat_isdeleted='%d' where boat_id=%d",
		table, die_count, dead, owner, deleted, id);
}

struct BoatSave
{
	bool with_pos;
	int id, berth, owner, cur_endure, mx_endure, cur_supply, mx_supply;
	const char* skill_state;
	const char* map;
	int map_x, map_y, angle, degree, exp;
	const char* bag;
};

inline std::string boat_save(const char* table, const BoatSave& b)
{
	std::string s = format("update %s set boat_berth=%d, boat_ownerid=%d, cur_endure=%d, mx_endure=%d, cur_supply=%d, mx_supply=%d, skill_state='%s', ",
		table, b.berth, b.owner, b.cur_endure, b.mx_endure, b.cur_supply, b.mx_supply, b.skill_state);
	if (b.with_pos)
		s += format("map='%s', map_x=%d, map_y=%d, angle=%d, ", b.map, b.map_x, b.map_y, b.angle);
	s += format("degree=%d, exp=%d, boat_bag='%s' where boat_id=%d", b.degree, b.exp, b.bag, b.id);
	return s;
}

inline std::string boat_save_cabin(const char* table, const char* bag, int id)
{
	return format("update %s set boat_bag='%s' where boat_id=%d", table, bag, id);
}


inline Select guild_by_id(const char* cols, int guild_id)
{
	return make_select("guild", cols, format("guild_id =%d", guild_id));
}

inline Select guild_by_challenger(const char* cols, int guild_id)
{
	return make_select("guild", cols, format("challid =%d", guild_id));
}

inline Select guild_by_challenge_level(const char* cols, int level)
{
	return make_select("guild", cols, format("challlevel =%d", level));
}

inline Select guild_free_slot()
{
	return make_select("guild", "coalesce(min(guild_id),0)", "guild_id >0 and leader_id =0");
}

inline Select guild_open_for_applications(int guild_id)
{
	return make_select("guild", "guild_id", format("leader_id >0 and guild_id =%d", guild_id));
}

inline std::string guild_found(const char* table, int leader, const char* passwd, const char* name, int guild_id)
{
	return format("update %s set leader_id =%d ,passwd ='%s', guild_name ='%s', exp =0, member_total =1,try_total =0 where leader_id =0 and guild_id =%d",
		table, leader, passwd, name, guild_id);
}

inline std::string guild_list_all()
{
	return "select gld.guild_id, gld.guild_name, gld.motto, gld.leader_id, cha.atorNome leader_name, gld.exp, gld.member_total "
		"from guild As gld, character As cha where gld.leader_id =cha.atorID";
}

inline Select guild_applicant_guild(int atorID, int guild_id)
{
	return make_select("character c,guild g", "c.guild_id, c.guild_stat, g.guild_name",
		format("c.guild_id =g.guild_id and c.atorID =%d and g.guild_id <>%d", atorID, guild_id));
}

inline std::string guild_apply(int guild_id, int atorID, int max_try, int max_members)
{
	return format("update character set guild_id =%d ,guild_stat =1,guild_permission =0 where atorID =%d and "
		"%d in (select guild_id from guild where leader_id >0 and guild_id =%d and try_total <%d and member_total <%d)",
		guild_id, atorID, guild_id, guild_id, max_try, max_members);
}

inline std::string guild_add_applicant(int guild_id)
{
	return format("update guild set try_total =try_total +1 where guild_id =%d", guild_id);
}

inline std::string guild_drop_applicant(int guild_id)
{
	return format("update guild set try_total =try_total -1 where guild_id =%d and try_total > 0", guild_id);
}

inline std::string guild_save_bank(const char* bank, int guild_id)
{
	return format("update guild set bank = '%s' where guild_id =%d", bank, guild_id);
}

inline std::string guild_add_gold(int money, int guild_id)
{
	return format("update guild set gold = gold + %d where guild_id =%d", money, guild_id);
}

inline std::string guild_save_bank_log(const char* data, int guild_id)
{
	return format("update guild set banklog = '%s' where guild_id =%d", data, guild_id);
}

inline std::string guild_applicants(int guild_id)
{
	return format("select c.atorID,c.atorNome,c.job,c.degree from character c where (c.guild_stat =1 and c.guild_id =%d and c.delflag =0)", guild_id);
}

inline Select guild_leader_summary(int guild_id)
{
	return make_select("character c,guild g", columns::try_player,
		format("g.leader_id =c.atorID and g.guild_id =%d", guild_id));
}

inline Select character_can(int atorID, int guild_id, int permission)
{
	return make_select("character c", "c.atorID",
		format("c.atorID =%d and c.guild_id =%d and c.guild_permission & %d =%d", atorID, guild_id, permission, permission));
}

inline std::string guild_accept_applicant_count(int guild_id, int max_members)
{
	return format("update guild set try_total =try_total -1, member_total =member_total +1 where guild_id =%d and member_total <%d and try_total > 0",
		guild_id, max_members);
}

inline std::string guild_accept_applicant(int permission, int atorID, int guild_id)
{
	return format("update character set guild_stat =0,guild_permission =%d where atorID =%d and guild_id =%d and guild_stat =1 and delflag =0",
		permission, atorID, guild_id);
}

inline std::string guild_reject_applicant(int atorID, int guild_id)
{
	return format("update character set guild_id =0 ,guild_stat =0,guild_permission =0 where atorID =%d and guild_id =%d and guild_stat =1",
		atorID, guild_id);
}

inline std::string guild_remove_member(int atorID, int guild_id)
{
	return format("update character set guild_id =0 ,guild_stat =0,guild_permission =0 where atorID =%d and guild_id =%d and guild_stat =0 and "
		"atorID not in(select leader_id from guild where guild_id =%d)", atorID, guild_id, guild_id);
}

inline std::string guild_drop_member(int guild_id)
{
	return format("update guild set member_total =member_total -1 where guild_id =%d", guild_id);
}

inline std::string guild_clear(int guild_id, const char* passwd)
{
	return format("update guild set level = 0, gold = 0,bank = '', motto ='',passwd ='',leader_id =0, exp =0,member_total =0,try_total =0 "
		"where guild_id =%d and passwd ='%s' ", guild_id, passwd);
}

inline std::string guild_release_members(int guild_id)
{
	return format("update character set guild_id =0 ,guild_stat =0,guild_permission =0 where guild_id =%d", guild_id);
}

inline std::string guild_set_motto(const char* motto, int guild_id)
{
	return format("update guild set motto ='%s' where guild_id =%d", motto, guild_id);
}

inline std::string guild_set_challenge(int level, int guild_id)
{
	return format("update guild set challid = 0, challstart = 0, challmoney = 0, challlevel = %d where guild_id =%d", level, guild_id);
}

inline std::string guild_bid(int challenger, int money, int guild_id, int min_money)
{
	return format("update guild set challid =%d,challmoney =%d where guild_id =%d and challmoney < %d and challstart = 0",
		challenger, money, guild_id, min_money);
}

inline std::string guild_start_challenge(int guild_id)
{
	return format("update guild set challstart = 1 where guild_id =%d and challstart = 0", guild_id);
}


inline Select last_property_id()
{
	return make_select("property", "id", "ORDER BY id DESC LIMIT 1");
}

inline std::string property_lock(int id, int atorID, const char* context, int sum)
{
	return format("insert into property (id, atorID, context, sum, time) values (%d, %d, '%s', %d, now())", id, atorID, context, sum);
}

inline std::string property_unlock(int id, int atorID)
{
	return format("delete from property where id =%d and atorID =%d", id, atorID);
}

inline std::string insert_trade_log(const char* time, const char* game_server, const char* action, const char* from, const char* to, const char* memo)
{
	return format("insert into Trade_Log(ExecuteTime,GameServer,\"action\",\"from\",\"to\",Memo) values ('%s','%s','%s','%s','%s','%s')",
		time, game_server, action, from, to, memo);
}

}

#endif
