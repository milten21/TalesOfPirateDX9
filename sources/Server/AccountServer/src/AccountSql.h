#ifndef ACCOUNTSQL_H_
#define ACCOUNTSQL_H_

#include <stdarg.h>
#include <stdio.h>
#include <string>
#include "db3.h"

namespace account_sql
{

inline std::string format(const char* fmt, ...)
{
	char buf[2048];
	va_list ap;
	va_start(ap, fmt);
	_vsnprintf(buf, sizeof buf - 1, fmt, ap);
	va_end(ap);
	buf[sizeof buf - 1] = 0;
	return buf;
}

inline std::string seconds_since(const char* column)
{
	return format("extract(epoch from date_trunc('second', now()::timestamp) - date_trunc('second', %s))::bigint", column);
}

inline std::string select_login(const char* name)
{
	return format("select id, password, sid, login_status, login_group, ban, %s as protect_time from account_login where name='%s'",
		seconds_since("enable_login_time").c_str(), name);
}

inline std::string login_to_online(int online, const char* group, const char* mac, const char* ip, int id)
{
	return format("update account_login set login_status=%d, login_group='%s', enable_login_time=now(), last_login_time=now(), last_login_mac='%s', last_login_ip='%s' where id=%d",
		online, group, mac, ip, id);
}

inline std::string login_to_saving(int saving, const char* group, const char* mac, const char* ip, int id)
{
	return format("update account_login set login_status=%d, login_group='%s', last_login_time=now(), last_login_mac='%s', last_login_ip='%s' where id=%d",
		saving, group, mac, ip, id);
}

inline std::string login_transition(int offline, int online, int saving, int relogin_time, int saving_time,
	int invalid_sid, int sid, const char* group, int id)
{
	return format(
		"update account_login set "
		"login_status=case login_status when %d then %d when %d then %d when %d then %d end, "
		"enable_login_time=case login_status when %d then now() + interval '%d seconds' when %d then now() + interval '%d seconds' when %d then now() + interval '%d seconds' end, "
		"sid=case login_status when %d then %d when %d then %d when %d then %d end, "
		"login_group=case login_status when %d then '%s' when %d then login_group when %d then '%s' end "
		"where id=%d and (login_status=%d or (login_status=%d and enable_login_time<=now()) or (login_status=%d and enable_login_time<=now()))",
		offline, online, online, saving, saving, saving,
		offline, relogin_time, online, saving_time, saving, saving_time,
		offline, sid, online, invalid_sid, saving, sid,
		offline, group, online, saving, group,
		id, offline, online, saving);
}

inline std::string select_status(int id)
{
	return format("select login_status, login_group from account_login where id=%d", id);
}

inline std::string touch_login(const char* mac, int id)
{
	return format("update account_login set last_login_time=now(), last_login_mac = '%s' where id=%d", mac, id);
}

inline std::string close_session(int id)
{
	return format("update account_login set last_logout_time=now(), total_live_time=total_live_time+%s where id=%d",
		seconds_since("last_login_time").c_str(), id);
}

inline std::string logout_to_saving(int id, int sid, int online, int saving, int invalid_sid, int saving_time)
{
	CSQLUpdate s("account_login");
	s.SetWhere(format("(id=%d and sid=%d and login_status=%d)", id, sid, online).c_str());
	s.SetColumn("login_status", saving);
	s.SetColumn("sid", invalid_sid);
	s.SetColumn(format("enable_login_time=now() + interval '%d seconds'", saving_time).c_str());
	s.SetColumn("last_logout_time=now()");
	s.SetColumn(format("total_live_time=total_live_time+%s", seconds_since("last_login_time").c_str()).c_str());
	return s.GetStatement();
}

inline std::string logout(int offline, int id)
{
	return format("update account_login set login_status=%d, login_group='', enable_login_time=now(), last_logout_time=now(), total_live_time=total_live_time+%s where id=%d",
		offline, seconds_since("last_login_time").c_str(), id);
}

inline std::string reset_all(int offline, int invalid_sid)
{
	CSQLUpdate s("account_login");
	s.SetColumn("login_status", offline);
	s.SetColumn("sid", invalid_sid);
	s.SetColumn("enable_login_time=now()");
	s.SetColumn("login_group", "");
	return s.GetStatement();
}

inline std::string insert_user(const char* name, const char* password, const char* email)
{
	return format("insert into account_login(name,password,sid,total_live_time,email)  values('%s','%s',0,0,'%s');",
		name, password, email);
}

inline std::string update_password(const char* name, const char* password)
{
	return format("update account_login set password= '%s' where name = '%s'", password, name);
}

inline std::string select_user_state(const char* name)
{
	return format("select id, login_group, last_logout_time from account_login where name='%s'", name);
}

inline std::string set_ban(const char* name, int ban)
{
	return format("update account_login set ban = %d where name = '%s'", ban, name);
}

inline const char* check_account_login() { return "select ban from account_login"; }
inline const char* check_user_log() { return "select log_id, user_id, user_name, login_time, logout_time, login_ip from user_log"; }

inline std::string user_log_login(int user_id, const char* user_name, const char* ip)
{
	return format("insert into user_log (user_id, user_name, login_time, login_ip) values (%d, '%s', now(), '%s')",
		user_id, user_name, ip);
}

inline std::string user_log_logout(int user_id)
{
	return format("update user_log set logout_time=now() where log_id=(select max(log_id) from user_log where user_id=%d)", user_id);
}

}

#endif
