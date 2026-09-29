#include "stdafx.h"
#include "Friend.h"
#include "Master.h"
#include "DBConnect.h"
#include "Guild.h"
#include "GroupServerApp.h"
#include "GameCommon.h"
#include "GroupSql.h"

using namespace std;


SQLRETURN Exec_sql_direct(const char *pszSQL, cfl_rs *pTable)
{
	//LG("group_sql", "表[%s], 开始执行SQL语句[%s]\n", pTable->get_table(), pszSQL);
	LG("group_sql", "Table [%s], begin execute SQL [%s]\n", pTable->get_table(), pszSQL);
	SQLRETURN r = pTable->exec_sql_direct(pszSQL);
	if(DBOK(r))
	{
		//LG("group_sql", "成功执行SQL!\n");
		LG("group_sql", "execute SQL success!");
	}
	else if(DBNODATA(r))
	{
		//LG("group_sql", "执行SQL, 但无结果返回\n");
		LG("group_sql", "execute SQL, no result \n");
	}
	else
	{
		//LG("group_sql", "执行SQL, 出错!\n");
		LG("group_sql", "execute SQL, failed!\n");
	}
	return r;
}

//==========TBLSystem===============================
bool TBLAccounts::IsReady()
{
	group_sql::Select q = group_sql::character_triggers();
	std::string l_tblname = _tbl_name;
	_tbl_name = q.table;
	int l_retrow	=0;
	bool l_ok = _get_row(m_buf, 1, q.columns.c_str(), q.filter.c_str(), &l_retrow);
	_tbl_name = l_tblname;
	if(!l_ok || l_retrow !=1 || atoi(m_buf[0].c_str()) !=2)
	{
		LogLine l_line(g_LogDB);
		l_line<<newln<<"SQL: character triggers missing, run the GameDB schema script !";
		return false;
	}
	return true;
}/*
int TBLSystem::Increment()
{
   char sql[SQL_MAXLEN];

    // account_save 表的 id 字段是主键
    sprintf(sql, "update %s set group_startup =group_startup +1",
            _get_table());
    Exec_sql_direct(sql, this);

	int l_retrow	=0;
	char* param = "group_startup";
	char filter[80];
	sprintf(filter, "group_startup = group_startup");
	if(_get_row(m_buf, 1, param, filter,&l_retrow) && l_retrow ==1 && get_affected_rows() ==1)
	{
		return atoi(m_buf[0].c_str());
	}else
	{
		return -1;
	}

}
void TBLSystem::Decrement()
{
   char sql[SQL_MAXLEN];

    // account_save 表的 id 字段是主键
    sprintf(sql, "update %s set group_startup =group_startup -1",
            _get_table());
    Exec_sql_direct(sql, this);
}
*/
//==========TBLAccounts===============================
void TBLAccounts::AddStatLog(long login,long play,long wgplay)
{
    char sql[SQL_MAXLEN];

    strcpy(sql, group_sql::add_stat_log(login, play, wgplay).c_str());
    Exec_sql_direct(sql, this);
}
bool TBLAccounts::SetDiscInfo(int actid,const char *cli_ip,const char *reason)
{
	char sql[SQL_MAXLEN];

	strcpy(sql, group_sql::set_disc_info(actid, cli_ip, reason).c_str());
	SQLRETURN l_ret =Exec_sql_direct(sql, this);
	return  (DBOK(l_ret))?true:false;
}
bool TBLAccounts::InsertRow(int ato_id,const char *ato_nome,const char *ator_ids)
{
    char sql[SQL_MAXLEN];

	DWORD dwActID = 0;
	std::string buf[1];
	group_sql::Select q = group_sql::last_account_id();
	int r1 = 0;
	int r = _get_rowOderby(buf, 1, q.columns.c_str(), q.filter.c_str(), &r1);
	if (DBOK(r) && r1 > 0)
	{
		dwActID = atol( buf[0].c_str() ) + 1;
	}
	else
	{
		dwActID = 1;
	}

    strcpy(sql, group_sql::insert_account(dwActID, ato_nome, ator_ids).c_str());
    SQLRETURN l_ret =Exec_sql_direct(sql, this);
    return (DBOK(l_ret))?true:false;

}
bool TBLAccounts::UpdateRow(int ato_id,const char *ator_ids)
{
	char sql[SQL_MAXLEN];

	strcpy(sql, group_sql::update_account_characters(ato_id, ator_ids).c_str());
	SQLRETURN l_ret =Exec_sql_direct(sql, this);
	return (DBOK(l_ret))?true:false;
}
bool TBLAccounts::UpdatePassword( int ato_id, const char szPassword[] )
{
	char sql[SQL_MAXLEN];

	strcpy(sql, group_sql::update_account_password(ato_id, szPassword).c_str());
	SQLRETURN l_ret =Exec_sql_direct(sql, this);
	return (DBOK(l_ret))?true:false;
}

int TBLAccounts::FetchRowByActName(const char szAccount[])
{
	int l_retrow	=0;
	group_sql::Select q = group_sql::account_by_name(szAccount);
	if(_get_row(m_buf, 7, q.columns.c_str(), q.filter.c_str(),&l_retrow))
	{
		if(l_retrow ==1 && get_affected_rows() ==1)
		{
			return l_retrow;
		}else
		{
			return 0;
		}
	}else
	{
		return -1;
	}
}

int TBLAccounts::FetchRowByActID(int ato_id)
{
	int l_retrow	=0;
	group_sql::Select q = group_sql::account_by_id(ato_id);
	if(_get_row(m_buf, 7, q.columns.c_str(), q.filter.c_str(),&l_retrow))
	{
		if(l_retrow ==1 && get_affected_rows() ==1)
		{
			return l_retrow;
		}else
		{
			return 0;
		}
	}else
	{
		return -1;
	}
}

//==========TBLCharacters===============================
bool TBLCharacters::ZeroAddr()
{
	char sql[SQL_MAXLEN];

	strcpy(sql, group_sql::zero_addr().c_str());
	SQLRETURN l_ret =Exec_sql_direct(sql, this);
	return (DBOK(l_ret))?true:false;
}

//bool TBLCharacters::ZeroAddr()
//{
//	char sql[SQL_MAXLEN];
//
//	for( int i = 0; i < 200; i++ )
//	{
//		// account_save 表的 id 字段是主键
//		int nMinID = i * 10000;
//		int nMaxID = (i + 1) * 10000;
//		sprintf(sql, "update %s set mem_addr = 0 where cha_id > %d and cha_id < %d and mem_addr != 0",_get_table(), nMinID, nMaxID );
//		SQLRETURN l_ret =Exec_sql_direct(sql, this, 60);
//		if( !DBOK(l_ret) )
//		{
//			return false;
//		}
//	}
//	return true;
//}

