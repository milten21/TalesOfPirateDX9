#include "stdafx.h"
#include <stdlib.h>

#include "CommRPC.h"
#include "util.h"
#include "DataBaseCtrl.h"
#include "IniFile.h"
#include "GlobalVariable.h"
#include "AccountServer2.h"
#include "AccountSql.h"
#include "NetCommand.h"
#include "NetRetCode.h"

using namespace std;

CDataBaseCtrl::CDataBaseCtrl(void)
{
	m_strServerIP="";
	m_strServerDB="";
	m_strUserID="";
	m_strUserPwd="";
	m_pDataBase=NULL;
}

CDataBaseCtrl::~CDataBaseCtrl(void)
{
	if (IsConnect())
	{
		ReleaseObject();
	}
}

bool CDataBaseCtrl::CreateObject()
{
	try
	{
		dbc::IniFile inf(g_strCfgFile.c_str());
		dbc::IniSection& is = inf["db"];

		m_strServerIP = is["dbserver"];
		m_strServerDB = is["db"];
		m_strUserID = is["userid"];
		m_strUserPwd = is["passwd"];
		try
		{
			m_strClientEncoding = is["client_encoding"];
		}
		catch (dbc::excp&)
		{
			// optional setting, WIN1252 unless configured
		}
	}
	catch (dbc::excp& e)
	{
		cout << e.what() << endl;
		return false;
	}
	{
		CSQLDatabase encodingCheck;
		if (!encodingCheck.SetClientEncoding(m_strClientEncoding.c_str()))
		{
			cout << "Invalid client_encoding in [db]: " << m_strClientEncoding << endl;
			return false;
		}
	}

	printf("Connecting database [%s : %s]... ", m_strServerIP.c_str(), m_strServerDB.c_str());
	if (!Connect())
		return false;
	C_PRINT("success!\n");

	//初始化并测试数据库字段
	try
	{
        //  TOM表名已修改
		//if (g_TomService.IsEnable())
		//{
		//	//m_pDataBase->ExecuteSQL("update tom_account set login_status=0, from_server='', last_login_tick=0");
		//	m_pDataBase->ExecuteSQL("select ban from tom_account");
		//}
		//else
		{
			m_pDataBase->ExecuteSQL(account_sql::check_account_login());    //
		}
		m_pDataBase->ExecuteSQL(account_sql::check_user_log());
	}
	catch (CSQLException* se)
	{
		LG("DBExcp", "Check data field failure! SQL Exception in CDataBaseCtrl::CreateObject(): %s\n", se->m_strError.c_str());
		printf("Check data field failure! SQL Exception in CDataBaseCtrl::CreateObject(): %s\r\n", se->m_strError.c_str());
		return false;
	}
	catch (...)
	{
		LG("DBExcp", "Check data field failure! unknown exception raised from CDataBaseCtrl::CreateObject()\n");
		printf("Check data field failure! unknown exception raised from CDataBaseCtrl::CreateObject()\n");
		return false;
	}

	return true;
}

bool CDataBaseCtrl::InsertUser(std::string username, std::string password, std::string email){
	string strSQL = account_sql::insert_user(username.c_str(), password.c_str(), email.c_str());
	if (!IsConnect()) Connect();

	if (IsConnect())
	{
		try
		{
			m_pDataBase->ExecuteSQL(strSQL.c_str());
			return true;
		}
		catch (CSQLException* se)
		{
			LG("DBExcp", "SQL Exception in CDataBaseCtrl::InsertUser: %s\n", se->m_strError.c_str());
		}
		catch (...)
		{
			LG("DBExcp", "unknown exception raised from CDataBaseCtrl::InsertUser\n");
		}
	}
	LG("AccountServer", "CDataBaseCtrl::InsertUser: A record of user login cannot be saved! UserName=%s \n\n", username.c_str());

	Disconnect();
}


bool CDataBaseCtrl::UpdatePassword(string user, string pass)
{
	string strSQL = account_sql::update_password(user.c_str(), pass.c_str());
	if (!IsConnect()) Connect();

	if (IsConnect())
	{
		try
		{
			m_pDataBase->ExecuteSQL(strSQL.c_str());
			return true;
		}
		catch (CSQLException* se)
		{
			LG("DBExcp", "SQL Exception in CDataBaseCtrl::UpdatePassword: %s\n", se->m_strError.c_str());
		}
		catch (...)
		{
			LG("DBExcp", "unknown exception raised from CDataBaseCtrl::UpdatePassword\n");
		}
	}
	Disconnect();
	return false;
}

void CDataBaseCtrl::ReleaseObject()
{
	Disconnect();
}

bool CDataBaseCtrl::Connect()
{
	if (IsConnect()) return true;

	//建立数据库对象
	try
	{
		m_pDataBase=new CSQLDatabase();
	}
	catch (std::bad_alloc& e)
	{
		LG("DBExcp", "CDataBaseCtrl::CreateObject() new failed: %s\n", e.what());
		m_pDataBase=NULL;
		return false;
	}
	catch (...)
	{
		LG("DBExcp", "CDataBaseCtrl::CreateObject() unknown exception\n");
		m_pDataBase=NULL;
		return false;
	}

	//连接数据库
	std::string err;
	m_pDataBase->SetClientEncoding(m_strClientEncoding.c_str());
	if (!m_pDataBase->OpenPostgres(m_strServerIP.c_str(), m_strServerDB.c_str(), m_strUserID.c_str(), m_strUserPwd.c_str(), err))
	{
		LG("DBExcp", "CDataBaseCtrl::Connect() %s\n", err.c_str());
		SAFE_DELETE(m_pDataBase);
		return false;
	}
	m_pDataBase->SetAutoCommit(true);

	return true;
}

