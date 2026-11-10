#include "ResourceBundleManage.h"

#include "IniFile.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <filesystem>
#endif
#include <cstdio>
#include <cstdlib>
#include <string>
#include <memory>
#include <mutex>

using namespace std;

#include "pi_Alloc.h"

#define BUFFER_SIZE 255

namespace
{

std::string ExecutableDirectory()
{
#ifdef _WIN32
	char szPath[MAX_PATH];
	if( !GetModuleFileName( NULL, szPath, MAX_PATH ) )
	{
		throw "GetModuleFileName failed \n";
	}
	const char* ret = strrchr(szPath, '\\');
	return std::string(szPath, ret - szPath);
#else
	std::error_code error;
	const std::filesystem::path path = std::filesystem::read_symlink("/proc/self/exe", error);
	if (error)
	{
		throw "readlink /proc/self/exe failed \n";
	}
	return path.parent_path().string();
#endif
}

std::string& ArgumentCodepage()
{
	static std::string codepage;
	return codepage;
}

}

CFormatParameter::CFormatParameter(int paraNum)
{
	m_MsgArgs = new icu::Formattable[paraNum];
	this->paraNum = paraNum;
}

CFormatParameter::~CFormatParameter()
{
	if(m_MsgArgs)
	{
		delete[] m_MsgArgs;
		m_MsgArgs = NULL;
	}
}

void CFormatParameter::setDouble(int index, double d)
{
	m_MsgArgs[index].setDouble(d);
}

void CFormatParameter::setLong(int index, int32_t l)
{
	m_MsgArgs[index].setLong(l);
}

void CFormatParameter::setInt64(int index, int64_t ll) 
{
	m_MsgArgs[index].setInt64(ll);
}

void CFormatParameter::setDate(int index, UDate d) 
{
	m_MsgArgs[index].setDate(d);
}

void CFormatParameter::setString(int index, const icu::UnicodeString &stringToCopy)
{
	m_MsgArgs[index].setString(stringToCopy);
}

void CFormatParameter::setString(int index, const char* codepageData)
{
	const std::string& codepage = ArgumentCodepage();
	m_MsgArgs[index].setString(icu::UnicodeString(codepageData, -1, codepage.empty() ? NULL : codepage.c_str()));
}

CResourceBundleManage::CResourceBundleManage(const char* configFileName)
:  m_pResourceBundle(NULL), m_pConverter(NULL), m_MaxSize(0), m_LogFlag(0), m_LogFile(NULL)
{
	const std::string exeDir = ExecutableDirectory();

	CIniFile ini;
	ini.Load(exeDir + "/" + configFileName);

	UErrorCode status = U_ZERO_ERROR;
	icu::Locale::setDefault(icu::Locale::getEnglish(),status);

	const char* lpAppName = "locale";

	const std::string locale = ini.GetString(lpAppName, "locale", "en_US");
	m_ResLocale = std::make_unique<char[]>(locale.size() + 1);
	memcpy(m_ResLocale.get(), locale.c_str(), locale.size() + 1);

	std::string resDir = ini.GetString(lpAppName, "path", exeDir);
	if(!resDir.empty() && resDir.back() != '/' && resDir.back() != '\\')
		resDir += '/';
	m_ResDir = std::make_unique<char[]>(resDir.size() + 1);
	memcpy(m_ResDir.get(), resDir.c_str(), resDir.size() + 1);

	const std::string codepage = ini.GetString(lpAppName, "codepage", "");

	m_LogFlag = ini.GetInt(lpAppName, "log", 0);

	if(m_LogFlag)
	{
		string logFileName = string(m_ResDir.get()) + "res.log";
		
		m_LogFile = fopen(logFileName.c_str(), "w+");
	}

	status = U_ZERO_ERROR;
	if(!codepage.empty())
	{
		ucnv_setDefaultName(codepage.c_str());
		ArgumentCodepage() = codepage;
	}
	m_pConverter = ucnv_open(codepage.empty() ? NULL : codepage.c_str(), &status);
	
	if(U_SUCCESS(status))
	{

	}
	else
	{
		if(status == U_MEMORY_ALLOCATION_ERROR)
			fprintf(stderr, "ICU converter: memory allocation error\n");
		else if(status == U_FILE_ACCESS_ERROR)
			fprintf(stderr, "ICU converter: the requested file cannot be found\n");
		else
			fprintf(stderr, "ICU converter: error %d\n", static_cast<int>(status));
		if(!codepage.empty())
			fprintf(stderr, "ICU converter: codepage \"%s\" from %s is not available\n", codepage.c_str(), configFileName);
		exit(EXIT_FAILURE);
	}

	m_MaxSize = ucnv_getMaxCharSize(m_pConverter);

	m_pResourceBundle = new icu::ResourceBundle(m_ResDir.get(),m_ResLocale.get(), status);
	if(U_FAILURE(status))
		fprintf(stderr, "ICU resource bundle: cannot open \"%s\" for locale \"%s\" (error %d)\n", m_ResDir.get(), m_ResLocale.get(), static_cast<int>(status));

	Init();
}