bool TBLCharacters::SetAddr(long atorID,long addr)
{
	char sql[SQL_MAXLEN];

	strcpy(sql, group_sql::set_addr(atorID, addr).c_str());
	SQLRETURN l_ret =Exec_sql_direct(sql, this);
	return (DBOK(l_ret))?true:false;
}
bool TBLCharacters::InsertRow(const char *atorNome,int ato_id,const char *birth,const char *map,const char *olhe)
{
	char sql[SQL_MAXLEN];

	strcpy(sql, group_sql::insert_character(atorNome, ato_id, birth, map, olhe).c_str());
	SQLRETURN l_ret =Exec_sql_direct(sql, this);
	return (DBOK(l_ret))?true:false;
}
bool TBLCharacters::UpdateInfo(unsigned long atorID,unsigned short icon,const char * motto)
{
	char sql[SQL_MAXLEN];

	strcpy(sql, group_sql::update_character_info(atorID, icon, motto).c_str());
	SQLRETURN l_ret =Exec_sql_direct(sql, this);
	return (DBOK(l_ret))?true:false;
}

int TBLCharacters::FetchRowByChaName(const char *atorNome)
{
	int l_retrow	=0;
	group_sql::Select q = group_sql::character_by_name(atorNome);
	if(_get_row(m_buf, 3, q.columns.c_str(), q.filter.c_str(),&l_retrow))
	{
		if(l_retrow ==1 && get_affected_rows() ==1)
		{
			return l_retrow;
		}else
		{
			return 0;
		}
	}else
	{
		return -1;
	}
}
bool TBLCharacters::FetchAccidByChaName(const char *atorNome, int& cha_accid )
{
	int l_retrow	=0;
	group_sql::Select q = group_sql::account_id_by_character_name(atorNome);
	if(_get_row(m_buf, 1, q.columns.c_str(), q.filter.c_str(),&l_retrow))
	{
		if(l_retrow ==1 && get_affected_rows() ==1)
		{
			cha_accid = atoi( m_buf[0].c_str() );
			return true;
		}else
		{
			return false;
		}
	}
	return false;
}

bool TBLCharacters::StartEstopTime( int atorID )
{
	char sql[SQL_MAXLEN];
	strcpy(sql, group_sql::start_estop(atorID).c_str());
	SQLRETURN l_ret =Exec_sql_direct(sql, this);
	return (DBOK(l_ret))?true:false;
}

bool TBLCharacters::EndEstopTime( int atorID )
{
	char sql[SQL_MAXLEN];
	strcpy(sql, group_sql::end_estop(atorID).c_str());
	SQLRETURN l_ret =Exec_sql_direct(sql, this);
	return (DBOK(l_ret))?true:false;
}

bool TBLCharacters::IsEstop(int atorID)
{
	int l_retrow	=0;
	group_sql::Select q = group_sql::active_estop(atorID);
	if(_get_row(m_buf, 1, q.columns.c_str(), q.filter.c_str(),&l_retrow))
	{
		if(l_retrow ==1 && get_affected_rows() ==1)
		{
			return true;
		}else
		{
			return false;
		}
	}
	return true;
}

bool TBLCharacters::Estop( const char *atorNome, uLong lTimes )
{
	char sql[SQL_MAXLEN];
	strcpy(sql, group_sql::estop(atorNome, lTimes).c_str());
	SQLRETURN l_ret =Exec_sql_direct(sql, this);
	return (DBOK(l_ret))?true:false;
}

bool TBLCharacters::AddMoney( int atorID, DWORD dwMoney )
{
	char sql[SQL_MAXLEN];
	strcpy(sql, group_sql::add_money(atorID, dwMoney).c_str());
	SQLRETURN l_ret =Exec_sql_direct(sql, this);
	return (DBOK(l_ret))?true:false;
}

bool TBLCharacters::DelEstop( const char *atorNome )
{
	char sql[SQL_MAXLEN];
	strcpy(sql, group_sql::del_estop(atorNome).c_str());
	SQLRETURN l_ret =Exec_sql_direct(sql, this);
	return (DBOK(l_ret))?true:false;
}

int TBLCharacters::FetchChaIDByCharName(cChar* atorNome)
{
	int l_retrow = 0;
	group_sql::Select q = group_sql::character_id_by_name(atorNome);
	std::string	l_tblname = _tbl_name;
	_tbl_name = q.table;
	bool l_bret = false;
	try{
		if (_get_row(m_buf, CHA_MAXCOL, q.columns.c_str(), q.filter.c_str(), &l_retrow)){
			return atoi(m_buf[0].c_str());
		}
	}
	catch (...)
	{
		LG("group_sql", "TBLCharacters::FetchChaIDByCharName execute SQL, failed!,atorID =%s\n", atorNome);
	}
	return 0;
}

int TBLCharacters::FetchActIDByCharName(cChar* atorNome)
{
	int l_retrow = 0;
	group_sql::Select q = group_sql::account_id_by_name(atorNome);
	std::string	l_tblname = _tbl_name;
	_tbl_name = q.table;
	bool l_bret = false;
	try{
		if (_get_row(m_buf, CHA_MAXCOL, q.columns.c_str(), q.filter.c_str(), &l_retrow)){
			return atoi(m_buf[0].c_str());
		}
	}
	catch (...)
	{
		LG("group_sql", "TBLCharacters::FetchActIDByCharName execute SQL, failed!,atorID =%s\n", atorNome);
	}
	return 0;
}

