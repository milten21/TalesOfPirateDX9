#ifndef GROUPSQL_H_
#define GROUPSQL_H_

#include <stdarg.h>
#include <stdio.h>
#include <string>

namespace group_sql
{

struct Select
{
	std::string table;
	std::string columns;
	std::string filter;
};

inline std::string format(const char* fmt, ...)
{
	char buf[8192];
	va_list ap;
	va_start(ap, fmt);
	_vsnprintf(buf, sizeof buf - 1, fmt, ap);
	va_end(ap);
	buf[sizeof buf - 1] = 0;
	return buf;
}

inline std::string minutes_since(const char* column)
{
	return format("(extract(epoch from date_trunc('minute', now()::timestamp) - date_trunc('minute', %s)) / 60)::bigint", column);
}

inline Select make_select(const char* table, const std::string& columns, const std::string& filter)
{
	Select s;
	s.table = table;
	s.columns = columns;
	s.filter = filter;
	return s;
}

inline std::string add_stat_log(long login, long play, long wgplay)
{
	return format("insert into stat_log (login_num , play_num, wgplay_num) values (%d, %d, %d)", login, play, wgplay);
}

inline std::string set_disc_info(int actid, const char* cli_ip, const char* reason)
{
	return format("update account set last_ip='%s',disc_reason ='%s',last_leave =now() where ato_id =%d", cli_ip, reason, actid);
}

inline Select last_account_id()
{
	return make_select("account", "ato_id", "ORDER BY ato_id DESC LIMIT 1");
}

inline std::string insert_account(unsigned long ato_id, const char* ato_nome, const char* ator_ids)
{
	return format("insert into account (ato_id, ato_nome, ator_ids) values (%d, '%s', '%s')", ato_id, ato_nome, ator_ids);
}

inline std::string update_account_characters(int ato_id, const char* ator_ids)
{
	return format("update account set ator_ids='%s' where ato_id=%d", ator_ids, ato_id);
}

inline std::string update_account_password(int ato_id, const char* password)
{
	return format("update account set password='%s' where ato_id=%d", password, ato_id);
}

inline Select account_by_name(const char* account)
{
	return make_select("account",
		"ato_id,jmes,ator_ids,password,last_ip,disc_reason,to_char(last_leave,'YYYY-MM-DD HH24:MI:SS')",
		format("ato_nome='%s'", account));
}

inline Select account_by_id(int ato_id)
{
	return make_select("account",
		"ato_nome,jmes,ator_ids,password,last_ip,disc_reason,to_char(last_leave,'YYYY-MM-DD HH24:MI:SS')",
		format("ato_id=%d", ato_id));
}

inline std::string zero_addr()
{
	return "update character set endeMem =0 where endeMem != 0";
}

inline std::string set_addr(long atorID, long addr)
{
	return format("update character set endeMem =%d where atorID =%d", addr, atorID);
}

inline std::string insert_character(const char* atorNome, int ato_id, const char* birth, const char* map, const char* olhe)
{
	return format("insert into character (atorNome, ato_id, birth,map,olhe) values ('%s', %d, '%s','%s', '%s')",
		atorNome, ato_id, birth, map, olhe);
}

inline std::string update_character_info(unsigned long atorID, unsigned short icon, const char* motto)
{
	return format("update character set icon =%d,motto ='%s' where atorID =%d", icon, motto, atorID);
}

inline Select character_by_name(const char* atorNome)
{
	return make_select("character", "atorID,motto,icon", format("atorNome='%s'", atorNome));
}

inline Select account_id_by_character_name(const char* atorNome)
{
	return make_select("character", "ato_id", format("atorNome='%s'", atorNome));
}

inline std::string start_estop(int atorID)
{
	return format("update character set estop = now() where atorID =%d", atorID);
}

inline std::string end_estop(int atorID)
{
	return format("update character set estoptime = estoptime - %s where atorID =%d", minutes_since("estop").c_str(), atorID);
}

inline Select active_estop(int atorID)
{
	return make_select("character", "estop", format("atorID = %d and estop + estoptime * interval '1 minute' > now()", atorID));
}

inline std::string estop(const char* atorNome, unsigned long lTimes)
{
	return format("update character set estop = now(), estoptime = %d where atorNome ='%s'", lTimes, atorNome);
}

inline std::string add_money(int atorID, unsigned long dwMoney)
{
	return format("update character set bomd = bomd + %d where atorID ='%d'", dwMoney, atorID);
}

inline std::string del_estop(const char* atorNome)
{
	return format("update character set estoptime = %d where atorNome ='%s'", 0, atorNome);
}

inline Select character_id_by_name(const char* atorNome)
{
	return make_select("character c", "c.atorID", format("c.atorNome='%s'", atorNome));
}

inline Select account_id_by_name(const char* atorNome)
{
	return make_select("character c", "c.ato_id", format("c.atorNome='%s'", atorNome));
}

inline Select character_by_id(int atorID, const char* no_guild_label)
{
	return make_select("character c,guild g",
		format("c.atorNome,c.motto,c.icon,"
			"case when c.guild_stat =0 then c.guild_id else 0 end,"
			"case when c.guild_stat <>0 or c.guild_id =0 then '%s' else g.guild_name end,"
			"c.job,c.degree,c.map,c.map_x,c.map_y,c.olhe,c.str,c.dex,c.agi,c.con,c.sta,c.luk,c.guild_permission,c.chatColour",
			no_guild_label),
		format("c.guild_id =g.guild_id and c.atorID=%d", atorID));
}

inline Select character_guild(int atorID)
{
	return make_select("character", "guild_id, guild_stat", format("atorID =%d", atorID));
}

inline std::string guild_leave_member(unsigned long guild_id)
{
	return format("update guild set member_total =member_total -1 where guild_id =%d and member_total > 0", guild_id);
}

inline std::string guild_leave_applicant(unsigned long guild_id)
{
	return format("update guild set try_total =try_total -1 where guild_id =%d and try_total > 0", guild_id);
}

inline std::string mark_character_deleted(int atorID)
{
	return format("update character set delflag =1,deldate =now() where atorID=%d", atorID);
}

inline Select friends_count(long cha_id1, long cha_id2)
{
	return make_select("friends", "count(*) num",
		format("(cha_id1=%d AND cha_id2 =%d)OR(cha_id1=%d AND cha_id2 =%d)", cha_id1, cha_id2, cha_id2, cha_id1));
}

inline Select friend_group_count(long cha_id1)
{
	return make_select(
		format("(select distinct friends.relation relation from friends "
			"where friends.cha_id1 =%d and friends.cha_id2 = -1) cc", cha_id1).c_str(),
		"count(*) num", "1=1");
}

inline Select friend_addr(long cha_id1, long cha_id2)
{
	return make_select("character INNER JOIN friends ON character.atorID = friends.cha_id2", "character.endeMem addr",
		format("(friends.cha_id1=%d AND friends.cha_id2 =%d)", cha_id1, cha_id2));
}

inline std::string move_friend_to_group(long cha_id1, long cha_id2, const char* newgroup)
{
	return format("update friends set relation ='%s' where cha_id1=%d AND cha_id2 =%d", newgroup, cha_id1, cha_id2);
}

inline std::string rename_friend_group(long cha_id1, const char* oldgroup, const char* newgroup)
{
	return format("update friends set relation ='%s' where cha_id1=%d AND relation = '%s'", newgroup, cha_id1, oldgroup);
}

inline std::string add_friend(long cha_id1, long cha_id2, const char* group)
{
	return format("insert into friends (cha_id1,cha_id2,relation) values(%d,%d,'%s')", cha_id1, cha_id2, group);
}

inline std::string del_friend(long cha_id1, long cha_id2)
{
	return format("delete from friends where (cha_id1=%d AND cha_id2 =%d)OR(cha_id1=%d AND cha_id2 =%d)",
		cha_id1, cha_id2, cha_id2, cha_id1);
}

inline Select master_count(long atorID)
{
	return make_select("master", "count(*) num", format("(cha_id1=%d)", atorID));
}

inline Select prentice_count(long atorID)
{
	return make_select("master", "count(*) num", format("(cha_id2=%d AND finish=0)", atorID));
}

inline Select has_master(long cha_id1, long cha_id2)
{
	return make_select("master", "count(*) num", format("(cha_id1=%d AND cha_id2=%d)", cha_id1, cha_id2));
}

inline std::string add_master(long cha_id1, long cha_id2)
{
	return format("insert into master (cha_id1,cha_id2,finish) values(%d,%d,%d)", cha_id1, cha_id2, 0);
}

inline std::string del_master(long cha_id1, long cha_id2)
{
	return format("delete from master where (cha_id1=%d AND cha_id2 =%d)", cha_id1, cha_id2);
}

inline std::string finish_master(long atorID)
{
	return format("update master set finish=1 where cha_id1=%d", atorID);
}

inline std::string master_relations()
{
	return "select cha_id1 cha_id1,cha_id2 cha_id2 from master";
}

inline std::string relation_tree(const char* near_side, const char* far_side, unsigned int atorID)
{
	return format(
		"select '' relation,count(*) addr,0 atorID,'' atorNome,0 icon,'' motto from ( "
		"select distinct master.relation relation from character INNER JOIN "
		"master ON character.atorID = master.%s where master.%s = %d "
		") cc union select master.relation relation,count(character.endeMem) addr,0 "
		"atorID,'' atorNome,1 icon,'' motto from character INNER JOIN master ON "
		"character.atorID = master.%s where master.%s = %d group by relation "
		"union select master.relation relation,character.endeMem addr,character.atorID "
		"atorID,character.atorNome atorNome,character.icon icon,character.motto motto "
		"from character INNER JOIN master ON character.atorID = master.%s "
		"where master.%s = %d order by relation,atorID,icon",
		far_side, near_side, atorID, far_side, near_side, atorID, far_side, near_side, atorID);
}

inline std::string master_data(unsigned int atorID)
{
	return relation_tree("cha_id1", "cha_id2", atorID);
}

inline std::string prentice_data(unsigned int atorID)
{
	return relation_tree("cha_id2", "cha_id1", atorID);
}

inline Select guild_count()
{
	return make_select("guild", "count(*)", "");
}

inline Select guild_by_name(const char* guild_name)
{
	return make_select("guild", "guild_id", format("guild_name='%s'", guild_name));
}

inline std::string disband_guild(unsigned long gldid)
{
	return format("update guild set motto ='',passwd ='',leader_id =0, gold =0,exp =0,member_total =0,try_total =0 "
		"where guild_id =%d", gldid);
}

inline std::string release_guild_members(unsigned long gldid)
{
	return format("update character set guild_id =0 ,guild_stat =0,guild_permission =0 where guild_id =%d", gldid);
}

inline std::string all_guilds()
{
	return "select g.guild_id, g.guild_name, g.motto, g.leader_id, "
		"g.exp, g.member_total, g.try_total,g.disband_date,g.level from guild As g where (g.guild_id >0)";
}

inline std::string guild_members(unsigned long gldid)
{
	return format("select c.endeMem,c.atorID, c.atorNome, c.motto, c.job, c.degree, c.icon, c.guild_permission "
		"from character As c where (c.guild_stat =0) and (c.guild_id =%d) and (c.delflag = 0)", gldid);
}

inline std::string load_order_params()
{
	return "select param1,param2,param3,param4,param5,param6,param7,param8,param9,param10 from param where id = 1";
}

inline std::string order_character(int atorID)
{
	return format("select atorNome,job,degree from character where atorID = %d ", atorID);
}

inline std::string save_order_params(const long* ids, const long* points)
{
	return format("update param set param1 = %d,param2 = %d,param3 = %d,param4 = %d,param5 = %d,"
		"param6 = %d,param7 = %d,param8 = %d,param9 = %d,param10 = %d where id = 1",
		ids[0], ids[1], ids[2], ids[3], ids[4], points[0], points[1], points[2], points[3], points[4]);
}

inline Select character_triggers()
{
	return make_select("pg_trigger", "count(*)", "tgname in ('tr_i_character','tr_d_character_friends') and not tgisinternal");
}

inline Select param_count()
{
	return make_select("param", "count(*)", "");
}

}

#endif