bool CDataBaseCtrl::IsConnect()
{
	return (m_pDataBase!=NULL);
}

void CDataBaseCtrl::Disconnect()
{
	if (m_pDataBase != NULL)
	{
		try
		{
			m_pDataBase->Close();
		}
		catch (...)
		{
			LG("DBExcp", "Exception raised when CDataBaseCtrl::Disconnect()\n");
		}
		SAFE_DELETE(m_pDataBase);
	}
}

bool CDataBaseCtrl::UserLogin(int nUserID, string strUserName, string strIP)
{
	if (!strUserName.c_str() || strUserName=="")
	{
		LG("AccountServer", "CDataBaseCtrl::UserLogin: parameter strUserName is empty or null\n");
		return false;
	}
	//LG("AccountServer", "CDataBaseCtrl::UserLogin: UserName=[%s] \n", strUserName.c_str());
	if (!strIP.c_str()) strIP="";

	string strSQL=account_sql::user_log_login(nUserID, strUserName.c_str(), strIP.c_str());
	if (!IsConnect()) Connect();

	if (IsConnect())
	{
		try
		{
			m_pDataBase->ExecuteSQL(strSQL.c_str());
			return true;
		}
		catch (CSQLException* se)
		{
			LG("DBExcp", "SQL Exception in CDataBaseCtrl::UserLogin: %s\n", se->m_strError.c_str());
		}
		catch (...)
		{
			LG("DBExcp", "unknown exception raised from CDataBaseCtrl::UserLogin\n");
		}
	}
	LG("AccountServer", "CDataBaseCtrl::UserLogin: A record of user login cannot be saved! UserID=%d UserName=%s IP=%s\n\n", nUserID, strUserName.c_str(), strIP.c_str());

	Disconnect();
	return false;
}


bool CDataBaseCtrl::UserLogout(int nUserID)
{
	string strSQL=account_sql::user_log_logout(nUserID);
	if (!IsConnect()) Connect();

	if (IsConnect())
	{
		try
		{
			m_pDataBase->ExecuteSQL(strSQL.c_str());
			return true;
		}
		catch (CSQLException* se)
		{
			LG("DBExcp", "SQL Exception in CDataBaseCtrl::UserLogout: %s\n", se->m_strError.c_str());
		}
		catch (...)
		{
			LG("DBExcp", "unknown exception raised from CDataBaseCtrl::UserLogout\n");
		}
	}
	LG("AccountServer", "CDataBaseCtrl::UserLogout: A record of user logout cannot be saved! UserID=%d \n", nUserID);

	Disconnect();
	return false;
}

bool CDataBaseCtrl::UserLoginMap(std::string strUserName, std::string strPassport)
{
	if (!strUserName.c_str() || strUserName=="")
	{
		LG("AccountServer", "CDataBaseCtrl::UserLoginMap: parameter strUserName is empty or null\n");
		return false;
	}
	//LG("AccountServer", "CDataBaseCtrl::UserLoginMap: UserName=[%s] login map\n", strUserName.c_str());

	CDataBaseCtrl::sPlayerData sData;
	sData.ctLoginTime = std::time(nullptr);
	m_mapUsers[strUserName.c_str()] = sData;
	return true;
}

bool CDataBaseCtrl::UserLogoutMap(std::string strUserName)
{
	if (!strUserName.c_str() || strUserName=="")
	{
		LG("AccountServer", "CDataBaseCtrl::UserLogoutMap: parameter strUserName is empty or null\n");
		return false;
	}
	//LG("AccountServer", "CDataBaseCtrl::UserLogoutMap: UserName=[%s] login map\n", strUserName.c_str());

	StringMap::const_iterator iter=m_mapUsers.find(strUserName.c_str());
	if (iter==m_mapUsers.end())
	{
		LG("AccountServer", "CDataBaseCtrl::UserLogoutMap : User [%s] not found in map, unable update the live time when logout\n", strUserName.c_str());
		return false;
	}

	CDataBaseCtrl::sPlayerData sData = iter->second;

	m_mapUsers.erase(strUserName.c_str());
	
	const std::time_t ctSpan = std::time(nullptr) - sData.ctLoginTime;
	if (ctSpan > 5 && ctSpan < 30 * 24 * 60 * 60)	//记录有效时间5秒到30天
	{
		return true;
	}

	Disconnect();
	return false;
}


bool CDataBaseCtrl::OperAccountBan(std::string strActName, int iban  )//Add by sunny.sun 20090828
{
	if(!strActName.c_str() || strActName=="")
	{
		LG("AccountServer", "CDataBaseCtrl::OperAccountBan: parameter strActName is empty or null\n");
		return false;
	}
	string strSQL = account_sql::set_ban(strActName.c_str(), iban);
	if (!IsConnect()) Connect();
	if (IsConnect())
	{
		try
		{
			m_pDataBase->ExecuteSQL(strSQL.c_str());
			return true;
		}
		catch (CSQLException* se)
		{
			LG("DBExcp", "SQL Exception in CDataBaseCtrl::OperAccountBan: %s\n", se->m_strError.c_str());
		}
		catch (...)
		{
			LG("DBExcp", "unknown exception raised from CDataBaseCtrl::OperAccountBan\n");
		}
	}
	LG("AccountServer", "CDataBaseCtrl::OperAccountBan: A record of user account cannot be ban! accName[%s]\n", strActName.c_str());

	Disconnect();
	return false;
}