int TBLCharacters::FetchRowByChaID(int atorID)
{
	int l_retrow	=0;
/*
	char* param = "c.atorNome,c.motto,c.icon,\
				  case when c.guild_stat =0 then c.guild_id else 0 end,\
				  case when c.guild_stat <>0 or c.guild_id =0 then '[无]' else g.guild_name end,\
				  c.job,c.degree,c.map,c.map_x,c.map_y,c.look,c.str,c.dex,c.agi,c.con,c.sta,c.luk\
				  ";
*/
	group_sql::Select q = group_sql::character_by_id(atorID, RES_STRING(GP_DBCONNECT_CPP_00001));
	std::string	l_tblname	=_tbl_name;
	_tbl_name	=q.table;
	bool l_bret =false;
	try{
		l_bret	=_get_row(m_buf, CHA_MAXCOL, q.columns.c_str(), q.filter.c_str(),&l_retrow);
	}catch(...)
	{
		//LG("group_sql", "TBLCharacters::FetchRowByChaID执行SQL, 发生异常!,atorID =%d\n", atorID);
		LG("group_sql", "TBLCharacters::FetchRowByChaID execute SQL, failed!,atorID =%d\n", atorID);
	}
	_tbl_name	=l_tblname;
	if(l_bret)
	{
		if(l_retrow ==1 && get_affected_rows() ==1)
		{
            return l_retrow;
		}else
		{
			return 0;
		}
	}else
	{
		return -1;
	}
}
bool TBLCharacters::BackupRow(int atorID)
{
	char sql[SQL_MAXLEN];
	//insert into character_log (atorID, atorNome, ato_id, guild_id, job, degree, exp, hp, sp, ap, tp, gd, str, dex, agi, con, sta, luk, map, map_x, map_y, radius, look)
	//	select * from character where atorID =11

	string buf[3];
	group_sql::Select q = group_sql::character_guild(atorID);
	int	 l_retrow =0;
	DWORD dwGuildID;
	BYTE  byType;
	bool bret = _get_row(buf, 2, q.columns.c_str(), q.filter.c_str(), &l_retrow);
	if(l_retrow ==1)
	{
		dwGuildID = atoi(buf[0].c_str());
		byType = atoi(buf[1].c_str());
		if( dwGuildID > 0 )
		{
			// 减少公会信息计数
			if( byType == emGldMembStatNormal )
			{
				// 已经是会员
				strcpy(sql, group_sql::guild_leave_member(dwGuildID).c_str());
				SQLRETURN l_sqlret = Exec_sql_direct(sql, this);
				if( !DBOK(l_sqlret) )
				{
					//LG( "公会系统", "1>Reject:删除角色，但是更新减少公会成员人计数操作失败！数据库sql错误.ret = ", l_sqlret );
					LG( "GuildSystem", "1>Reject:delete cha，update guild count failed! database sql failed .ret = ", l_sqlret );
					return false;
				}
				else
				{
					if(get_affected_rows() !=1)
					{
						//LG( "公会系统", "2>Reject:删除角色，但是更新减少公会成员人计数操作失败！数据库sql错误.ret = ", l_sqlret );
					LG( "GuildSystem", "2>Reject:delete cha，update guild count failed! database sql failed .ret = ", l_sqlret );
						return false;

					}
					else
					{
					}
				}
			}
			else
			{
				// 正在申请
				strcpy(sql, group_sql::guild_leave_applicant(dwGuildID).c_str());
				SQLRETURN l_sqlret = Exec_sql_direct(sql, this);
				if( !DBOK(l_sqlret) )
				{
					//LG( "公会系统", "1>BackupRow:删除角色，但是更新减少公会申请人计数操作失败！数据库sql错误.ret = ", l_sqlret );
					LG( "GuildSystem", "1>BackupRow:delete cha，update guild count failed! database sql failed .ret = ", l_sqlret );
					return false;
				}
				else
				{
					if(get_affected_rows() !=1)
					{
						//LG( "公会系统", "2>BackupRow:删除角色，但是更新减少公会申请人计数操作失败！数据库sql错误.ret = ", l_sqlret );
						LG( "GuildSystem", "2>BackupRow:delete cha，update guild count failed! database sql failed .ret = ", l_sqlret );
						return false;
					}
					else
					{

					}
				}
			}
		}
	}else
	{
		//LG( "公会系统", "BackupRow:删除角色，获取角色公会信息失败！数据库sql错误.atorID = ", atorID );
		LG( "GuildSystem", "BackupRow:delete cha，get guild info failed! database sql failed.atorID = ", atorID );
		return false;
	}


	//sprintf(sql, "delete from %s where atorID=%d",_get_table(), atorID);
	strcpy(sql, group_sql::mark_character_deleted(atorID).c_str());   //  删除时间独立
	SQLRETURN l_ret =Exec_sql_direct(sql, this);
	return (DBOK(l_ret))?true:false;
}

//==========TBLFriends===============================
int TBLFriends::GetFriendsCount(long cha_id1,long cha_id2)
{
	int l_retrow	=0;
	group_sql::Select q = group_sql::friends_count(cha_id1, cha_id2);

	if(_get_row(m_buf, FRD_MAXCOL, q.columns.c_str(), q.filter.c_str(),&l_retrow) && l_retrow ==1 && get_affected_rows() ==1)
	{
		return atoi(m_buf[0].c_str());
	}else
	{
		return -1;
	}
}

int TBLFriends::GetGroupCount(long cha_id1)
{
	int l_retrow	=0;
	group_sql::Select q = group_sql::friend_group_count(cha_id1);
	_tbl_name = q.table;

	if(_get_row(m_buf, FRD_MAXCOL, q.columns.c_str(), q.filter.c_str(),&l_retrow) && l_retrow ==1 && get_affected_rows() ==1)
	{
		_tbl_name ="friends";
		return atoi(m_buf[0].c_str());
	}else
	{
		_tbl_name ="friends";
		return -1;
	}
}
unsigned long TBLFriends::GetFriendAddr(long cha_id1,long cha_id2)
{
	int l_retrow	=0;
	group_sql::Select q = group_sql::friend_addr(cha_id1, cha_id2);
	_tbl_name	=q.table;
	if(_get_row(m_buf, FRD_MAXCOL, q.columns.c_str(), q.filter.c_str(),&l_retrow) && l_retrow ==1 && get_affected_rows() ==1)
	{
		_tbl_name ="friends";
		return atoi(m_buf[0].c_str());
	}else
	{
		_tbl_name ="friends";
		return 0;
	}
}

bool TBLFriends::UpdateGroup(long cha_id1, long cha_id2, const char *newgroup)
{
	char sql[SQL_MAXLEN];

	strcpy(sql, group_sql::move_friend_to_group(cha_id1, cha_id2, newgroup).c_str());
	SQLRETURN l_ret =Exec_sql_direct(sql, this);
	return (DBOK(l_ret))?true:false;
}

bool TBLFriends::UpdateGroup(long cha_id1,const char *oldgroup,const char *newgroup)
{
	char sql[SQL_MAXLEN];

	strcpy(sql, group_sql::rename_friend_group(cha_id1, oldgroup, newgroup).c_str());
	SQLRETURN l_ret =Exec_sql_direct(sql, this);
	return (DBOK(l_ret))?true:false;
}

bool TBLFriends::AddFriend(long cha_id1,long cha_id2)
{
	char sql[SQL_MAXLEN];

	strcpy(sql, group_sql::add_friend(cha_id1, cha_id2, gc_standard_group).c_str());
	SQLRETURN l_ret =Exec_sql_direct(sql, this);

	if(DBOK(l_ret) && get_affected_rows() ==1)
	{
		strcpy(sql, group_sql::add_friend(cha_id2, cha_id1, gc_standard_group).c_str());
		SQLRETURN l_ret =Exec_sql_direct(sql, this);
		return (DBOK(l_ret))?true:false;
	}
	return false;
}
bool TBLFriends::DelFriend(long cha_id1,long cha_id2)
{
	char sql[SQL_MAXLEN];

	strcpy(sql, group_sql::del_friend(cha_id1, cha_id2).c_str());
	SQLRETURN l_ret =Exec_sql_direct(sql, this);
	return (DBOK(l_ret))?true:false;
}

//==========TBLMaster===============================
int TBLMaster::GetMasterCount(long atorID)
{
	int l_retrow	=0;
	group_sql::Select q = group_sql::master_count(atorID);
	if(_get_row(m_buf, MASTER_MAXCOL, q.columns.c_str(), q.filter.c_str(),&l_retrow) && l_retrow ==1 && get_affected_rows() ==1)
	{
		return atoi(m_buf[0].c_str());
	}else
	{
		return -1;
	}
}