CResourceBundleManage::~CResourceBundleManage(void)
{
	Release();
}

// 取得资源个数
int CResourceBundleManage::GetSize(void)
{
	return (int)mapRes.size();
}

// 把Unicode字符串转换为多字节本机编码
UErrorCode CResourceBundleManage::ToCodePageString(UConverter *conv, UChar* source, char* target, int destCapacity, int& len)
{
  UErrorCode status = U_ZERO_ERROR;

  len = ucnv_fromUChars(conv, target, destCapacity, source, -1, &status);

  return status;
}

// 初始化资源管理器
bool CResourceBundleManage::Init()
{
	int len = 0;
	int maxSize = 0;

	UErrorCode status = U_ZERO_ERROR;

#if _DEBUG

	const char* name = ucnv_getName(m_pConverter, &status);
	printf("Current CodePage is %s\n",name);

#endif

	status = U_ZERO_ERROR;

	if(U_SUCCESS(status))
	{
		m_pResourceBundle->resetIterator();
		
        while(m_pResourceBundle->hasNext()) {
            icu::ResourceBundle bundle = m_pResourceBundle->getNext(status);

			const char * key = bundle.getKey();

			icu::UnicodeString value = bundle.getString(status);

			len = 0;
			maxSize = m_MaxSize * ( 1 + value.length());

			auto buffer = std::make_unique<char[]>(maxSize);
			memset(buffer.get(), 0, maxSize);

			ToCodePageString(m_pConverter, const_cast<UChar*>(value.getTerminatedBuffer()), buffer.get(), maxSize, len);

			mapRes[key] = std::move(buffer);
		}
	
		return true;
	}

	return false;
}

// 释放资源
void CResourceBundleManage::Release(void)
{
	m_ResDir.reset(nullptr);
	m_ResLocale.reset(nullptr);
	mapRes.clear();

	delete m_pResourceBundle;

	if (m_pConverter != NULL)
		ucnv_close(m_pConverter);

	if(m_LogFlag)
	{
		if(m_LogFile)
		{
			fclose(m_LogFile);
			m_LogFile = NULL;
		}
	}
}

// 根据ID取得字符串
const char* CResourceBundleManage::LoadResString(const char* key)
{
	const char* ret ="";
	StringMap::iterator in = mapRes.find(key);

	if(in != mapRes.end())
	{
		ret = in->second.get();
	}
#if _DEBUG
	//printf("key = %s value = %s\n",key, ret);
#endif

	static std::mutex lock;
	{
		std::lock_guard<std::mutex> guard(lock);
		if(m_LogFlag)
		{
			if(m_LogFile)
			{
				fprintf(m_LogFile, "Key = [%s] Value = %s\r\n", key, ret);
			}
		}
	}

	return ret;
}
 
icu::UnicodeString CResourceBundleManage::LoadUResString(const char* key)
{
	UErrorCode status = U_ZERO_ERROR;
	return m_pResourceBundle->getStringEx(key, status);
}

// 格式根据参数化一个字符串
int CResourceBundleManage::Format(const char* key, CFormatParameter& parameter, char buffer[])
{
    UErrorCode status = U_ZERO_ERROR;
    icu::UnicodeString str;
    icu::FieldPosition pos;

	icu::UnicodeString format = LoadUResString(key);

	// Create a message format
#if U_ICU_VERSION_MAJOR_NUM >= 50
    icu::MessageFormat msg(icu::UnicodeString(), status);
    msg.applyPattern(format, UMSGPAT_APOS_DOUBLE_REQUIRED, NULL, status);
#else
    icu::MessageFormat msg(format, status);
#endif
    msg.format(parameter.GetMsgArgs(), parameter.GetParaNum(), str, pos, status);

	int len = 0;
	int maxSize = m_MaxSize * ( 1 + str.length());

	ToCodePageString(m_pConverter, const_cast<UChar*>(str.getTerminatedBuffer()), buffer, maxSize, len);

	return len;
}