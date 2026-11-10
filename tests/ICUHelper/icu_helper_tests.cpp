#include "IniFile.h"
#include "ResourceBundleManage.h"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

namespace
{

int g_failures = 0;

void Check(bool ok, const char* what)
{
	if (!ok)
	{
		std::cerr << "FAIL: " << what << "\n";
		++g_failures;
	}
}

std::string ExecutableDir()
{
#ifdef _WIN32
	char path[MAX_PATH];
	GetModuleFileNameA(nullptr, path, MAX_PATH);
	std::string p = path;
	return p.substr(0, p.find_last_of("\\/"));
#else
	return std::filesystem::read_symlink("/proc/self/exe").parent_path().string();
#endif
}

void WriteFile(const std::string& path, const std::string& content)
{
	std::ofstream f(path, std::ios::binary);
	f << content;
}

void TestIniFile()
{
	std::string path = ExecutableDir() + "/icu_helper_tests.ini";
	WriteFile(path,
		"\xEF\xBB\xBF; comment\r\n"
		"[Locale]\r\n"
		"Locale = en_US\r\n"
		"  path =  \"C:/x y\"  \r\n"
		"codepage=windows-1252\r\n"
		"log = 1\r\n"
		"empty =\r\n"
		"word = abc\r\n"
		"dup = first\r\n"
		"dup = second\r\n"
		"[other]\r\n"
		"locale = zz\r\n");

	CIniFile ini;
	Check(ini.Load(path), "ini loads");
	Check(ini.GetString("locale", "locale", "") == "en_US", "ini value");
	Check(ini.GetString("LOCALE", "LOCALE", "") == "en_US", "ini section and key are case-insensitive");
	Check(ini.GetString("locale", "path", "") == "C:/x y", "ini trims whitespace and strips quotes");
	Check(ini.GetString("locale", "codepage", "") == "windows-1252", "ini key without spaces around =");
	Check(ini.GetString("other", "locale", "") == "zz", "ini keeps sections apart");
	Check(ini.GetString("locale", "missing", "fallback") == "fallback", "ini default for missing key");
	Check(ini.GetString("locale", "empty", "fallback") == "", "ini empty value is not the default");
	Check(ini.GetString("locale", "dup", "") == "first", "ini first duplicate wins");
	Check(ini.GetInt("locale", "log", 0) == 1, "ini integer");
	Check(ini.GetInt("locale", "missing", 7) == 7, "ini integer default");
	Check(ini.GetInt("locale", "word", 7) == 0, "ini non-numeric integer is 0");

	CIniFile none;
	Check(!none.Load(ExecutableDir() + "/does-not-exist.ini"), "ini missing file fails to load");
	Check(none.GetString("locale", "locale", "en_US") == "en_US", "ini defaults when file is missing");
}

std::string LoadStall(const std::string& serverDir, const std::string& codepage)
{
	std::string loc = "icu_helper_tests_" + codepage + ".loc";
	WriteFile(ExecutableDir() + "/" + loc,
		"[locale]\nlocale = en_US\ncodepage = " + codepage + "\npath = " + serverDir + "\n");
	CResourceBundleManage manager(loc.c_str());
	return manager.LoadResString("GM_CHARSTALL_CPP_00054");
}

void TestCodepage(const std::string& serverDir)
{
	std::string cp1252 = LoadStall(serverDir, "windows-1252");
	Check(cp1252.find("\x93") != std::string::npos && cp1252.find("\xE2\x80") == std::string::npos, "windows-1252 output is single-byte");

	std::string utf8 = LoadStall(serverDir, "UTF-8");
	Check(utf8.find("\xE2\x80\x9C") != std::string::npos, "codepage key selects the output encoding");

	std::string latin1 = LoadStall(serverDir, "ISO-8859-1");
	Check(latin1.find("\x93") == std::string::npos && latin1.find("\x1A") != std::string::npos, "unmappable characters become the substitution byte");
}

void TestMissingConfig()
{
	CResourceBundleManage manager("icu_helper_tests_missing.loc");
	Check(manager.GetSize() == 0, "no bundle is loaded without a locale file");
	Check(std::string(manager.LoadResString("NO_SUCH_KEY")).empty(), "unknown key gives an empty string");
}

}

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		std::cerr << "usage: ICUHelperTests <GameServer dir>\n";
		return 2;
	}

	TestIniFile();
	TestCodepage(argv[1]);
	TestMissingConfig();

	if (g_failures == 0)
		std::cout << "all ICUHelper checks passed\n";
	return g_failures == 0 ? 0 : 1;
}