int TBLMaster::GetPrenticeCount(long atorID)
{
	int l_retrow	=0;
	group_sql::Select q = group_sql::prentice_count(atorID);
	if(_get_row(m_buf, MASTER_MAXCOL, q.columns.c_str(), q.filter.c_str(),&l_retrow) && l_retrow ==1 && get_affected_rows() ==1)
	{
		return atoi(m_buf[0].c_str());
	}else
	{
		return -1;
	}
}

int TBLMaster::HasMaster(long cha_id1,long cha_id2)
{
	int l_retrow	=0;
	group_sql::Select q = group_sql::has_master(cha_id1, cha_id2);
	if(_get_row(m_buf, MASTER_MAXCOL, q.columns.c_str(), q.filter.c_str(),&l_retrow) && l_retrow ==1 && get_affected_rows() ==1)
	{
		return atoi(m_buf[0].c_str());
	}else
	{
		return -1;
	}
}

bool TBLMaster::AddMaster(long cha_id1,long cha_id2)
{
	char sql[SQL_MAXLEN];

	strcpy(sql, group_sql::add_master(cha_id1, cha_id2).c_str());
	SQLRETURN l_ret =Exec_sql_direct(sql, this);
	return (DBOK(l_ret))?true:false;
}

bool TBLMaster::DelMaster(long cha_id1,long cha_id2)
{
	char sql[SQL_MAXLEN];

	strcpy(sql, group_sql::del_master(cha_id1, cha_id2).c_str());
	SQLRETURN l_ret =Exec_sql_direct(sql, this);
	return (DBOK(l_ret))?true:false;
}

bool TBLMaster::FinishMaster(long atorID)
{
	char sql[SQL_MAXLEN];

	strcpy(sql, group_sql::finish_master(atorID).c_str());
	SQLRETURN l_ret =Exec_sql_direct(sql, this);
	return (DBOK(l_ret))?true:false;
}

bool TBLMaster::InitMasterRelation(map<uLong, uLong> &mapMasterRelation)
{
	bool ret = false;
	char sql[SQL_MAXLEN];
	strcpy(sql, group_sql::master_relations().c_str());

	// 执行查询操作
	SQLRETURN sqlret;
	SQLHSTMT hstmt = SQL_NULL_HSTMT;
	SQLSMALLINT col_num = 0;
	bool found = true;

	try
	{
		do
		{
			sqlret = SQLAllocHandle(SQL_HANDLE_STMT, _hdbc, &hstmt);
			if ((sqlret != SQL_SUCCESS) && (sqlret != SQL_SUCCESS_WITH_INFO))
			{
				handle_err(_hdbc, SQL_HANDLE_DBC, sqlret);
				throw 1;
			}

			sqlret = SQLExecDirect(hstmt, (SQLCHAR *)sql, SQL_NTS);
			if (sqlret != SQL_SUCCESS)
			{
				handle_err(hstmt, SQL_HANDLE_STMT, sqlret);
				if (sqlret != SQL_SUCCESS_WITH_INFO)
					throw 2;
			}

			sqlret = SQLNumResultCols(hstmt, &col_num);
			col_num = min(col_num, MAX_COL);
			col_num = min(col_num, _max_col);

			// Bind Column
			for (int i = 0; i < col_num; ++ i)
			{
				SQLBindCol(hstmt, UWORD(i + 1), SQL_C_CHAR, _buf[i], MAX_DATALEN, &_buf_len[i]);
			}

			// Fetch each Row
			for (int i = 0; ((sqlret = SQLFetch(hstmt)) == SQL_SUCCESS) || (sqlret == SQL_SUCCESS_WITH_INFO); ++ i)
			{
				if (sqlret != SQL_SUCCESS) handle_err(hstmt, SQL_HANDLE_STMT, sqlret, sql);

				uLong ulPID = atoi((char const *)_buf[0]);
				uLong ulMID = atoi((char const *)_buf[1]);

				mapMasterRelation[ulPID] = ulMID;
			}

			SQLFreeStmt(hstmt, SQL_CLOSE);
			SQLFreeStmt(hstmt, SQL_RESET_PARAMS);
			SQLFreeStmt(hstmt, SQL_UNBIND);
			ret = true;

		} while (0);
	}
	catch (...)
	{
		LogLine	l_line(g_LogMaster);
		l_line<<newln<<"Unknown Exception raised when InitMasterRelation()";
	}

	if (hstmt != SQL_NULL_HSTMT)
	{
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
		hstmt = SQL_NULL_HSTMT;
	}

	return ret;
}

bool TBLMaster::GetMasterData(master_dat* farray, int& array_num, unsigned int atorID)
{
	//NOTE(Ogge): Remember the size of farray before resetting array_num to be used as a return value
	const auto array_size{ array_num };
	array_num = 0;

	if (!farray || array_size <= 0 || atorID == 0) return false;

	bool ret = false;
	char sql[SQL_MAXLEN];
	strcpy(sql, group_sql::master_data(atorID).c_str());

	// 执行查询操作
	SQLRETURN sqlret;
	SQLHSTMT hstmt = SQL_NULL_HSTMT;
	SQLSMALLINT col_num = 0;
	bool found = true;

	try
	{
		do
		{
			sqlret = SQLAllocHandle(SQL_HANDLE_STMT, _hdbc, &hstmt);
			if ((sqlret != SQL_SUCCESS) && (sqlret != SQL_SUCCESS_WITH_INFO))
			{
				handle_err(_hdbc, SQL_HANDLE_DBC, sqlret);
				throw 1;
			}

			sqlret = SQLExecDirect(hstmt, (SQLCHAR *)sql, SQL_NTS);
			if (sqlret != SQL_SUCCESS)
			{
				handle_err(hstmt, SQL_HANDLE_STMT, sqlret);
				if (sqlret != SQL_SUCCESS_WITH_INFO)
					throw 2;
			}

			sqlret = SQLNumResultCols(hstmt, &col_num);
			col_num = min(col_num, MAX_COL);
			col_num = min(col_num, _max_col);

			// Bind Column
			for (int i = 0; i < col_num; ++ i)
			{
				SQLBindCol(hstmt, UWORD(i + 1), SQL_C_CHAR, _buf[i], MAX_DATALEN, &_buf_len[i]);
			}

			// Fetch each Row
			int i = 0;
			for (; ((sqlret = SQLFetch(hstmt)) == SQL_SUCCESS) || (sqlret == SQL_SUCCESS_WITH_INFO); ++ i)
			{
				if (i >= array_size)
				{
					break;
				}

				if (sqlret != SQL_SUCCESS) handle_err(hstmt, SQL_HANDLE_STMT, sqlret, sql);

				farray[i].relation = (char const *)_buf[0];
				farray[i].memaddr = atoi((char const *)_buf[1]);
				farray[i].atorID = atoi((char const *)_buf[2]);
				farray[i].atorNome = (char const *)_buf[3];
				farray[i].icon_id = atoi((char const *)_buf[4]);
				farray[i].motto = (char const *)_buf[5];
			}

			array_num = i; //NOTE(Ogge): Return number of rows read through referenced argument

			SQLFreeStmt(hstmt, SQL_CLOSE);
			SQLFreeStmt(hstmt, SQL_RESET_PARAMS);
			SQLFreeStmt(hstmt, SQL_UNBIND);
			ret = true;

		} while (0);
	}
	catch (...)
	{
		LogLine	l_line(g_LogMaster);
		l_line<<newln<<"Unknown Exception raised when GetMasterData()";
	}

	if (hstmt != SQL_NULL_HSTMT)
	{
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
		hstmt = SQL_NULL_HSTMT;
	}

	return ret;
}

bool TBLMaster::GetPrenticeData(master_dat* farray, int& array_num, unsigned int atorID)
{
	const auto array_size{ array_num };
	array_num = 0;

	if (!farray || array_size <= 0 || atorID == 0)
	{
		return false;
	}

	bool ret = false;
	char sql[SQL_MAXLEN];
	strcpy(sql, group_sql::prentice_data(atorID).c_str());

	// 执行查询操作
	SQLRETURN sqlret;
	SQLHSTMT hstmt = SQL_NULL_HSTMT;
	SQLSMALLINT col_num = 0;
	bool found = true;

	try
	{
		do
		{
			sqlret = SQLAllocHandle(SQL_HANDLE_STMT, _hdbc, &hstmt);
			if ((sqlret != SQL_SUCCESS) && (sqlret != SQL_SUCCESS_WITH_INFO))
			{
				handle_err(_hdbc, SQL_HANDLE_DBC, sqlret);
				throw 1;
			}

			sqlret = SQLExecDirect(hstmt, (SQLCHAR *)sql, SQL_NTS);
			if (sqlret != SQL_SUCCESS)
			{
				handle_err(hstmt, SQL_HANDLE_STMT, sqlret);
				if (sqlret != SQL_SUCCESS_WITH_INFO)
					throw 2;
			}

			sqlret = SQLNumResultCols(hstmt, &col_num);
			col_num = min(col_num, MAX_COL);
			col_num = min(col_num, _max_col);

			// Bind Column
			for (int i = 0; i < col_num; ++ i)
			{
				SQLBindCol(hstmt, UWORD(i + 1), SQL_C_CHAR, _buf[i], MAX_DATALEN, &_buf_len[i]);
			}

			// Fetch each Row
			int i = 0;
			for (; ((sqlret = SQLFetch(hstmt)) == SQL_SUCCESS) || (sqlret == SQL_SUCCESS_WITH_INFO); ++ i)
			{
				if (i >= array_size)
				{
					break;
				}

				if (sqlret != SQL_SUCCESS) handle_err(hstmt, SQL_HANDLE_STMT, sqlret, sql);

				farray[i].relation = (char const *)_buf[0];
				farray[i].memaddr = atoi((char const *)_buf[1]);
				farray[i].atorID = atoi((char const *)_buf[2]);
				farray[i].atorNome = (char const *)_buf[3];
				farray[i].icon_id = atoi((char const *)_buf[4]);
				farray[i].motto = (char const *)_buf[5];
			}

			array_num = i; // 取出的行数

			SQLFreeStmt(hstmt, SQL_CLOSE);
			SQLFreeStmt(hstmt, SQL_RESET_PARAMS);
			SQLFreeStmt(hstmt, SQL_UNBIND);
			ret = true;

		} while (0);
	}
	catch (...)
	{
		LogLine	l_line(g_LogMaster);
		l_line<<newln<<"Unknown Exception raised when GetPrenticeData()";
	}

	if (hstmt != SQL_NULL_HSTMT)
	{
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
		hstmt = SQL_NULL_HSTMT;
	}

	return ret;
}

//==========TBLGuilds===============================
bool TBLGuilds::IsReady()
{
	int l_retrow	=0;
	group_sql::Select q = group_sql::guild_count();
	if(_get_row(m_buf, 1, q.columns.c_str(), 0,&l_retrow))
	{
		if (l_retrow == 1 &&
			get_affected_rows() == 1 &&
			atoi(m_buf[0].c_str()) == 199) // NOTE: 199 is the maximum guild count
		{
			return true;
		}
	}
	return false;
}

int TBLGuilds::FetchRowByName(const char *guild_name)
{
	int l_retrow	=0;
	group_sql::Select q = group_sql::guild_by_name(guild_name);
	if(_get_row(m_buf, 1, q.columns.c_str(), q.filter.c_str(),&l_retrow))
	{
		if(l_retrow ==1 && get_affected_rows() ==1)
		{
			return l_retrow;
		}else
		{
			return 0;
		}
	}else
	{
		return -1;
	}
}
bool TBLGuilds::Disband(uLong gldid)
{
	char sql[SQL_MAXLEN];
	strcpy(sql, group_sql::disband_guild(gldid).c_str());
	SQLRETURN l_sqlret =Exec_sql_direct(sql, this);
	if(!DBOK(l_sqlret))
	{
		if(DBNODATA(l_sqlret))
		{
			LogLine	l_line(g_LogGuild);
			//l_line<<newln<<"解散公会SQL操作失败2！公会ID:"<<gldid;
			l_line<<newln<<"dismiss guild SQL failed2! guild ID:"<<gldid;
			return false;
		}else
		{
			LogLine	l_line(g_LogGuild);
			//l_line<<newln<<"解散公会SQL操作失败1！公会ID:"<<gldid;
			l_line<<newln<<"dismiss guild SQL failed1! guild ID:"<<gldid;
			return false;	//普通SQL错误
		}
	}
	strcpy(sql, group_sql::release_guild_members(gldid).c_str());
	l_sqlret =Exec_sql_direct(sql, this);
	if(!DBOK(l_sqlret))
	{
		LogLine	l_line(g_LogGuild);
		//l_line<<newln<<"解散公会SQL操作失败3！公会ID:"<<gldid;
		l_line<<newln<<"dismiss guild SQL failed3! guild ID:"<<gldid;
		return false;	//普通SQL错误
	}

	return true;
}
bool TBLGuilds::InitAllGuilds(char disband_days)
{
	if(disband_days<1)
	{
		return false;
	}

	bool l_ret = false;
	char sql[SQL_MAXLEN];
	strcpy(sql, group_sql::all_guilds().c_str());

	// 执行查询操作
	SQLRETURN sqlret;
	SQLHSTMT hstmt	= SQL_NULL_HSTMT;
	SQLSMALLINT col_num = 0;
	bool found		= true;

	try
	{
		sqlret = SQLAllocHandle(SQL_HANDLE_STMT, _hdbc, &hstmt);
		if ((sqlret != SQL_SUCCESS) && (sqlret != SQL_SUCCESS_WITH_INFO))
		{
			handle_err(_hdbc, SQL_HANDLE_DBC, sqlret);
			throw 1;
		}

		sqlret = SQLExecDirect(hstmt, (SQLCHAR *)sql, SQL_NTS);
		if (sqlret != SQL_SUCCESS)
		{
			handle_err(hstmt, SQL_HANDLE_STMT, sqlret);
			if (sqlret != SQL_SUCCESS_WITH_INFO)
				throw 2;
		}

		sqlret = SQLNumResultCols(hstmt, &col_num);
		col_num = min(col_num, MAX_COL);
		col_num = min(col_num, _max_col);

		// Bind Column
		for (int i = 0; i < col_num; ++ i)
		{
			SQLBindCol(hstmt, UWORD(i + 1), SQL_C_CHAR, _buf[i], MAX_DATALEN, &_buf_len[i]);
		}

		// Fetch each Row	int i; // 取出的行数
		for (int f_row = 1; (sqlret = SQLFetch(hstmt)) == SQL_SUCCESS || sqlret == SQL_SUCCESS_WITH_INFO; ++ f_row)
		{
			if (sqlret != SQL_SUCCESS)
			{
				handle_err(hstmt, SQL_HANDLE_STMT, sqlret);
			}
			Guild* l_gld = Guild::Alloc();
			l_gld->m_id = atol((cChar*) _buf[0]);			//公会ID
			strcpy(l_gld->m_name, (cChar*)_buf[1]);			//公会名
			strcpy(l_gld->m_motto,(cChar*)_buf[2]);			//公会座右铭
			l_gld->m_leaderID = atol((cChar*)_buf[3]);		//会长ID
			l_gld->m_remain_minute = atol((cChar*)_buf[7]);	//公会解散剩余分钟数
			l_gld->m_tick = GetTickCount();

			l_gld->BeginRun();
		}

		SQLFreeStmt(hstmt, SQL_UNBIND);
		l_ret = true;
	}catch(int&e)
	{
		LogLine	l_line(g_LogGuild);
		//l_line<<newln<<"初始化公会过程ODBC 接口调用错误，InitAllGuilds()位置码："<<e;
		l_line<<newln<<"init guild ODBC interface failed, InitAllGuilds() error:"<<e;
	}catch (...)
	{
		LogLine	l_line(g_LogGuild);
		l_line<<newln<<"Unknown Exception raised when InitAllGuilds()";
	}

	if (hstmt != SQL_NULL_HSTMT)
	{
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
		hstmt = SQL_NULL_HSTMT;
	}

	return l_ret;
}
bool TBLGuilds::SendGuildInfo(Player *ply)
{
	WPacket l_togmSelf  =g_gpsvr->GetWPacket();
	l_togmSelf.WriteCmd( CMD_PM_GUILDINFO );
	l_togmSelf.WriteLong( ply->m_chaid[ply->m_currcha] ); // 角色DBID
	l_togmSelf.WriteLong(ply->m_guild[ply->m_currcha]);	//公会ID	
	l_togmSelf.WriteLong(ply->GetGuild()->m_leaderID);	//会长ID	
	l_togmSelf.WriteString(ply->GetGuild()->m_name);	//公会name
	l_togmSelf.WriteString(ply->GetGuild()->m_motto);	//公会座佑名
	ply->m_gate->GetDataSock()->SendData( l_togmSelf );
	return true;
}
bool TBLGuilds::InitGuildMember(Player *ply,uLong chaid,uLong gldid,int mode)
{
	bool l_ret = false;
	if(ply && gldid ==0)
	{
		WPacket l_toSelf  =g_gpsvr->GetWPacket();
		l_toSelf.WriteCmd(CMD_PC_GUILD);
		l_toSelf.WriteChar(MSG_GUILD_START);

		l_toSelf.WriteLong(0);
		l_toSelf.WriteChar(0);

		g_gpsvr->SendToClient(ply,l_toSelf);
	}else
	{
		char sql[SQL_MAXLEN];
		strcpy(sql, group_sql::guild_members(gldid).c_str());
		// 执行查询操作
		SQLRETURN sqlret;
		SQLHSTMT hstmt	= SQL_NULL_HSTMT;
		SQLSMALLINT col_num = 0;
		bool found		= true;

		try
		{
			sqlret = SQLAllocHandle(SQL_HANDLE_STMT, _hdbc, &hstmt);
			if ((sqlret != SQL_SUCCESS) && (sqlret != SQL_SUCCESS_WITH_INFO))
			{
				handle_err(_hdbc, SQL_HANDLE_DBC, sqlret);

				throw 1;
			}

			sqlret = SQLExecDirect(hstmt, (SQLCHAR *)sql, SQL_NTS);
			if (sqlret != SQL_SUCCESS)
			{
				handle_err(hstmt, SQL_HANDLE_STMT, sqlret);

				if (sqlret != SQL_SUCCESS_WITH_INFO)
					throw 2;
			}

			sqlret = SQLNumResultCols(hstmt, &col_num);
			col_num = min(col_num, MAX_COL);
			col_num = min(col_num, _max_col);

			// Bind Column
			for (int i = 0; i < col_num; ++ i)
			{
				SQLBindCol(hstmt, UWORD(i + 1), SQL_C_CHAR, _buf[i], MAX_DATALEN, &_buf_len[i]);
			}
			WPacket l_toGuild =g_gpsvr->GetWPacket();
			l_toGuild.WriteCmd(CMD_PC_GUILD);
			if(mode)
			{
				l_toGuild.WriteChar(MSG_GUILD_ADD);
			}else
			{
				l_toGuild.WriteChar(MSG_GUILD_ONLINE);
				l_toGuild.WriteLong(chaid);
			}

			WPacket l_toSelf,l_wpk0;
			if(ply)
			{
				l_wpk0  =g_gpsvr->GetWPacket();
				l_wpk0.WriteCmd(CMD_PC_GUILD);
				l_wpk0.WriteChar(MSG_GUILD_START);
			}
			bool	l_hrd	=false;

			Player *l_plylst[10240];
			short	l_plynum	=0;

			long lPacketNum = 0;

			// Fetch each Row	int i; // 取出的行数
			int f_row = 1;
			for (; (sqlret = SQLFetch(hstmt)) == SQL_SUCCESS || sqlret == SQL_SUCCESS_WITH_INFO; ++ f_row)
			{
				if (sqlret != SQL_SUCCESS)
				{
					handle_err(hstmt, SQL_HANDLE_STMT, sqlret);
				}
				if(ply && (f_row %20) ==1)
				{
					l_toSelf	=l_wpk0;
				}
				if(ply && !l_hrd)
				{
					l_hrd	=true;
					l_toSelf.WriteLong(ply->m_guild[ply->m_currcha]);	//公会ID
					l_toSelf.WriteString(ply->GetGuild()->m_name);		//公会name
					l_toSelf.WriteLong(ply->GetGuild()->m_leaderID);	//会长ID
				}
				uLong l_memaddr		=atol((cChar *)_buf[0]);
				if(l_memaddr)
				{
					l_plylst[l_plynum] = ToPointer<Player>(l_memaddr);
					l_plynum++;
				}
				if(mode && chaid ==atol((cChar*)_buf[1]))
				{
					l_toGuild.WriteChar(l_memaddr?1:0);			//online
					l_toGuild.WriteLong(atol((cChar*)_buf[1]));	//chaid
					l_toGuild.WriteString(	(cChar*)_buf[2]);	//chaname
					l_toGuild.WriteString(	(cChar*)_buf[3]);	//motto
					l_toGuild.WriteString(	(cChar*)_buf[4]);	//job
					l_toGuild.WriteShort(atoi((cChar*)_buf[5]));//degree
					l_toGuild.WriteShort(atoi((cChar*)_buf[6]));//icon
					l_toGuild.WriteLong(stoull((cChar*)_buf[7]));//permission
				}
				if(ply)
				{
					l_toSelf.WriteChar(l_memaddr?1:0);			//online
					l_toSelf.WriteLong(atol((cChar*)_buf[1]));	//chaid
					l_toSelf.WriteString(	(cChar*)_buf[2]);	//chaname
					l_toSelf.WriteString(	(cChar*)_buf[3]);	//motto
					l_toSelf.WriteString(	(cChar*)_buf[4]);	//job
					l_toSelf.WriteShort(atoi((cChar*)_buf[5]));	//degree
					l_toSelf.WriteShort(atoi((cChar*)_buf[6]));	//icon
					l_toSelf.WriteLong(stoull((cChar*)_buf[7]));	//permission
				}
				if(ply && !(f_row %20))
				{
					l_toSelf.WriteLong(lPacketNum);
					lPacketNum++;
					l_toSelf.WriteChar(((f_row-1)%20)+1);	//本次包括的条数
					g_gpsvr->SendToClient(ply,l_toSelf);
				}
			}
			if(ply && (f_row%20) ==1)
			{
				l_toSelf	=l_wpk0;
			}
			if(ply && !l_hrd)
			{
				l_hrd	=true;
				l_toSelf.WriteLong(ply->m_guild[ply->m_currcha]);	//公会ID
				l_toSelf.WriteString(ply->GetGuild()->m_name);		//公会name
				l_toSelf.WriteLong(ply->GetGuild()->m_leaderID);	//会长ID
			}
			if(ply)
			{
				l_toSelf.WriteLong(lPacketNum);
				lPacketNum++;
				l_toSelf.WriteChar((f_row -1)%20);
				g_gpsvr->SendToClient(ply,l_toSelf);
			}
			LogLine	l_line(g_LogGuild);
			//l_line<<newln<<"上线通知的会友数："<<l_plynum<<endln;
			l_line<<newln<<"online guild num:"<<l_plynum<<endln;
			g_gpsvr->SendToClient(l_plylst,l_plynum,l_toGuild);

			SQLFreeStmt(hstmt, SQL_UNBIND);
			l_ret = true;
		}catch(int&e)
		{
			LogLine	l_line(g_LogGuild);
			//l_line<<newln<<"初始化公会过程ODBC 接口调用错误，InitGuildMember()位置码："<<e;
			l_line<<newln<<"init guild ODBC interface failed, InitGuildMember() error:"<<e;

			l_line<<newln<<sql;
		}catch (...)
		{
			LogLine	l_line(g_LogGuild);
			l_line<<newln<<"Unknown Exception raised when InitGuildMember()";
		}

		if (hstmt != SQL_NULL_HSTMT)
		{
			SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
			hstmt = SQL_NULL_HSTMT;
		}
	}

	return l_ret;
}


bool TBLParam::InitParam(void)
{
	string strSQL = group_sql::load_order_params();
//	string strSQL = "select param1 from param where id = 1";

	SQLRETURN sqlret;
	SQLHSTMT hstmt	= SQL_NULL_HSTMT;
	SQLINTEGER buf_len[MAXORDERNUM+MAXORDERNUM];
//	SQLINTEGER		nID = 0,nlen;
	bool found		= true;

	try
	{
		sqlret = SQLAllocHandle(SQL_HANDLE_STMT, _hdbc, &hstmt);
		if ((sqlret != SQL_SUCCESS) && (sqlret != SQL_SUCCESS_WITH_INFO))
		{
			handle_err(_hdbc, SQL_HANDLE_DBC, sqlret);
			throw 1;
		}
		sqlret = SQLSetStmtAttr( hstmt,SQL_ATTR_ROW_ARRAY_SIZE,(SQLPOINTER)1,0);
		if ((sqlret != SQL_SUCCESS) && (sqlret != SQL_SUCCESS_WITH_INFO))
		{
			handle_err(_hdbc, SQL_HANDLE_DBC, sqlret);
			throw 1;
		}
		SQLBindCol(hstmt, 1, SQL_C_LONG, &m_nOrder[0].nid, 0, &(buf_len[0]));
		SQLBindCol(hstmt, 2, SQL_C_LONG, &m_nOrder[1].nid, 0, &buf_len[1]);
		SQLBindCol(hstmt, 3, SQL_C_LONG, &m_nOrder[2].nid, 0, &buf_len[2]);
		SQLBindCol(hstmt, 4, SQL_C_LONG, &m_nOrder[3].nid, 0, &buf_len[3]);
		SQLBindCol(hstmt, 5, SQL_C_LONG, &m_nOrder[4].nid, 0, &buf_len[4]);

		SQLBindCol(hstmt, 6, SQL_C_LONG, &m_nOrder[0].nfightpoint, 0, &buf_len[5]);
		SQLBindCol(hstmt, 7, SQL_C_LONG, &m_nOrder[1].nfightpoint, 0, &buf_len[6]);
		SQLBindCol(hstmt, 8, SQL_C_LONG, &m_nOrder[2].nfightpoint, 0, &buf_len[7]);
		SQLBindCol(hstmt, 9, SQL_C_LONG, &m_nOrder[3].nfightpoint, 0, &buf_len[8]);
		SQLBindCol(hstmt, 10,SQL_C_LONG, &m_nOrder[4].nfightpoint, 0, &buf_len[9]);
		sqlret = SQLExecDirect(hstmt,(SQLCHAR *)const_cast<char*>(strSQL.c_str()),SQL_NTS);
		if (sqlret != SQL_SUCCESS)
		{
			handle_err(hstmt, SQL_HANDLE_STMT, sqlret);
			if (sqlret != SQL_SUCCESS_WITH_INFO)
				throw 2;
		}

		sqlret = SQLFetch(hstmt);
		if (sqlret != SQL_SUCCESS)
		{
			// Modfi by lark.li 20080714 begin
			if(sqlret != SQL_NO_DATA)
				handle_err(hstmt, SQL_HANDLE_STMT, sqlret);
			// End
		}
		SQLFreeStmt(hstmt, SQL_CLOSE);
		SQLFreeStmt(hstmt, SQL_UNBIND);	}
	catch(int&e)
	{
		LogLine	l_line(g_LogGarner2);
		//l_line<<newln<<"初始化公会过程ODBC 接口调用错误，InitParam()位置码："<<e;
		l_line<<newln<<"init guild ODBC interface failed, InitParam() error:"<<e;
	}catch (...)
	{
		LogLine	l_line(g_LogGarner2);
		l_line<<newln<<"Unknown Exception raised when InitParam()";
	}

	char buff[255];
	int nlev;
	try
	{
		sqlret = SQLAllocHandle(SQL_HANDLE_STMT, _hdbc, &hstmt);
		if ((sqlret != SQL_SUCCESS) && (sqlret != SQL_SUCCESS_WITH_INFO))
		{
			handle_err(_hdbc, SQL_HANDLE_DBC, sqlret);
			throw 1;
		}
		SQLBindCol(hstmt, 1, SQL_C_CHAR, _buf[0], MAX_DATALEN, &buf_len[0]);
		SQLBindCol(hstmt, 2, SQL_C_CHAR, _buf[1], MAX_DATALEN, &buf_len[1]);
		SQLBindCol(hstmt, 3, SQL_C_ULONG, &nlev, 0, &buf_len[2]);
		for(int n = 0;n<MAXORDERNUM;n++)
		{
			strcpy(buff, group_sql::order_character(m_nOrder[n].nid).c_str());

			sqlret = SQLExecDirect(hstmt,(SQLCHAR *)buff,SQL_NTS);
			if (sqlret != SQL_SUCCESS)
			{
				handle_err(hstmt, SQL_HANDLE_STMT, sqlret);
				if (sqlret != SQL_SUCCESS_WITH_INFO)
					throw 2;
			}
			int i =0;
			if((sqlret = SQLFetch(hstmt)) != SQL_NO_DATA) 
			{
				if(sqlret == SQL_NO_DATA)
				{
					LogLine	l_line(g_LogGarner2);
					//l_line<<newln<<"角色名查询出错了。角色ID："<<m_nOrder[n].nid;
					l_line<<newln<<"cha name query failed .cha ID："<<m_nOrder[n].nid;
					continue;

				}
				if (sqlret != SQL_SUCCESS)
				{
					handle_err(hstmt, SQL_HANDLE_STMT, sqlret);
				}
				if(buf_len[0] >20 )
				{
					LogLine	l_line(g_LogGarner2);
					//l_line<<newln<<"角色名查询出错了。";
					l_line<<newln<<"cha name query failed.";
					return false;
				}
				memcpy(m_nOrder[n].strname,_buf[0],buf_len[0]);
				m_nOrder[n].strname[buf_len[0]]='\0';
				memcpy(m_nOrder[n].strjob,_buf[1],buf_len[1]);
				m_nOrder[n].strjob[buf_len[1]]='\0';
				m_nOrder[n].nlev = nlev;
			}
			SQLFreeStmt(hstmt, SQL_CLOSE);
		}
		SQLFreeStmt(hstmt, SQL_UNBIND);
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	}
	catch(int&e)
	{
		LogLine	l_line(g_LogGarner2);
		//l_line<<newln<<"初始化公会过程ODBC 接口调用错误，InitParam()位置码："<<e;
		l_line<<newln<<"init guild ODBC interface failed, InitParam() erro :"<<e;
	}catch (...)
	{
		LogLine	l_line(g_LogGarner2);
		l_line<<newln<<"Unknown Exception raised when InitParam()";
	}
	return true;
}

bool TBLParam::SaveParam(void)
{
	long ids[MAXORDERNUM];
	long points[MAXORDERNUM];
	for(int i = 0;i < MAXORDERNUM;i++)
	{
		ids[i] = m_nOrder[i].nid;
		points[i] = m_nOrder[i].nfightpoint;
	}
	char buff[512];
	strcpy(buff, group_sql::save_order_params(ids, points).c_str());
	SQLRETURN sqlret;
	sqlret = _db->exec_sql_direct(buff);
	if(sqlret != SQL_SUCCESS)
	{
		LG("ParamErr","Save Param Error SQL = %s",buff);
	}
	return true;
}

bool TBLParam::IsReady()
{
	int l_retrow	=0;
	group_sql::Select q = group_sql::param_count();
	if(_get_row(m_buf, 1, q.columns.c_str(), 0,&l_retrow))
	{
		if(l_retrow ==1 && get_affected_rows() ==1 && atoi(m_buf[0].c_str()) >=199)
		{
			return true;
		}
	}
	return false;
}

void TBLParam::UpdateOrder(ORDERINFO &Order)
{
	ORDERINFO ordertemp[MAXORDERNUM];

	memcpy(ordertemp,m_nOrder,sizeof(ORDERINFO)*MAXORDERNUM);


	int i = 0;
	int oldid = 0;
	for(i = 0;i< MAXORDERNUM;i++)
	{
		if(ordertemp[i].nfightpoint >= Order.nfightpoint)
		{
			if(ordertemp[i].nid == Order.nid)
				break;
			continue;
		}
		else
		{
			oldid = i;
			if(ordertemp[i].nid == Order.nid)
			{
				m_nOrder[i].nfightpoint = Order.nfightpoint;
				break;
			}
			memcpy(&m_nOrder[i++],&Order,sizeof(ORDERINFO));

			int n=-1;
			for(int a = i;a<MAXORDERNUM;)
			{
				if(ordertemp[a+n].nid == Order.nid)
				{
					n++;
					continue;
				}

				if(a+n < MAXORDERNUM)
					memcpy(&m_nOrder[a],&ordertemp[a+n],sizeof(ORDERINFO));
				else
				{
					strcpy(m_nOrder[a].strjob,"");
					strcpy(m_nOrder[a].strname,"");
					m_nOrder[a].nid = -1;
					m_nOrder[a].nlev = 0;
					m_nOrder[a].nfightpoint=0;
				}
				a++;
			}

			SaveParam();
			WPacket l_wpk = g_gpsvr->GetWPacket();
			l_wpk.WriteCmd(CMD_PM_GARNER2_UPDATE);
			for(i = 0;i < MAXORDERNUM;i++)
			{
				l_wpk.WriteLong(m_nOrder[i].nid);
			}
			l_wpk.WriteLong(oldid);
			l_wpk.WriteLong(0);

			for (auto& gate : g_gpsvr->m_gate)
			{
				if (gate.GetDataSock())
				{
					gate.GetDataSock()->SendData(l_wpk);
					break;
				}
			}
			LogLine	l_line(g_LogGarner2);			
			//l_line<<newln<<"反斗白银排名变了";
			l_line<<newln<<"order chaned";
			break;
		}
	}
}